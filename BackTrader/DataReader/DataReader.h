#ifndef DATAREADER_H
#define DATAREADER_H

#include <unordered_map>
#include <string>
#include "stockdata.h"

unordered_map<std::string, StockData> ReadData(const string &fileName);

#endif