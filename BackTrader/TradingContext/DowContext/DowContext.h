#ifndef DOWCONTEXT_H
#define DOWCONTEXT_H

#include "../BaseContext.h"
#include "../../Functions/WindowStatistics.h"
#include "../../Indicators/TrendIdentifier/TrendIdentifier.h"
#include "../../Indicators/TrendLineTracker/TrendLineTracker.h"
#include "../../Indicators/SMAMACD/SMAMACD.h"
#include "../../Indicators/RSI/RSI.h"
#include "../../Indicators/ATR/ATR.h"
#include <cmath>
#include <queue>

using namespace std;

class DowContext: public BaseContext {
    private:
        bool firstUpdate = true; // Signal that determines whether the first update for the context has occured
        int doubleLookBackPeriod;
        int signalLookBackPeriod;
        TrendMode trendMode;
        TrendLineMode trendLineMode;

        TrendIdentifier trend; // Captures the trend for the given look back period
        TrendIdentifier doubleTrend; // Captures the trend for the given double look back period
        TrendLineTracker trendLine; // Is the trend line for the current trend
        TrendLineTracker doubleTrendLine; // Is the trend line for the current double look back period trend

        SMAMACD sMACD;

        RSI rsi;       // RSI over lookBackPeriod
        RSI doubleRsi; // RSI over doubleLookBackPeriod

        ATR atr;        // ATR over lookBackPeriod
        ATR doubleAtr;  // ATR over doubleLookBackPeriod

        double agreementThreshold;
        double relativeMomentumThreshold;
        double breakoutConfluenceThreshold;
        double ecosystemVolatilityThreshold;

    public:
        DowContext(int lookBackPeriod, int doubleLookBackPeriod, int signalLookBackPeriod, TrendMode trendMode, TrendLineMode trendLineMode,
                    double agreementThreshold, double relativeMomentumThreshold, double breakoutConfluenceThreshold, double ecosystemVolatilityThreshold);

        void updateContext(const StockDataInstance &currentData, const StockDataInstance &previousData) override;

        Trade shouldExecuteTrade(const StockDataInstance &currentData) const override;

        Trade shouldExecuteTrade(const StockDataInstance &currentData, const MacroFeatures &macroFeatures, const string& primary) const override;

        bool shouldSellTrade(const Position &currentPosition, const StockDataInstance &currentData) const override;

        string getStats() const override;

        json getContextData() const override;

        bool isValid() const override;

        void clear() override;

        void onPositionSold() override;
};


#endif