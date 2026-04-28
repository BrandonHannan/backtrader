#include "./ATR.h"
#include <algorithm>
#include <cmath>

ATR::ATR(int period) : period(period), trueRangeSum(0.0), currentATR(0.0) {}

void ATR::processNewData(const StockDataInstance &currentData, const StockDataInstance &previousData) {
    double highMinusLow       = currentData.high - currentData.low;
    double highMinusPrevClose = abs(currentData.high - previousData.close);
    double lowMinusPrevClose  = abs(currentData.low  - previousData.close);
    double newTrueRange       = max({highMinusLow, highMinusPrevClose, lowMinusPrevClose});

    trueRangeWindow.push_back(newTrueRange);
    trueRangeSum += newTrueRange;

    if (trueRangeWindow.size() > (size_t)period) {
        trueRangeSum -= trueRangeWindow.front();
        trueRangeWindow.pop_front();
    }

    currentATR = (trueRangeWindow.size() == (size_t)period) ? trueRangeSum / period : 0.0;
}

double ATR::getATR() const { return currentATR; }

bool ATR::isReady() const { return trueRangeWindow.size() == (size_t)period; }

void ATR::clear() {
    trueRangeWindow.clear();
    trueRangeSum = 0.0;
    currentATR   = 0.0;
}
