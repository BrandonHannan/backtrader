#ifndef TRENDIDENTIFIER_H
#define TRENDIDENTIFIER_H

#include "../../Objects/StockData/StockData.h"
#include "../../Objects/Trend/Trend.h"
#include <iostream>
#include <vector>
#include <deque>

using namespace std;

enum class TrendMode { FIVE_POINT, THREE_POINT };

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