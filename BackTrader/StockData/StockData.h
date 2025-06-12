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
    StockData() = default; 
    StockData(vector<double> o, vector<double> c, vector<double> h, vector<double> l, vector<double> v, 
    vector<string> d): open(o), close(c), high(h), low(l), volume(v), date(d) {}
};

#endif