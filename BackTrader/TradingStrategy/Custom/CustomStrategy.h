#ifndef CUSTOMSTRATEGY_H
#define CUSTOMSTRATEGY_H
#include "../BaseStrategy.h"
#include "../../PositionSizing/ATRPositionSize/ATRPositionSize.h"

class CustomStrategy: public BaseStrategy {
    private:
        
    public:
        CustomStrategy(double balance, unique_ptr<BasePositionSize> sizer, unique_ptr<BaseContext> context);

        void ExecuteStrategy(const string &stockName, const StockData &data) override;
};

#endif