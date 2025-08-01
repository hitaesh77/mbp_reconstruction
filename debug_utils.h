#pragma once

#include <iostream>
#include <fstream>
#include <map>
#include <list>
#include <vector>
#include <iomanip>
#include <sstream>

#ifdef DEBUG
extern std::ofstream debug_log_file;
extern std::ofstream debug_log_mbp_file;
#endif

// DEBUG COLUMN SPACES FOR MBP LOGGING
#ifdef DEBUG
const int COL_WIDTH_ORDER_ID = 10;
const int COL_WIDTH_ACTION = 7;
const int COL_WIDTH_SIDE = 7;
const int COL_WIDTH_DEPTH = 7;
const int COL_WIDTH_PRICE = 12;
const int COL_WIDTH_SIZE = 10;
const int COL_WIDTH_COUNT = 7;
const int COL_WIDTH_MBP_PX = COL_WIDTH_PRICE;
const int COL_WIDTH_MBP_SZ = COL_WIDTH_SIZE;
const int COL_WIDTH_MBP_CT = COL_WIDTH_COUNT;
#endif

// DEBUG PRINT FOR ADDING ORDER
#ifdef DEBUG
inline void debug_log_order_add(int order_id, char side, int size, double price){
    if (debug_log_file.is_open()){
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
inline void debug_log_order_cancel(int order_id, char side, double price){
    if (debug_log_file.is_open()){
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
    if (debug_log_file.is_open()){
        if (!bids.empty()){
            auto top_bid = *bids.begin();
            debug_log_file << "Top Bid: " << top_bid.first << " x " << top_bid.second.size() << "\n";
        }
        if (!asks.empty()){
            auto top_ask = *asks.begin();
            debug_log_file << "Top Ask: " << top_ask.first << " x " << top_ask.second.size() << "\n";
        }
    }
}
#else
inline void debug_log_top_of_book(std::map<double, std::list<int>>, std::map<double, std::list<int>, std::greater<>>) {}
#endif

// DEBUG PRINT FOR SKIPPING TFC WHEN SIDE = N
#ifdef DEBUG
inline void debug_log_tfc_skip(){
    if (debug_log_file.is_open()){
        debug_log_file << "Skipping T-F-C sequence with side 'N'" << std::endl;
    }
}
#else
inline void debug_log_tfc_skip() {}
#endif

// DEBUG PRINT FOR PROCESSING TFC CANCEL
#ifdef DEBUG
inline void debug_log_process_tfc(char side, std::vector<std::string> cancel_row){
    if (debug_log_file.is_open()){
        debug_log_file << "Processing T-F-C sequence: Trade on " << side
                       << " side, canceling order " << cancel_row[10] << std::endl;
    }
}
#else
inline void debug_log_process_tfc(char, std::vector<std::string>) {}
#endif

// DEBUG PRINT FOR PROCESSING TFC CANCEL
#ifdef DEBUG
inline void debug_log_eof_buffer(std::deque<std::vector<std::string>> buffer_rows){
    if (debug_log_file.is_open()){
        debug_log_file << "[DBG] EOF: Processing remaining buffered row. Action: " << buffer_rows.front()[5] << ", Order ID: " << buffer_rows.front()[10] << std::endl;
    }
}
#else
inline void debug_log_eof_buffer(std::deque<std::vector<std::string>>) {}
#endif

// DEBUG PRINT FOR MBP LOG HEADER
#ifdef DEBUG
inline void debug_log_mbp_header(){
    if (debug_log_mbp_file.is_open()){
        debug_log_mbp_file
            << std::left
            << std::setw(COL_WIDTH_ORDER_ID) << "order_id"
            << std::setw(COL_WIDTH_ACTION) << "action"
            << std::setw(COL_WIDTH_SIDE) << "side"
            << std::setw(COL_WIDTH_DEPTH) << "depth"
            << std::setw(COL_WIDTH_PRICE) << "price"
            << std::setw(COL_WIDTH_SIZE) << "size";

        // // Loop for MBP levels (00 to 09)
        for (int i = 0; i < 10; ++i){
            std::ostringstream ss_bid_px, ss_bid_sz, ss_bid_ct, ss_ask_px, ss_ask_sz, ss_ask_ct;
            ss_bid_px << "bid_px_" << i << std::setw(5);
            ss_bid_sz << "bid_sz_" << i << std::setw(5);
            ss_bid_ct << "bid_ct_" << i << std::setw(5);
            ss_ask_px << "ask_px_" << i << std::setw(5);
            ss_ask_sz << "ask_sz_" << i << std::setw(5);
            ss_ask_ct << "ask_ct_" << i << std::setw(5);

            debug_log_mbp_file
                << std::setw(COL_WIDTH_MBP_PX) << ss_bid_px.str()
                << std::setw(COL_WIDTH_MBP_SZ) << ss_bid_sz.str()
                << std::setw(COL_WIDTH_MBP_CT) << ss_bid_ct.str()
                << "\t"
                << std::setw(COL_WIDTH_MBP_PX) << ss_ask_px.str()
                << std::setw(COL_WIDTH_MBP_SZ) << ss_ask_sz.str()
                << std::setw(COL_WIDTH_MBP_CT) << ss_ask_ct.str();
        }

        debug_log_mbp_file << std::endl;
    }
}
#else
inline void debug_log_mbp_header() {}
#endif

// DEBUG PRINT FOR MBP OASPS
#ifdef DEBUG
inline void debug_log_mbp_oasps(int order_id, char action, char side, int depth, double price, int size){
    if (debug_log_mbp_file.is_open()){
        debug_log_mbp_file
            << std::left
            << std::setw(COL_WIDTH_ORDER_ID) << order_id
            << std::setw(COL_WIDTH_ACTION) << action
            << std::setw(COL_WIDTH_SIDE) << side
            << std::setw(COL_WIDTH_DEPTH) << depth
            << std::setw(COL_WIDTH_PRICE) << std::fixed << std::setprecision(4) << price
            << std::setw(COL_WIDTH_SIZE) << size;
    }
}
#else
inline void debug_log_mbp_oasps(int, char, char, int, double, int) {}
#endif

// DEBUG PRINT FOR MBP level
#ifdef DEBUG
inline void debug_log_levels(
    std::string bid_px, std::string bid_sz, std::string bid_ct,
    std::string ask_px, std::string ask_sz, std::string ask_ct
    ) {
    if (debug_log_mbp_file.is_open()) {
        // Print the bid side data for the current price level
        debug_log_mbp_file << std::left
            << std::setw(COL_WIDTH_MBP_PX) << bid_px
            << std::setw(COL_WIDTH_MBP_SZ) << bid_sz
            << std::setw(COL_WIDTH_MBP_CT) << bid_ct;

        // Print the ask side data for the current price level
        debug_log_mbp_file << std::left
            << std::setw(COL_WIDTH_MBP_PX) << ask_px
            << std::setw(COL_WIDTH_MBP_SZ) << ask_sz
            << std::setw(COL_WIDTH_MBP_CT) << ask_ct;
    }
}
#else
inline void debug_log_levels(std::string, std::string, std::string, std::string, std::string, std::string) {}
#endif