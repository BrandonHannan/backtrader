#ifndef EMAMACD_H
#define EMAMACD_H

#include "../Indicator.h"
#include "../../Functions/WindowStatistics.h"

class EMAMACD: public Indicator {
    private:
        int lookBackPeriod;
        int doubleLookBackPeriod;
        int signalLookBackPeriod;

        WindowStatistics shortTerm;
        WindowStatistics longTerm;
        WindowStatistics signal;
    
    public:
        EMAMACD(int lookBackPeriod, int doubleLookBackPeriod, int signalLookBackPeriod);

        double getMACD() const;

        double getSignal() const;

        void clear() override;
};

#endif