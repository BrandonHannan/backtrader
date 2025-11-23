#ifndef BASECONTEXT_H
#define BASECONTEXT_H

#include "../StockData/StockData.h"
#include <unordered_set>
#include "memory"
#include <string>

using namespace std;

class BaseContext {
    protected:
        int lookBackPeriod;
        unordered_set<string> allowedTradeTypes;
    public:
        BaseContext();
        BaseContext(int lookBackPeriod);
        BaseContext(int lookBackPeriod, unordered_set<string> allowedTradeTypes);

        void addTradeType(string tradeType);

        virtual void updateContext(StockDataInstance &currentData, StockDataInstance &previousData) = 0;

        // Should return make_unique<string>(value) or nullptr if it should not execute the trade
        virtual unique_ptr<string> shouldExecuteTrade(StockDataInstance &data) const = 0;
};

#endif