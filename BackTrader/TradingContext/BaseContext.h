#ifndef BASECONTEXT_H
#define BASECONTEXT_H

#include "../StockData/StockData.h"
#include <unordered_set>
#include <string>

using namespace std;

struct Trade {
    string positionType;
    string tradeType;
    bool isValid;

    Trade() : isValid(false) {}
    Trade(string positionType, string tradeType): positionType(positionType), tradeType(tradeType) {}
};

class BaseContext {
    protected:
        int lookBackPeriod;
        unordered_set<string> allowedTradeTypes;
    public:
        BaseContext();
        BaseContext(int lookBackPeriod);
        BaseContext(int lookBackPeriod, unordered_set<string> allowedTradeTypes);

        void addTradeType(string tradeType);

        virtual void updateContext(const StockDataInstance &currentData, const StockDataInstance &previousData) = 0;

        // Should return string or "" if it should not execute the trade
        virtual Trade shouldExecuteTrade(const StockDataInstance &data) const = 0;

        virtual bool isValid() const = 0;

        virtual ~BaseContext() = default;
};

#endif