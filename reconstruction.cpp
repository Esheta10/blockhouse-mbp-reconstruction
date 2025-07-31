#include <iostream>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <map>
#include <vector>
#include <iomanip>
#include <algorithm>

struct Order {
    std::string order_id;
    char side; // B or S
    double price;
    int size;
};

using namespace std;

unordered_map<string, Order> order_book;
map<double, int, greater<>> bid_levels;
map<double, int> ask_levels;

void update_levels(char side, double price, int delta_size) {
    if (side == 'B') {
        bid_levels[price] += delta_size;
        if (bid_levels[price] <= 0) bid_levels.erase(price);
    } else if (side == 'S') {
        ask_levels[price] += delta_size;
        if (ask_levels[price] <= 0) ask_levels.erase(price);
    }
}

void print_mbp(ofstream& out, const string& timestamp) {
    out << timestamp;
    int count = 0;

    for (auto it = bid_levels.begin(); it != bid_levels.end() && count < 10; ++it, ++count)
        out << "," << fixed << setprecision(2) << it->first << "," << it->second;
    for (; count < 10; ++count)
        out << ",,";

    count = 0;
    for (auto it = ask_levels.begin(); it != ask_levels.end() && count < 10; ++it, ++count)
        out << "," << fixed << setprecision(2) << it->first << "," << it->second;
    for (; count < 10; ++count)
        out << ",,";

    out << "\n";
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        cerr << "Usage: ./reconstruction <mbo.csv>" << endl;
        return 1;
    }

    ifstream file(argv[1]);
    ofstream out("output_mbp.csv");
    string line, timestamp, order_id;

    getline(file, line); // Skip header or initial clear row

    while (getline(file, line)) {
        stringstream ss(line);
        string token;
        vector<string> row;
        while (getline(ss, token, ',')) row.push_back(token);

        if (row.size() < 6) {
            cerr << "[ERROR] Malformed row, less than 6 fields: " << line << endl;
            continue;
        }

        try {
            timestamp = row[0];
            string action = row[1];
            order_id = row[2];
            char side = row[3][0];
            double price = stod(row[4]);
            int size = stoi(row[5]);

            if (action == "A") {
                order_book[order_id] = {order_id, side, price, size};
                update_levels(side, price, size);
            } else if (action == "C") {
                if (order_book.count(order_id)) {
                    Order& o = order_book[order_id];
                    update_levels(o.side, o.price, -size);
                    o.size -= size;
                    if (o.size <= 0) order_book.erase(order_id);
                }
            } else if (action == "M") {
                if (order_book.count(order_id)) {
                    Order& o = order_book[order_id];
                    update_levels(o.side, o.price, -o.size);
                    order_book.erase(order_id);
                }
                order_book[order_id] = {order_id, side, price, size};
                update_levels(side, price, size);
            } else if (action == "T" || action == "F") {
                // Currently ignored; handled only if T/F/C grouping is implemented
            }

            print_mbp(out, timestamp);

        } catch (const exception& e) {
            cerr << "[ERROR] Failed to parse row: " << line << "\nReason: " << e.what() << endl;
            continue;
        }
    }

    return 0;
}
