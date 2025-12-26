#ifndef BASEPOSITIONSIZE_H
#define BASEPOSITIONSIZE_H
#include "../StockData/StockData.h"
#include "../PositionType/Position.h"
#include "../PositionType/PositionType.h"
#include <iostream>
#include <cmath>
#include <algorithm>

struct PositionPriceInfo{
    double numShares;
    double stopLossPrice;
    PositionPriceInfo(double numShares, double stopLossPrice): numShares(numShares), stopLossPrice(stopLossPrice) {}
};

class BasePositionSize{
    protected:
        double riskAmount;
    
    protected:
        // Determines the number of shares to purchase before purchasing
        virtual PositionPriceInfo calculatePositionSize(double balance, PositionType position, const StockDataInstance &data) const = 0;
    public:
        BasePositionSize(double riskAmount);

        virtual ~BasePositionSize() = default;

        // Determines the position to purchase or an invalid position
        // Returns a Position object with the following filled attributes:
        // - Position Type (LONG or SHORT)
        // - Purchase Date
        // - Purchase Price
        // - Number of Shares to Purchase
        // - Stop Loss Price

        // Does not include:
        // - Trade Type
        // - Sold Date
        // - Sold Price
        virtual Position purchasePosition(double balance, const string stockName, const PositionType position, const StockDataInstance &data) const = 0;

        // Updates context on new data
        virtual void processNewData(const StockDataInstance &currentData, const StockDataInstance &previousData) = 0;

        // Updates the Stop Loss Price of a Position
        virtual void updateStopLossPrice(Position &currentPosition, const StockDataInstance &data) const = 0;

        virtual bool isValid() const = 0;

        virtual ~BasePositionSize() = default;
};

#endif