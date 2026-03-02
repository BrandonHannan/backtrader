#ifndef DOWCONTEXT_H
#define DOWCONTEXT_H

#include "../BaseContext.h"
#include <cmath>
#include <queue>

using namespace std;

class DowContext: public BaseContext {
    private:
        double s;
    public:
        DowContext(int lookBackPeriod);

        void updateContext(const StockDataInstance &currentData, const StockDataInstance &previousData) override;

        Trade shouldExecuteTrade(const StockDataInstance &currentData) const override;

        bool shouldSellTrade(const Position &currentPosition, const StockDataInstance &currentData) const override;

        string getStats() const override;

        bool isValid() const override;

        void clear() override;
};


#endif