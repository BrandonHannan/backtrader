#ifndef DOWCONTEXT_H
#define DOWCONTEXT_H

#include "../BaseContext.h"
#include "../../Functions/WindowStatistics.h"
#include "../../Indicators/TrendIdentifier/TrendIdentifier.h"
#include "../../Indicators/TrendLineTracker/TrendLineTracker.h"
#include <cmath>
#include <queue>

using namespace std;

class DowContext: public BaseContext {
    private:
        bool firstUpdate = true; // Signal that determines whether the first update for the context has occured
        int doubleLookBackPeriod;
        TrendMode trendMode;
        TrendLineMode trendLineMode;

        TrendIdentifier trend; // Captures the trend for the given look back period
        TrendIdentifier doubleTrend; // Captures the trend for the given double look back period
        TrendLineTracker trendLine; // Is the trend line for the current trend
        TrendLineTracker doubleTrendLine; // Is the trend line for the current double look back period trend

        WindowStatistics priceStatistics;
        WindowStatistics volumeStatistics;

    public:
        DowContext(int lookBackPeriod, int doubleLookBackPeriod, TrendMode trendMode, TrendLineMode trendLineMode);

        void updateContext(const StockDataInstance &currentData, const StockDataInstance &previousData) override;

        Trade shouldExecuteTrade(const StockDataInstance &currentData) const override;

        bool shouldSellTrade(const Position &currentPosition, const StockDataInstance &currentData) const override;

        string getStats() const override;

        bool isValid() const override;

        void clear() override;
};


#endif