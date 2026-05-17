#ifndef STOCKDATA_H
#define STOCKDATA_H

#include <vector>
#include <string>

using namespace std;

// When hasAskData() is true (Dukascopy bid/ask source), open/close/high/low hold Bid OHLC
// and askOpen/askClose/askHigh/askLow hold Ask OHLC. When false (yfinance single-series),
// the canonical vectors are used for both sides.
struct StockData{
    vector<double> open;
    vector<double> close;
    vector<double> high;
    vector<double> low;
    vector<double> volume;
    vector<string> date;
    // Optional Ask-side OHLC, populated only for Dukascopy bid/ask data. Empty for yfinance.
    vector<double> askOpen;
    vector<double> askClose;
    vector<double> askHigh;
    vector<double> askLow;
    double contractSize = 0.0; // 0 = not set; DataReader populates from the ContractSize: section
    StockData() = default;
    StockData(vector<double> o, vector<double> c, vector<double> h, vector<double> l, vector<double> v,
    vector<string> d, double cs): open(o), close(c), high(h), low(l), volume(v), date(d), contractSize(cs) {}

    bool hasAskData() const { return !askOpen.empty(); }
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
