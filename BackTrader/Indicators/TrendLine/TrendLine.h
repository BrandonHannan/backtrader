#ifndef TRENDLINE_H
#define TRENDLINE_H

#include "../TrendIdentifier/TrendIdentifier.h"
#include "../../TradingContext/Functions/DateHelper.h"
#include "cmath"
#include "limits"

using namespace std;

enum class TrendLineMode { MINIMUM, MAXIMUM };

struct Trendline {
    bool isActive = false;
    TrendType type = TrendType::NONE;
    Extremum anchor;       // The starting point (e.g., the first trough)
    Extremum currentPoint; // The point making the smallest absolute gradient
    double currentGradient = std::numeric_limits<double>::max();
};

class TrendLine {
    private:
        TrendLineMode mode;
        Trendline activeTrendline;

        int LengthOfTradeBetweenDates(string date1, string date2);

        double calculateGradient(const Extremum& p1, const Extremum& p2);
    
    public:
        TrendLine(TrendLineMode mode) {}

        void update(const Trend &currentTrend);

        Trendline getActiveTrend() const;
};

#endif