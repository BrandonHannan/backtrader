#ifndef SMAMACD_H
#define SMAMACD_H

#include "../../Functions/WindowStatistics.h"

class SMAMACD {
    private:
        int lookBackPeriod;
        int doubleLookBackPeriod;
        int signalLookBackPeriod;

        WindowStatistics shortTerm;
        WindowStatistics longTerm;
        WindowStatistics signal;

    public:
        SMAMACD(int lookBackPeriod, int doubleLookBackPeriod, int signalLookBackPeriod);

        double getMACD() const;

        double getSignal() const;
};

#endif