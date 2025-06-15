#ifndef CUSTOMCHANNELBREAKOUT_H
#define CUSTOMCHANNELBREAKOUT_H

#include "../TradingStrategy.h"

class CustomChannelBreakout: private TradingStrategy{
    private:
        int lookbackPeriod;

        MaxMinPos maxPrice;
        MaxMinPos minPrice;
        double sumPrice;
        double sumSQPrice;
        MaxMinPos maxVol;
        MaxMinPos minVol;
        double sumVol;
        double sumSQVol;

        double sumPricePrev;
        double sumSQPricePrev;
        double sumVolPrev;
        double sumSQVolPrev;

        double priceVolatilityThreshold; // E.g. 1 - 100 Comparison
        double volumeVolatilityThreshold; // E.g. 1 - 100 Comparison
        double volumeDropThreshold; // E.g. 0 - 1 Multiplier
        int waitingPeriod; // E.g. 1 - 10 Days
        double volumeComparison; // E.g. 0.01 - 0.25 Multiplier
        double volumeDropComparison; // E.g. 0.05 - 0.5 Multiplier
        double priceSurge; // E.g. 1.05 - 1.5 Multiplier
        double volumeComparisonPriceSurge; // E.g. 0.01 - 0.5 Multiplier
    public:
        CustomChannelBreakout(double bal, bool cOP, Position pos, vector<Position> cPoses, int lbPeriod,
        double pVT, double vVT, double vDT, int wPeriod, double vC, double vDC, double pS, double vCPS);
};

#endif