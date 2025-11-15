#ifndef ATRPOSITIONSIZE_H
#define ATRPOSITIONSIZE_H
#include "../BasePositionSize.h"
#include <deque>

using namespace std;

class ATRPositionSize: public BasePositionSize{
    private:
        int ATRPeriod;
        double ATRMultiplier;
        deque<double> trueRangeWindow;
        double trueRangeSum;
        double currentATR;
    public:
        ATRPositionSize(double riskAmount, int ATRPeriod, double ATRMultiplier);
        double calculatePositionSize(double balance, PositionType position) const override;
};

#endif