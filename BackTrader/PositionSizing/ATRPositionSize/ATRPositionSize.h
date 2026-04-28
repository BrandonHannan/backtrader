#ifndef ATRPOSITIONSIZE_H
#define ATRPOSITIONSIZE_H
#include "../BasePositionSize.h"
#include "../../Indicators/ATR/ATR.h"

using namespace std;

class ATRPositionSize: public BasePositionSize{
    private:
        int ATRPeriod;
        double ATRMultiplier;
        ATR atr;
    protected:
        PositionPriceInfo calculatePositionSize(double balance, PositionType position, const StockDataInstance &data) const override;
    public:
        ATRPositionSize(double riskAmount, int ATRPeriod, double ATRMultiplier);
        Position purchasePosition(double balance, const string stockName, const PositionType position, const StockDataInstance &data) const override;
        void processNewData(const StockDataInstance &currentData, const StockDataInstance &previousData) override;
        void updateStopLossPrice(Position &currentPosition, const StockDataInstance &data) const override;
        bool isValid() const override;
        void clear() override;
};

#endif
