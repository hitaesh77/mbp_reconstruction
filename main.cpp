#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include "debug_utils.h"

using namespace std;

/* ----------------------------------------------------------------------------------------------------

    STEPS (FOR MY OWN PLANNING):

    1) Build a working orderbook from MBO data
        a) Use loadCSV function to go through entire orderbook
        b) Use process row to build the actual orderbook
        c) Use an unordered map (key val pair), ot keep track of orders and order_id with O(1) lookup

 ------------------------------------------------------------------------------------------------------ */

struct Order
{
    char side; // 'B' for Bid, 'A' for Ask
    double price;
    int size;
};

class OrderBook
{
public:
    void loadCSV(const std::string &filename)
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

            processRow(row);
        }
    }

    void processRow(const std::vector<std::string> &row)
    {
        char action = row[5][0];

        if (action == 'R') {
            return;
        }

        char side = row[6][0];
        double price = std::stod(row[7]);
        int size = std::stoi(row[8]);
        int order_id = std::stoi(row[10]);

        if (action == 'A') {

            debug_log_order_add(order_id, side, size, price);
            add_order(order_id, price, side, size);

        } else if (action == 'C') {

            debug_log_order_cancel(order_id, side, size);
            cancel_order(order_id);

        }
    }

private:
    std::unordered_map<int, Order> orders;

    void add_order(int order_id, double price, char side, int size){
        Order order = { side, price, size};
        orders[order_id] = order;
    }

    void cancel_order(int order_id){
        if (orders.find(order_id) == orders.end()) return;
        orders.erase(order_id);
    }
};

int main(int argc, char *argv[])
{

    std::string filename = argv[1];

    OrderBook myOrderBook;

    myOrderBook.loadCSV(filename);

    return 0;
}