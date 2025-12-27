#ifndef BASECONTEXT_H
#define BASECONTEXT_H

#include "../StockData/StockData.h"
#include "../PositionType/Position.h"
#include "./Functions/StringHelper.h"
#include <unordered_set>

using namespace std;

struct Trade {
    string positionType;
    string tradeType;
    bool isValid;

    Trade() : isValid(false) {}
    Trade(string positionType, string tradeType): positionType(positionType), tradeType(tradeType), isValid(true) {}
};

class BaseContext {
    protected:
        int lookBackPeriod;
        unordered_set<string> allowedTradeTypes;

        // Returns TRUE if the current stock price is >= to the stop loss price for LONG trades and <= to the stop loss price for SHORT trades
        bool checkStopLossPrice(const Position &currentPosition, const StockDataInstance &data) const;
    public:
        BaseContext();
        BaseContext(int lookBackPeriod);
        BaseContext(int lookBackPeriod, unordered_set<string> allowedTradeTypes);

        void addTradeType(string tradeType);

        virtual void updateContext(const StockDataInstance &currentData, const StockDataInstance &previousData) = 0;

        // Returns these attributes of a new Position:
        // - Position Type (LONG or SHORT)
        // - Trade Type
        virtual Trade shouldExecuteTrade(const StockDataInstance &data) const = 0;

        // Should return TRUE to sell the current position or FALSE to not
        virtual bool shouldSellTrade(const Position &currentPosition, const StockDataInstance &data) const = 0;

        // Returns a string of the current statistics
        virtual string getStats() const = 0;

        virtual bool isValid() const = 0;

        virtual ~BaseContext() = default;
};

#endif