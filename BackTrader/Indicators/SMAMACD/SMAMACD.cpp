#include "SMAMACD.h"

SMAMACD::SMAMACD(int lookBackPeriod, int doubleLookBackPeriod, int signalLookBackPeriod): 
        lookBackPeriod(lookBackPeriod), doubleLookBackPeriod(doubleLookBackPeriod), signalLookBackPeriod(signalLookBackPeriod),
        shortTerm(lookBackPeriod), longTerm(doubleLookBackPeriod), signal(signalLookBackPeriod) {}


double SMAMACD::getMACD() const {
    if (!this->shortTerm.isReady() || !this->longTerm.isReady()){
        return 0.0;
    }
    return this->shortTerm.getMean() - this->longTerm.getMean();
}

double SMAMACD::getSignal() const {
    if (!this->signal.isReady()){
        return 0.0;
    }
    return this->signal.getMean();
}
