#pragma once
#include <iostream>

// DEBUG PRINT FOR ADDING ORDER
#ifdef DEBUG
inline void debug_log_order_add(int order_id, char side, int size, double price) {
    std::cerr << "[ADD] Order ID " << order_id << ": "
              << (side == 'B' ? "Bid" : "Ask") << " "
              << size << " @ " << price << std::endl;
}
#else
inline void debug_log_order_add(int, char, int, double) {}
#endif

// DEBUG PRINT FOR CANCELLING ORDER
#ifdef DEBUG
inline void debug_log_order_cancel(int order_id, char side, double price) {
    std::cerr << "[CANCEL] Order ID " << order_id << ": "
              << (side == 'B' ? "Bid" : "Ask") << " "
              << " @ " << price << std::endl;
}
#else
inline void debug_log_order_cancel(int, char, double) {}
#endif