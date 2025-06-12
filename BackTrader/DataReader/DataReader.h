#ifndef DATAREADER_H
#define DATAREADER_H

#include <unordered_map>
#include <string>
#include "../StockData/StockData.h"

unordered_map<std::string, StockData> ReadData(const string &fileName);

#endif