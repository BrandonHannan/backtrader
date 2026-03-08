#ifndef TRENDLINETRACKER_H
#define TRENDLINETRACKER_H

#include "../../Objects/Trendline/Trendline.h"
#include "../../Functions/DateHelper.h"
#include "cmath"
#include "limits"

using namespace std;

enum class TrendLineMode { MINIMUM, MAXIMUM };

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