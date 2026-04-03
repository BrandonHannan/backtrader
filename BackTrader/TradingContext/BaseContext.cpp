#include "BaseContext.h"

BaseContext::BaseContext(): lookBackPeriod(-1), allowedTradeTypes({}), priceStatistics(0), volumeStatistics(0) {}

BaseContext::BaseContext(int lookBackPeriod): lookBackPeriod(lookBackPeriod), allowedTradeTypes({}), priceStatistics(lookBackPeriod), volumeStatistics(lookBackPeriod) {}

BaseContext::BaseContext(int lookBackPeriod, unordered_set<string> allowedTradeTypes): lookBackPeriod(lookBackPeriod), allowedTradeTypes(allowedTradeTypes), priceStatistics(lookBackPeriod), volumeStatistics(lookBackPeriod) {}

void BaseContext::addTradeType(string tradeType){
    this->allowedTradeTypes.insert(tradeType);
}

bool BaseContext::checkStopLossPrice(const Position &currentPosition, const StockDataInstance &data) const {
    double currentPrice = data.close;
    double currentLow = data.low;
    double currentHigh = data.high;
    double stopLossPrice = currentPosition.getStopLossPrice();
    PositionType positionType = currentPosition.getPositionType();

    if (positionType.getPositiontype() == "LONG"){
        if (currentLow < currentPrice){
            currentPrice = currentLow;
        }
        if (currentPrice <= stopLossPrice){
            return true;
        }
    }
    else if (positionType.getPositiontype() == "SHORT"){
        if (currentHigh > currentPrice){
            currentPrice = currentHigh;
        }
        if (currentPrice >= stopLossPrice){
            return true;
        }
    }

    return false;
}

void BaseContext::clearBase() {
    allowedTradeTypes.clear();
    priceStatistics = WindowStatistics(this->lookBackPeriod);
    volumeStatistics = WindowStatistics(this->lookBackPeriod);
}