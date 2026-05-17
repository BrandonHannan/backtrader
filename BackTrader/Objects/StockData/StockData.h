#ifndef STOCKDATA_H
#define STOCKDATA_H

#include <vector>
#include <string>

using namespace std;

struct StockData{
    vector<double> open;
    vector<double> close;
    vector<double> high;
    vector<double> low;
    vector<double> volume;
    vector<string> date;
    double contractSize = 0.0; // 0 = not set; DataReader populates from the ContractSize: section
    StockData() = default;
    StockData(vector<double> o, vector<double> c, vector<double> h, vector<double> l, vector<double> v,
    vector<string> d, double cs): open(o), close(c), high(h), low(l), volume(v), date(d), contractSize(cs) {}
};

struct StockDataInstance{
    int index;
    double open;
    double close;
    double high;
    double low;
    double volume;
    string date;
    double contractSize;

    StockDataInstance(int index, double open, double close, double high, double low, double volume, string date, double contractSize):
    index(index), open(open), close(close), high(high), low(low), volume(volume), date(date), contractSize(contractSize) {}
};

#endif
