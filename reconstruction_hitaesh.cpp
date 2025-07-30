#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>

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
            std::cout << std::endl;
        }
    }

    void processRow(const std::vector<std::string> &row){
        for (const std::string& cell : row ){
            std::cout << cell + " ";
        }
    }

private:
};

int main(int argc, char *argv[])
{

    std::cout << "Hello World!" << std::endl;

    std::string filename = argv[1];

    OrderBook myOrderBook;

    myOrderBook.loadCSV(filename);

    return 0;
}