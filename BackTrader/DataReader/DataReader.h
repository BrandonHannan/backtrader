#ifndef DATAREADER_H
#define DATAREADER_H

#include <unordered_map>
#include <fstream>
#include <sstream>
#include <iostream>
#include "../Objects/StockData/StockData.h"

unordered_map<std::string, StockData> ReadData(const string &fileName);

// Loads a Dukascopy bid/ask pair: the Bid file populates the canonical open/close/high/low
// vectors and the Ask file populates askOpen/askClose/askHigh/askLow on the same StockData.
// Tickers present only in one side, or whose date arrays don't align, are dropped with a warning.
unordered_map<std::string, StockData> ReadDukascopyData(const string &bidFile, const string &askFile);

#endif