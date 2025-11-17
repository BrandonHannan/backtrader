#ifndef BASEPOSITIONSIZE_H
#define BASEPOSITIONSIZE_H
#include "../StockData/StockData.h"
#include "../PositionType/PositionType.h"
#include <iostream>
#include <cmath>
#include <algorithm>

class BasePositionSize{
    protected:
        double riskAmount;
    
    public:
        BasePositionSize(double riskAmount);

        virtual ~BasePositionSize() = default;
        // Determines the number of shares to purchase before purchasing
        virtual double calculatePositionSize(double balance, PositionType position, StockDataInstance &data) const = 0;
        // Updates context on new data
        virtual void processNewData(StockDataInstance &currentData, StockDataInstance &previousData) = 0;
        virtual bool isValid() const = 0;

};

#endif