#ifndef TRENDIDENTIFIER_H
#define TRENDIDENTIFIER_H

#include "../../StockData/StockData.h"
#include <iostream>
#include <vector>
#include <deque>

using namespace std;

enum class TrendType { NONE, UPTREND, DOWNTREND };
enum class TrendMode { FIVE_POINT, THREE_POINT };

// Structure to represent a turning point (peak or trough)
struct Extremum {
    int index = -1;
    StockDataInstance data = StockDataInstance(-1, 0, 0, 0, 0, 0, "");
    bool isTrough = false; 
};

// Structure to hold our confirmed trend pattern
struct Trend {
    TrendType type = TrendType::NONE;
    Extremum e1, e2, e3, e4, e5; // e5 is the most recent (T3 for uptrend, P3 for downtrend)
};

class TrendIdentifier {
    private:
        int lookBackPeriod;
        TrendMode mode;
        deque<Extremum> extrema;
        Trend currentTrend;
        
        int currentDay = 0;
        StockDataInstance lastData = StockDataInstance(-1, 0, 0, 0, 0, 0, "");
        enum class Direction { UNKNOWN, UP, DOWN } dir = Direction::UNKNOWN;
    public:
        TrendIdentifier(int lookBackPeriod, TrendMode mode = TrendMode::FIVE_POINT);

        // Feeds one day of data at a time. 
        void processNextDay(StockDataInstance data);

        Trend getCurrentTrend();
};


#endif