#ifndef BASEPOSITIONSIZE_H
#define BASEPOSITIONSIZE_H
#include "../StockData/StockData.h"
#include "../PositionType/PositionType.h"
#include <iostream>
#include <cmath>

class BasePositionSize{
    protected:
        double riskAmount;
    
    public:
        BasePositionSize(double riskAmount);

        virtual ~BasePositionSize() = default;
        virtual double calculatePositionSize(double balance, PositionType position) const = 0;
        virtual void processNewData(StockDataInstance data) const = 0;

};

#endif