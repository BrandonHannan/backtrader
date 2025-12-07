#include "ATRPositionSize.h"

ATRPositionSize::ATRPositionSize(double riskAmount, int ATRPeriod, double ATRMultiplier): BasePositionSize(riskAmount),
ATRPeriod(ATRPeriod), ATRMultiplier(ATRMultiplier), trueRangeSum(0.0), currentATR(0.0) {}

double ATRPositionSize::calculatePositionSize(double balance, PositionType position, const StockDataInstance &data) const {
    if (this->currentATR <= 0 || position.isNull()){
        return 0.0;
    }

    double dollarRisk = balance * this->riskAmount;
    double stopLossPrice = 0.0;
    double riskPerShare = 0.0;
    if (position.getPositiontype() == "LONG"){
        stopLossPrice = data.close - (this->currentATR * this->ATRMultiplier);
        riskPerShare = data.close - stopLossPrice;
        if (riskPerShare <= 0){
            // Avoid division by 0 or negative risk
            return 0.0;
        }
        double result = dollarRisk/riskPerShare;
        return result;
    }
    else if (position.getPositiontype() == "SHORT"){
        stopLossPrice = data.close + (this->currentATR * this->ATRMultiplier);
        riskPerShare = stopLossPrice - data.close;
        if (riskPerShare <= 0){
            return 0.0;
        }
        double result = dollarRisk/riskPerShare;
        return result;
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

bool ATRPositionSize::isValid() const {
    if (this->trueRangeWindow.size() != this->ATRPeriod){
        return false;
    }
    return true;
}