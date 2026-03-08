#ifndef POSITIONPRICEINFO_H
#define POSITIONPRICEINFO_H

struct PositionPriceInfo{
    double numShares;
    double stopLossPrice;
    PositionPriceInfo(double numShares, double stopLossPrice): numShares(numShares), stopLossPrice(stopLossPrice) {}
};

#endif