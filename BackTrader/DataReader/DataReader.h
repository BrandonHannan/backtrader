#ifndef DATAREADER_H
#define DATAREADER_H

#include <unordered_map>
#include <fstream>
#include <sstream>
#include <iostream>
#include "../Objects/StockData/StockData.h"

unordered_map<std::string, StockData> ReadData(const string &fileName);

#endif