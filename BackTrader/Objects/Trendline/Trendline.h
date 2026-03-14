#ifndef TRENDLINE_H
#define TRENDLINE_H

#include "../../Objects/StockData/StockData.h"
#include "../../Objects/Trend/Trend.h"
#include "../../Functions/DateHelper.h"

enum class TrendLineMode { MINIMUM, MAXIMUM };

class Trendline {
    public:
        bool isActive;
        Extremum anchor;       // The starting point (e.g., the first trough)
        Extremum currentPoint; // The point making the smallest absolute gradient
        int dateDifference;
        double m;
        double c;
        TrendType initialTrendType;

        Trendline() {}

        bool isPointValidWithinTrendLine(const StockDataInstance &data);
};

#endif