#ifndef TRENDLINE_H
#define TRENDLINE_H

#include "../TrendIdentifier/TrendIdentifier.h"
#include "../../TradingContext/Functions/DateHelper.h"
#include "cmath"
#include "limits"

using namespace std;

enum class TrendLineMode { MINIMUM, MAXIMUM };

class Trendline {
    public:
        bool isActive;
        Extremum anchor;       // The starting point (e.g., the first trough)
        Extremum currentPoint; // The point making the smallest absolute gradient
        int dateDifference;
        double m;
        double c;

        Trendline() {}

        bool isPointValidWithinTrendLine(const StockDataInstance &data);
};

class TrendLineTracker {
    private:
        TrendLineMode mode;
        Trendline activeTrendline;

        void updateActiveTrendline(const double &newGradient, const Extremum &newPoint);

        double calculateGradient(const Extremum& p1, const Extremum& p2) const;
    
    public:
        TrendLineTracker(TrendLineMode mode) {}

        void update(const Trend &currentTrend);

        Trendline getActiveTrend() const;

        void clear();
};

#endif