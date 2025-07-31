#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <map>
#include <list>
#include <deque>

#include "debug_utils.h"

using namespace std;

/* ---------------------------------------------------------------------------------------------------------

    STEPS (FOR MY OWN PLANNING):

    1) Build a working orderbook from MBO data
        a) Use loadCSV function to go through entire orderbook
        b) Use process row to build the actual orderbook
        c) Use an unordered map (key val pair), to keep track of orders and order_id with O(1) lookup
        d) Use ordered map to keep trakc of price levels, since a tree is used for these, O(logn) operations
            i) "top" of bid should be highest bid
            ii) "top" of asks should be lowest ask

 ----------------------------------------------------------------------------------------------------------- */

#ifdef DEBUG
std::ofstream debug_log_file("logs.txt", std::ios::trunc); // Only in DEBUG builds
#endif

struct Order
{
    char side; // 'B' for Bid, 'A' for Ask
    double price;
    int size;
};

class OrderBook
{
public:
    void load_csv(const std::string &filename)
    {
        std::ifstream file(filename);
        std::string line;

        std::getline(file, line); // skip first row

        while (std::getline(file, line))
        {
            std::stringstream ss(line);
            std::string cell;
            std::vector<std::string> row;

            while (std::getline(ss, cell, ','))
            {
                row.push_back(cell);
            }

            add_to_buffer(row);
        }
    }

private:
    std::unordered_map<int, Order> orders;
    std::deque<std::vector<std::string>> buffer_rows;

    // maps that hold price level as key, and a list of order_ids as values
    std::map<double, std::list<int>, std::greater<>> bids;
    std::map<double, std::list<int>> asks;

    void add_to_buffer(std::vector<std::string> row){
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

    void process_buffer_rows(){
        if (buffer_rows.size() != 3) return;
        
        const auto &row1 = buffer_rows[0];
        const auto &row2 = buffer_rows[1];
        const auto &row3 = buffer_rows[2];
        
        char action1 = row1[5][0];
        char action2 = row2[5][0];
        char action3 = row3[5][0];

        // Check for T-F-C sequence
        if (action1 == 'T' && action2 == 'F' && action3 == 'C')
        {
            handle_trade_sequence(row1, row2, row3);
            buffer_rows.clear();
        }
        else
        {
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
            return;
        }

        // std::cout << "start block" << endl;
        // std::cout << trade_row[10] << endl;
        // std::cout << fill_row[10] << endl;
        // std::cout << cancel_row[10] << endl;
        // std::cout << "end block" << endl;

        // Process only the Cancel action (which affects the orderbook)
        debug_log_process_tfc(trade_side, cancel_row);
        
        process_row(cancel_row);
    }

    void process_row(const std::vector<std::string> &row)
    {
        debug_log_top_of_book(asks, bids);
        char action = row[5][0];

        if (action == 'R') {
            return;
        }

        char side = row[6][0];
        double price = std::stod(row[7]);
        int size = std::stoi(row[8]);
        int order_id = std::stoi(row[10]);

        switch(action){
            case 'A':
                debug_log_order_add(order_id, side, size, price);
                add_order(order_id, price, side, size);
                break;
            
            case 'C':
                debug_log_order_cancel(order_id, side, size);
                cancel_order(order_id);
                break;

            case 'T':
            case 'F':
            default:
                break;
        }
    }

    void add_order(int order_id, double price, char side, int size){
        Order order = { side, price, size};
        orders[order_id] = order;
        
        if (side == 'A'){
            asks[price].push_back(order_id);
        } else if (side == 'B'){
            bids[price].push_back(order_id);
        }
    }

    void cancel_order(int order_id){
        auto it = orders.find(order_id);
        if (it == orders.end()) return;

        Order order_to_cancel = it->second;
        double price = order_to_cancel.price;
        char side = order_to_cancel.side;

        if (side == 'A'){
            auto &asks_list = asks[price];
            asks_list.remove(order_id);
            if (asks_list.empty()) asks.erase(price);
        } else if (side == 'B'){
            auto &bids_list = bids[price];
            bids_list.remove(order_id);
            if (bids_list.empty()) bids.erase(price);
        }

        orders.erase(order_id);
    }
};

int main(int argc, char *argv[])
{

    std::string filename = argv[1];

    OrderBook myOrderBook;

    myOrderBook.load_csv(filename);

    #ifdef DEBUG
    debug_log_file.close();
    #endif

    return 0;
}