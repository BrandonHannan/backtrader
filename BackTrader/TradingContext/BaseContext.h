#ifndef BASECONTEXT_H
#define BASECONTEXT_H

#include "../Objects/StockData/StockData.h"
#include "../Objects/PositionType/Position.h"
#include "../Objects/Trade/Trade.h"
#include "../Functions/StringHelper.h"
#include "../Functions/WindowStatistics.h"
#include "../include/nlohmann/json.hpp"
#include <unordered_set>

using namespace std;
using json = nlohmann::json;

class BaseContext {
    protected:
        int lookBackPeriod;
        unordered_set<string> allowedTradeTypes;
        WindowStatistics priceStatistics;
        WindowStatistics volumeStatistics;

        void clearBase();

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
        virtual Trade shouldExecuteTrade(const StockDataInstance &currentData) const = 0;

        // Should return TRUE to sell the current position or FALSE to not
        virtual bool shouldSellTrade(const Position &currentPosition, const StockDataInstance &currentData) const = 0;

        // Returns a string of the current statistics
        virtual string getStats() const = 0;

        // Returns a JSON object of the current context data
        virtual json getContextData() const = 0;

        virtual bool isValid() const = 0;

        virtual void clear() = 0;

        virtual void onPositionSold() {}

        virtual ~BaseContext() = default;
};

#endif