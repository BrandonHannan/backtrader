#include "ATRPositionSize.h"
#include <cmath>

ATRPositionSize::ATRPositionSize(double riskAmount, int ATRPeriod, double ATRMultiplier): BasePositionSize(riskAmount),
ATRPeriod(ATRPeriod), ATRMultiplier(ATRMultiplier), atr(ATRPeriod) {}

PositionPriceInfo ATRPositionSize::calculatePositionSize(double balance, PositionType position, const StockDataInstance &data) const {
    if (atr.getATR() <= 0 || position.isNull() || data.contractSize <= 0.0){
        return PositionPriceInfo(0.0, 0.0);
    }

    double dollarRisk = balance * this->riskAmount;
    double stopLossPrice = 0.0;
    double riskPerContract = 0.0;
    double notionalPerContract = data.open * data.contractSize;
    if (notionalPerContract <= 0.0){
        return PositionPriceInfo(0.0, 0.0);
    }
    double maxContractsByBalance = balance / notionalPerContract;

    if (position.getPositiontype() == "LONG"){
        stopLossPrice = data.open - (atr.getATR() * this->ATRMultiplier);
        riskPerContract = (data.open - stopLossPrice) * data.contractSize;
        if (riskPerContract <= 0){
            // Avoid division by 0 or negative risk
            return PositionPriceInfo(0.0, 0.0);
        }
        double result = dollarRisk / riskPerContract;
        if (result > maxContractsByBalance) { result = maxContractsByBalance; }
        result = std::floor(result); // futures trade in whole contracts
        if (result <= 0){
            return PositionPriceInfo(0.0, 0.0);
        }
        return PositionPriceInfo(result, stopLossPrice);
    }
    else if (position.getPositiontype() == "SHORT"){
        stopLossPrice = data.open + (atr.getATR() * this->ATRMultiplier);
        riskPerContract = (stopLossPrice - data.open) * data.contractSize;
        if (riskPerContract <= 0){
            return PositionPriceInfo(0.0, 0.0);
        }
        double result = dollarRisk / riskPerContract;
        if (result > maxContractsByBalance) { result = maxContractsByBalance; }
        result = std::floor(result); // futures trade in whole contracts
        if (result <= 0){
            return PositionPriceInfo(0.0, 0.0);
        }
        return PositionPriceInfo(result, stopLossPrice);
    }
    else{
        return PositionPriceInfo(0.0, 0.0);
    }
}

Position ATRPositionSize::purchasePosition(double balance, const string stockName, const PositionType position, const StockDataInstance &data) const {
    PositionPriceInfo positionPriceInfo = this->calculatePositionSize(balance, position, data);
    if (position.getPositiontype() == "LONG" || position.getPositiontype() == "SHORT"){
        Position newPosition(stockName, position.getPositiontype(), "", data.date, "", data.open, -1, positionPriceInfo.numShares, positionPriceInfo.stopLossPrice, data.contractSize);
        return newPosition;
    }
    else{
        return Position();
    }
}

void ATRPositionSize::processNewData(const StockDataInstance &currentData, const StockDataInstance &previousData) {
    atr.processNewData(currentData, previousData);
}

void ATRPositionSize::updateStopLossPrice(Position &currentPosition, const StockDataInstance &data) const {

    if (atr.getATR() <= 0) {
        return;
    }

    PositionType positionType = currentPosition.getPositionType();
    double currentStopLoss = currentPosition.getStopLossPrice();

    if (positionType.getPositiontype() == "LONG"){
        double newStopLossPrice = data.high - (atr.getATR() * this->ATRMultiplier);
        if (newStopLossPrice > currentStopLoss) {
            currentPosition.setStopLossPrice(newStopLossPrice);
        }
    }
    else if (positionType.getPositiontype() == "SHORT"){
        double newStopLossPrice = data.low + (atr.getATR() * this->ATRMultiplier);

        if (newStopLossPrice < currentStopLoss) {
            currentPosition.setStopLossPrice(newStopLossPrice);
        }
    }
}

bool ATRPositionSize::isValid() const {
    return atr.isReady();
}

void ATRPositionSize::clear() {
    atr.clear();
}
