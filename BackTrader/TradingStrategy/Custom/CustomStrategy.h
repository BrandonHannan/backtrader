#ifndef CUSTOMSTRATEGY_H
#define CUSTOMSTRATEGY_H
#include "../BaseStrategy.h"
#include "../../PositionSizing/ATRPositionSize/ATRPositionSize.h"
#include "../../Functions/MacroFeatures/MacroFeatures.h"

class CustomStrategy: public BaseStrategy {
    private:
        const MacroFeatures* macroFeatures = nullptr;

    public:
        CustomStrategy(double balance, unique_ptr<BasePositionSize> sizer, unique_ptr<BaseContext> context);

        void setMacroFeatures(const MacroFeatures* m) { macroFeatures = m; }

        void ExecuteStrategy(const string &stockName, const StockData &data) override;
};

#endif
