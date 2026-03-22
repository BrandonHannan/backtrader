#ifndef SMAMACD_H
#define SMAMACD_H

#include "../Indicator.h"
#include "../../Functions/WindowStatistics.h"

class SMAMACD: public Indicator {
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

        bool isReady() const;

        void clear() override;
};

#endif