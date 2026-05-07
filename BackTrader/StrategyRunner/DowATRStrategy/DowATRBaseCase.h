#ifndef DOWATRBASECASE_H
#define DOWATRBASECASE_H

#include "../../Objects/Trendline/Trendline.h"

struct DowBaseCase {
    double balance = 1000.0;
    
    // Position Sizer Base Variables
    int atrPeriod = 20;
    double atrMultiplier = 2.0;
    double riskAmount = 0.035;

    // Context Base Variables
    int lookback = 20;
    int doubleLookback = 40;
    int signalLookback = 10;
    TrendMode trendMode = TrendMode::THREE_POINT;
    TrendLineMode trendLineMode = TrendLineMode::MINIMUM;
    double agreementThreshold = 0.7;
    double relativeMomentumThreshold = 2.0;
    double breakoutConfluenceThreshold = 0.8;
    double ecosystemVolatilityThreshold = 1.5;
};

#endif