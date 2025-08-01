#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <map>
#include <list>
#include <deque>
#include <iomanip>

#include "debug_utils.h"

using namespace std;

/* ---------------------------------------------------------------------------------------------------------

    STEPS (FOR MY OWN PLANNING):

    1) Build a working orderbook from MBO data
        a) Use loadCSV function to go through entire orderbook
        b) Use process row to build the actual orderbook
        c) Use an unordered map (key val pair), to keep track of orders and order_id with O(1) lookup
        d) Use ordered map to keep track of price levels, since a tree is used for these, O(logn) operations
            i) "top" of bid should be highest bid
            ii) "top" of asks should be lowest ask
    
    2) Build logic to handle T-F-C MBO sequences
        a) Use existing process_row() function for actions that are not 'T'
        b) When 'T' encountered, use a queue of size 3 to track T-F-C
            i) Implement logic for ask side 'N'
    
    3) Build output CSV using the internal orderbook we build
        a) create a snapshot at every row that is processed
        b) start by printing it in a output_logs.txt file to cross referene with mbp.csv
        c) build output csv
            i) implement logic to determine if a snapshot should be added to csv

 ----------------------------------------------------------------------------------------------------------- */

#ifdef DEBUG
std::ofstream debug_log_file("logs.txt", std::ios::trunc);
std::ofstream debug_log_mbp_file("mbp_logs.txt", std::ios::trunc);
#endif

struct Order {
    char side; // 'B' for Bid, 'A' for Ask
    double price;
    int size;
};

class OrderBook {
public:
    OrderBook() {
        // Open output CSV file and write header
        output_file.open("output.csv");
        write_csv_header();
    }
    
    ~OrderBook() {
        if (output_file.is_open()) {
            output_file.close();
        }
    }

    void load_csv(const std::string &filename) {
        debug_log_mbp_header();

        std::ifstream file(filename);
        std::string line;

        std::getline(file, line); // skip first row

        while (std::getline(file, line)) {
            std::stringstream ss(line);
            std::string cell;
            std::vector<std::string> row;

            while (std::getline(ss, cell, ','))
            {
                row.push_back(cell);
            }

            add_to_buffer(row);
        }

        // cleanup eof buffer
        while (!buffer_rows.empty()) {
            debug_log_eof_buffer(buffer_rows);
            process_row(buffer_rows.front());
            buffer_rows.pop_front();
        }
    }

private:
    std::ofstream output_file;
    std::unordered_map<int, Order> orders;
    std::deque<std::vector<std::string>> buffer_rows;
    int row_index = 0;

    // maps that hold price level as key, and a list of order_ids as values
    std::map<double, std::list<int>, std::greater<>> bids;
    std::map<double, std::list<int>> asks;

    void write_csv_header() {
        output_file << " ,ts_recv,ts_event,rtype,publisher_id,instrument_id,action,side,depth,price,size,flags,ts_in_delta,sequence";
        
        for (int i = 0; i < 10; i++) {
            output_file << ",bid_px_" << std::setfill('0') << std::setw(2) << i
                        << ",bid_sz_" << std::setfill('0') << std::setw(2) << i
                        << ",bid_ct_" << std::setfill('0') << std::setw(2) << i;
            
            output_file << ",ask_px_" << std::setfill('0') << std::setw(2) << i
                        << ",ask_sz_" << std::setfill('0') << std::setw(2) << i
                        << ",ask_ct_" << std::setfill('0') << std::setw(2) << i;
        }

        output_file << ",symbol,order_id";
        output_file << "\n";
    }

    void add_to_buffer(std::vector<std::string> row) {
        char action = row[5][0];

        // buffer empty
        if (buffer_rows.empty() && action != 'T') {
            process_row(row);
            return;
        }

        // row action is T or buffer not empty
        buffer_rows.push_back(row);
        
        // Process the T-F-C sequence
        if (buffer_rows.size() == 3) {
            process_buffer_rows();
        }
    }

    void process_buffer_rows() {
        if (buffer_rows.size() != 3) return;
        
        const auto &row1 = buffer_rows[0];
        const auto &row2 = buffer_rows[1];
        const auto &row3 = buffer_rows[2];
        
        char action1 = row1[5][0];
        char action2 = row2[5][0];
        char action3 = row3[5][0];

        // Check for T-F-C sequence
        if (action1 == 'T' && action2 == 'F' && action3 == 'C') {
            handle_trade_sequence(row1, row2, row3);
            buffer_rows.clear();
        } else {
            // Not a T-F-C sequence, process first row normally and shift buffer
            process_row(buffer_rows.front());
            buffer_rows.pop_front();
        }
    }

    void handle_trade_sequence(
        std::vector<std::string> trade_row, 
        std::vector<std::string> fill_row, 
        std::vector<std::string> cancel_row) 
    {
        char trade_side = trade_row[6][0];
        if (trade_side == 'N') {
            debug_log_tfc_skip();
            get_snapshot(trade_row, 'T', 'N', 0, 0, 0, 0); // might remove
            return;
        }

        debug_log_process_tfc(trade_side, cancel_row);
        
        process_row(cancel_row);
    }

    void process_row(const std::vector<std::string> &row) {
        debug_log_top_of_book(asks, bids);

        char action = row[5][0];

        if (action == 'R') {
            get_snapshot(row, action, 'N', 0, 0, 0, 0);
            return;
        }

        char side = row[6][0];

        double price = std::stod(row[7]);
        int size = std::stoi(row[8]);
        int order_id = std::stoi(row[10]);

        bool book_changed = false;

        switch(action) {
            case 'A':
                debug_log_order_add(order_id, side, size, price);
                book_changed = add_order(order_id, price, side, size);
                break;
            
            case 'C':
                debug_log_order_cancel(order_id, side, size);
                book_changed = cancel_order(order_id);
                break;

            case 'T':
            case 'F':
            default:
                break;
        }

        if (book_changed) {
            int depth = calculate_depth(side, price);
            get_snapshot(row, action, side, depth, price, size, order_id);
        }
    }

    int calculate_depth(char side, double price) {
        int depth = 0;
        
        if (side == 'B') {
            // For bids, count levels above this price
            for (auto it = bids.begin(); it != bids.end() && it->first > price; ++it) {
                depth++;
            }
        } else if (side == 'A') {
            // For asks, count levels below this price
            for (auto it = asks.begin(); it != asks.end() && it->first < price; ++it) {
                depth++;
            }
        }
        
        return depth;
    }

    void get_snapshot(const std::vector<std::string> &row, char action, char side, int depth, double price, int size, int order_id) {
        auto bid_it = bids.begin();
        auto ask_it = asks.begin();

        std::string ts_recv = row[0];
        std::string ts_event = row[1];
        std::string rtype = row[2];
        rtype = "10";
        std::string publisher_id = row[3];
        std::string instrument_id = row[4];
        std::string channel_id = row[9];
        std::string flags = row[11];
        std::string ts_in_delta = row[12];
        std::string sequence = row[13];
        std::string symbol = row[14];

        // debug output
        debug_log_mbp_oasps(order_id, action, side, depth, price, size);

        // csv output
        output_file << row_index << "," << ts_event << "," << ts_event << "," << rtype << "," 
                    << publisher_id << "," << instrument_id << "," << action << "," 
                    << side << "," << depth << "," << std::fixed << std::setprecision(2) << price 
                    << "," << size << "," << flags << "," << ts_in_delta << "," << sequence;

        for (int price_level = 0; price_level < 10; price_level++) {
            std::string bid_px = "", bid_sz = "", bid_ct = "";
            std::string ask_px = "", ask_sz = "", ask_ct = "";
            
            if (bid_it != bids.end()) {
                double bid_price = bid_it->first;
                int bid_size = sum_size(bid_it->second);
                int bid_count = bid_it->second.size();

                output_file << "," << std::fixed << std::setprecision(2) << bid_price
                        << "," << bid_size << "," << bid_count;
                
                #ifdef DEBUG
                bid_px = std::to_string(bid_price);
                bid_sz = std::to_string(bid_size);
                bid_ct = std::to_string(bid_count);
                #endif

                bid_it++;
            } else {
                output_file << ",,0,0";
            }

            if (ask_it != asks.end()) {
                double ask_price = ask_it->first;
                int ask_size = sum_size(ask_it->second);
                int ask_count = ask_it->second.size();
            
                output_file << "," << std::fixed << std::setprecision(2) << ask_price
                            << "," << ask_size << "," << ask_count;

                ask_px = std::to_string(ask_it->first);
                ask_sz = std::to_string(sum_size(ask_it->second));
                ask_ct = std::to_string(ask_it->second.size());

                #ifdef DEBUG
                std::string ask_px = std::to_string(ask_price);
                std::string ask_sz = std::to_string(ask_size);
                std::string ask_ct = std::to_string(ask_count);
                #endif

                ask_it++;
            } else {
                output_file << ",,0,0";
            }

            debug_log_levels(bid_px, bid_sz, bid_ct, ask_px, ask_sz, ask_ct);
        }

        output_file << "," << symbol << "," << order_id << "\n";
        output_file.flush();

        row_index++;

        #ifdef DEBUG
        debug_log_mbp_file << std::endl;
        #endif
    }

    bool add_order(int order_id, double price, char side, int size) {
        Order order = { side, price, size};
        orders[order_id] = order;
        
        std::map<double, std::list<int>, std::greater<>> *book = nullptr;
        if (side == 'A') {
            asks[price].push_back(order_id);
            
            int level = 0;
            for (const auto& [p, _] : asks) {
                if (++level > 10) break;
                if (p == price) return true;
            }

        } else if (side == 'B') {
            bids[price].push_back(order_id);
            
            int level = 0;
            for (const auto& [p, _] : bids) {
                if (++level > 10) break;
                if (p == price) return true;
            }   
        }

        return false;
    }

    bool cancel_order(int order_id) {
        auto it = orders.find(order_id);
        if (it == orders.end()) return false;

        Order order_to_cancel = it->second;
        double price = order_to_cancel.price;
        char side = order_to_cancel.side;

        bool was_in_top_10 = false;

        if (side == 'A') {
            auto price_it = asks.find(price);
            if (price_it != asks.end()) {

                // book changed logic
                int level = 1;
                for (auto it = asks.begin(); it != price_it && level <= 10; ++it, ++level) {}
                was_in_top_10 = (level <= 10);
                
                // remove order logic
                price_it->second.remove(order_id);
                if (price_it->second.empty()) {
                    asks.erase(price_it);
                }
            }

        } else if (side == 'B') {
            auto price_it = bids.find(price);
            if (price_it != bids.end()) {

                // book changed logic
                int level = 1;
                for (auto it = bids.begin(); it != price_it && level <= 10; ++it, ++level) {}
                was_in_top_10 = (level <= 10);
                

                // remove order logic
                price_it->second.remove(order_id);
                if (price_it->second.empty()) {
                    bids.erase(price_it);
                }
            }
        }

        orders.erase(order_id);

        return was_in_top_10;
    }

    int sum_size(std::list<int>& order_ids) {
        int total = 0;
        for (int id : order_ids) {
            total += orders.at(id).size;
        }
        return total;
    }

};

int main(int argc, char *argv[]) {

    std::string filename = argv[1];

    OrderBook myOrderBook;

    myOrderBook.load_csv(filename);

    #ifdef DEBUG
    debug_log_file.close();
    #endif

    return 0;
}