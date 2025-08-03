# README - MBP-10 Reconstruction from MBO Data
Overview:
    This project reconstructs the MBP-10 (Market By Price - Top 10 levels)
    order book from raw MBO (Market By Order) data using C++. The primary 
    goal is to maintain correctness while ensuring high performance 
    suitable for high-frequency trading environments.


## How to Run:

  Prerequisites:
    - C++17 compatible compiler
    - make installed

  Build & Execute:
    To compile and run the code, execute:

        make run_release

This will:
  - Compile the source with optimization flags
  - Produce an executable named 'reconstruction_mbp'
  - Run the program on 'mbo.csv' (modifiable via Makefile or CLI args)

By default, the output will be saved as 'output.csv'.


## Features & Implementation Notes:

Data Structures:
    
- Unordered Map:
        std::unordered_map<int, Order> is used for constant time 
        order lookups by order_id.
    
- Ordered Maps:
    std::map<double, std::list<int>, std::greater<>> for bids 
    (descending) and std::map<double, std::list<int>> for asks 
    (ascending), giving efficient O(log n) insertions and top-level access.

Core Logic:

- MBO Parsing:
    A CSV reader extracts and parses each row while skipping the 
    initial ‘R’ clear row.
        
- OrderBook Maintenance:
    Handled using add_order and cancel_order, updating both price 
    levels and individual orders.

- Snapshot Generation:
    At every change in top 10 price levels, a full MBP-10 snapshot 
    is created.

- T-F-C Handling:
    Special handling buffers sequences of T -> F -> C, applying 
    the cancel on the side opposite of the trade (since that’s 
    the book side).

- Ignored Trades:
    Trades with side 'N' are ignored, per spec.

Debugging:

- Custom debug_utils.h was used to log intermediate states 
(top-of-book, missed book change, T-F-C processing, etc.) for 
validation. Comparison with provided mbp.csv was done using 
Diffchecker Excel Compare.


## Optimization Strategies:

  Efficient Data Structures:
    - unordered_map for constant time lookups
    - map with correct comparators for bid/ask priority sorting

  Buffered T-F-C Processing:
    - Deque buffer for efficient pop/push
    - Avoids redundant processing by collapsing three rows into 
      one book update

  Snapshot Optimization:
    - Only snapshot if top-10 price levels are affected
    - Used price_in_top_n() check to minimize unnecessary file writes

  Memory & Speed:
    - Used const std::vector<std::string>& to avoid copies
    - Consolidated file I/O and debug flushing to reduce overhead


## Known Limitations:

While correctness was a priority, some discrepancies still exist 
between the generated output.csv and the provided mbp.csv 
reference file. Despite extensive debugging and validation, 
a few corner cases could not be resolved within the timeframe. 
Logs and diffs have been included to aid further diagnosis.


## Future Work:

To further improve this project, I plan to:

- Modularize Further:
    Extract snapshot logic, order processing, and file I/O into 
    dedicated classes to improve code readability and testability.

- Add Unit Tests:
    Especially for sequence handling (T-F-C), edge cases, and 
    price level updates.

- Parallel Parsing (Future Idea):
    If permitted by the data format, consider pipelining parsing 
    and snapshot generation.

- Performance Profiling:
    Integrate tools like valgrind, perf, or gprof to pinpoint 
    further bottlenecks.
