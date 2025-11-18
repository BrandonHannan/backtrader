#include "BaseContext.h"

BaseContext::BaseContext(): lookBackPeriod(-1), allowedTradeTypes({}) {}

BaseContext::BaseContext(int lookBackPeriod): lookBackPeriod(lookBackPeriod), allowedTradeTypes({}) {}

BaseContext::BaseContext(int lookBackPeriod, unordered_set<string> allowedTradeTypes): lookBackPeriod(lookBackPeriod), allowedTradeTypes(allowedTradeTypes) {}

void BaseContext::addTradeType(string tradeType){
    if (this->allowedTradeTypes.find(tradeType) != this->allowedTradeTypes.end()){
        this->allowedTradeTypes.insert(tradeType);
    }
}