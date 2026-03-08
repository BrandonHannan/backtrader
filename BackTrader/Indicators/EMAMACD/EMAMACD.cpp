#include "EMAMACD.h"

EMAMACD::EMAMACD(int lookBackPeriod, int doubleLookBackPeriod, int signalLookBackPeriod): 
        lookBackPeriod(lookBackPeriod), doubleLookBackPeriod(doubleLookBackPeriod), signalLookBackPeriod(signalLookBackPeriod),
        shortTerm(lookBackPeriod), longTerm(doubleLookBackPeriod), signal(signalLookBackPeriod) {}

double EMAMACD::getMACD() const {
    if (!this->shortTerm.isReady() || !this->longTerm.isReady()){
        return 0.0;
    }
    return this->shortTerm.getEMAMean() - this->longTerm.getEMAMean();
}

double EMAMACD::getSignal() const {
    if (!this->signal.isReady()){
        return 0.0;
    }
    return this->signal.getEMAMean();
}