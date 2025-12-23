#include "BaseContext.h"

BaseContext::BaseContext(): lookBackPeriod(-1), allowedTradeTypes({}) {}

BaseContext::BaseContext(int lookBackPeriod): lookBackPeriod(lookBackPeriod), allowedTradeTypes({}) {}

BaseContext::BaseContext(int lookBackPeriod, unordered_set<string> allowedTradeTypes): lookBackPeriod(lookBackPeriod), allowedTradeTypes(allowedTradeTypes) {}

void BaseContext::addTradeType(string tradeType){
    if (this->allowedTradeTypes.find(tradeType) != this->allowedTradeTypes.end()){
        this->allowedTradeTypes.insert(tradeType);
    }
}

bool BaseContext::checkStopLossPrice(const Position &currentPosition, const StockDataInstance &data) const {
    bool result = false;
    double currentPrice = data.close;
    double stopLossPrice = currentPosition.getStopLossPrice();
    PositionType positionType = currentPosition.getPositionType();

    if (positionType.getPositiontype() == "LONG"){
        if (currentPrice <= stopLossPrice){
            return true;
        }
    }
    else if (positionType.getPositiontype() == "SHORT"){
        if (currentPrice >= stopLossPrice){
            return true;
        }
    }

    return false;
}