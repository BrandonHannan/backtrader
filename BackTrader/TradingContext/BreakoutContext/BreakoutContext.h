#ifndef BREAKOUTCONTEXT_H
#define BREAKOUTCONTEXT_H

#include "../BaseContext.h"
#include "../../Functions/WindowStatistics.h"
#include "../../Functions/InverseNormalCDF.h"
#include <cmath>
#include <queue>

using namespace std;

class BreakoutContext: public BaseContext {
    private:
        double priceHighZ;
        double priceLowZ;
        double volumeHighZ;
        double volumeLowZ;
        double priceMedZ;

        double priceHighPct;
        double volumeHighPct;
        double priceLowPct;
        double volumeLowPct;
        double priceMedPct;

    public:
        // A lookbackPeriod represents the number of days that the strategy takes into consideration
        // A priceHighPercentageThreshold represents the percentage of prices that the current price has to be greater than. E.g. A current price of $5 must be greater than 85% of prices in a normal distribution
        // A volumeHighPercentageThreshold represents the percentage of volumes that the current volume has to be greater than
        // A priceLowPercentageThreshold represents the percentage of prices that the current price has to be lower than
        // A volumeLowPercentageThrehold represents the percentage of volumes that the current volume has to be lower than
        // A priceMediumPercentageThreshold represents the percentage of prices that the current price has to be within
        BreakoutContext(int lookbackPeriod, double priceHighPercentageThreshold, double volumeHighPercentageThreshold, double priceLowPercentageThreshold, double volumeLowPercentageThreshold, double priceMediumPercentageThreshold);

        void updateContext(const StockDataInstance &currentData, const StockDataInstance &previousData) override;

        Trade shouldExecuteTrade(const StockDataInstance &currentData) const override;

        bool shouldSellTrade(const Position &currentPosition, const StockDataInstance &currentData) const override;

        string getStats() const override;

        json getContextData() const override;

        bool isValid() const override;

        void clear() override;
};

#endif