#ifndef EXTREMUM_H
#define EXTREMUM_H

#include "../StockData/StockData.h"

// Structure to represent a turning point (peak or trough)
struct Extremum {
    int index = -1;
    StockDataInstance data = StockDataInstance(-1, 0, 0, 0, 0, 0, "");
    bool isTrough = false; 
};

#endif