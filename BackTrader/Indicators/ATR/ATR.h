#ifndef ATR_H
#define ATR_H

#include "../Indicator.h"
#include "../../Objects/StockData/StockData.h"
#include <deque>

using namespace std;

// This indicator does NOT need to be reset after a position is closed.
// ATR tracks broad market volatility across the full price history and
// should continue accumulating data between trades.
class ATR : public Indicator {
private:
    int period;
    deque<double> trueRangeWindow;
    double trueRangeSum;
    double currentATR;

public:
    ATR(int period);

    void processNewData(const StockDataInstance &currentData, const StockDataInstance &previousData);

    double getATR() const;

    bool isReady() const;

    void clear() override;
};

#endif
