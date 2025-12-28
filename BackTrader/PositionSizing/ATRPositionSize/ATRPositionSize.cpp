#include "ATRPositionSize.h"

ATRPositionSize::ATRPositionSize(double riskAmount, int ATRPeriod, double ATRMultiplier): BasePositionSize(riskAmount),
ATRPeriod(ATRPeriod), ATRMultiplier(ATRMultiplier), trueRangeSum(0.0), currentATR(0.0) {}

PositionPriceInfo ATRPositionSize::calculatePositionSize(double balance, PositionType position, const StockDataInstance &data) const {
    if (this->currentATR <= 0 || position.isNull()){
        return PositionPriceInfo(0.0, 0.0);
    }

    double dollarRisk = balance * this->riskAmount;
    double stopLossPrice = 0.0;
    double riskPerShare = 0.0;
    if (position.getPositiontype() == "LONG"){
        stopLossPrice = data.close - (this->currentATR * this->ATRMultiplier);
        riskPerShare = data.close - stopLossPrice;
        if (riskPerShare <= 0){
            // Avoid division by 0 or negative risk
            return PositionPriceInfo(0.0, 0.0);
        }
        double result = dollarRisk/riskPerShare;
        return PositionPriceInfo(result, stopLossPrice);
    }
    else if (position.getPositiontype() == "SHORT"){
        stopLossPrice = data.close + (this->currentATR * this->ATRMultiplier);
        riskPerShare = stopLossPrice - data.close;
        if (riskPerShare <= 0){
            return PositionPriceInfo(0.0, 0.0);
        }
        double result = dollarRisk/riskPerShare;
        return PositionPriceInfo(result, stopLossPrice);
    }
    else{
        return PositionPriceInfo(0.0, 0.0);
    }
}

Position ATRPositionSize::purchasePosition(double balance, const string stockName, const PositionType position, const StockDataInstance &data) const {
    PositionPriceInfo positionPriceInfo = this->calculatePositionSize(balance, position, data);
    if (position.getPositiontype() == "LONG" || position.getPositiontype() == "SHORT"){
        Position newPosition(stockName, position.getPositiontype(), "", data.date, "", data.close, -1, positionPriceInfo.numShares, positionPriceInfo.stopLossPrice);
        return newPosition;
    }
    else{
        return Position();
    }
}

void ATRPositionSize::processNewData(const StockDataInstance &currentData, const StockDataInstance &previousData) {
    double highMinusLow = currentData.high - currentData.low;
    double highMinusPrevClose = abs(currentData.high - previousData.close);
    double lowMinusPrevClose = abs(currentData.low - previousData.close);
    double newTrueRange = max({highMinusLow, highMinusPrevClose, lowMinusPrevClose});

    this->trueRangeWindow.push_back(newTrueRange);
    this->trueRangeSum += newTrueRange;

    if (this->trueRangeWindow.size() > this->ATRPeriod){
        this->trueRangeSum -= this->trueRangeWindow.front();
        this->trueRangeWindow.pop_front();
    }

    if (this->trueRangeWindow.size() == this->ATRPeriod) {
        this->currentATR = this->trueRangeSum/this->ATRPeriod;
    }
    else{
        this->currentATR = 0.0;
    }
}

void ATRPositionSize::updateStopLossPrice(Position &currentPosition, const StockDataInstance &data) const {
    PositionType positionType = currentPosition.getPositionType();
    double purchasePrice = currentPosition.getPurchasePrice();
    double priceDifference = purchasePrice - currentPosition.getStopLossPrice();
    double currentPrice = data.close;

    if (positionType.getPositiontype() == "LONG"){

        if (currentPrice > purchasePrice){
            double newStopLossPrice = currentPrice - priceDifference;
            currentPosition.setStopLossPrice(newStopLossPrice);
        }
    }
    else if (positionType.getPositiontype() == "SHORT"){

        if (currentPrice < purchasePrice){
            double newStopLossPrice = currentPrice + priceDifference;
            currentPosition.setStopLossPrice(newStopLossPrice);
        }
    }
}

bool ATRPositionSize::isValid() const {
    if (this->trueRangeWindow.size() == this->ATRPeriod){
        return true;
    }
    return false;
}