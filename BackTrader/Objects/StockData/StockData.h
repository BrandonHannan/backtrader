#ifndef STOCKDATA_H
#define STOCKDATA_H

#include <vector>
#include <string>
#include <utility>

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
    double contractSize = 0.0;         // 0 = not set; DataReader populates from ContractSize: section
    double frictionPerRoundTrip = 0.0; // round-trip cost in dollars per contract (commission + spread estimate)
    StockData() = default;
    StockData(vector<double> o, vector<double> c, vector<double> h, vector<double> l, vector<double> v,
    vector<string> d, double cs, double fr = 0.0): open(std::move(o)), close(std::move(c)), high(std::move(h)), low(std::move(l)), volume(std::move(v)), date(std::move(d)), contractSize(cs), frictionPerRoundTrip(fr) {}

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
