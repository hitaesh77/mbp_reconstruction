#pragma once

#include <iostream>
#include <fstream>
#include <map>
#include <list>
#include <vector>

#ifdef DEBUG
extern std::ofstream debug_log_file;
#endif

// DEBUG PRINT FOR ADDING ORDER
#ifdef DEBUG
inline void debug_log_order_add(int order_id, char side, int size, double price) {
    if (debug_log_file.is_open()) {
        debug_log_file << "[ADD] Order ID " << order_id << ": "
                       << (side == 'B' ? "Bid" : "Ask") << " "
                       << size << " @ " << price << std::endl;
    }
}
#else
inline void debug_log_order_add(int, char, int, double) {}
#endif

// DEBUG PRINT FOR CANCELLING ORDER
#ifdef DEBUG
inline void debug_log_order_cancel(int order_id, char side, double price) {
    if (debug_log_file.is_open()) {
        debug_log_file << "[CANCEL] Order ID " << order_id << ": "
                       << (side == 'B' ? "Bid" : "Ask") << " "
                       << " @ " << price << std::endl;
    }
}
#else
inline void debug_log_order_cancel(int, char, double) {}
#endif

// DEBUG PRINT FOR GETTING TOP OF BOOK
#ifdef DEBUG
inline void debug_log_top_of_book(std::map<double, std::list<int>> asks, std::map<double, std::list<int>, std::greater<>> bids){
    if (debug_log_file.is_open()) {
        if (!bids.empty()) {
            auto top_bid = *bids.begin();
            debug_log_file << "Top Bid: " << top_bid.first << " x " << top_bid.second.size() << "\n";
        }
        if (!asks.empty()) {
            auto top_ask = *asks.begin();
            debug_log_file << "Top Ask: " << top_ask.first << " x " << top_ask.second.size() << "\n";
        }
    }
}
#else
inline void debug_log_top_of_book(std::map<double, std::list<int>>, std::map<double, std::list<int>, std::greater<>>){}
#endif

// DEBUG PRINT FOR SKIPPING TFC WHEN SIDE = N
#ifdef DEBUG
inline void debug_log_tfc_skip(){
    if (debug_log_file.is_open()) {
        debug_log_file << "Skipping T-F-C sequence with side 'N'" << std::endl;
    }
}
#else
inline void debug_log_tfc_skip(){}
#endif

// DEBUG PRINT FOR PROCESSING TFC CANCEL
#ifdef DEBUG
inline void debug_log_process_tfc(char side, std::vector<std::string> cancel_row){
    if (debug_log_file.is_open()) {
        debug_log_file << "Processing T-F-C sequence: Trade on " << side
                       << " side, canceling order " << cancel_row[10] << std::endl;
    }
}
#else
inline void debug_log_process_tfc(char, std::vector<std::string>){}
#endif

// DEBUG PRINT FOR PROCESSING TFC CANCEL
#ifdef DEBUG
inline void debug_log_eof_buffer(std::deque<std::vector<std::string>> buffer_rows){
    if (debug_log_file.is_open()) {
        debug_log_file << "[DBG] EOF: Processing remaining buffered row. Action: " << buffer_rows.front()[5] << ", Order ID: " << buffer_rows.front()[10] << std::endl;
    }
}
#else
inline void debug_log_eof_buffer(std::deque<std::vector<std::string>> buffer_rows){}
#endif