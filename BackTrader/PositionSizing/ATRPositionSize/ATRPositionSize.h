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
    protected:
        PositionPriceInfo calculatePositionSize(double balance, PositionType position, const StockDataInstance &data) const override;
    public:
        ATRPositionSize(double riskAmount, int ATRPeriod, double ATRMultiplier);
        Position purchasePosition(double balance, PositionType position, const StockDataInstance &data);
        void processNewData(const StockDataInstance &currentData, const StockDataInstance &previousData) override;
        void updateStopLossPrice(Position &currentPosition, const StockDataInstance &data) const override;
        bool isValid() const override;
};

#endif