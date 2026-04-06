#ifndef RSI_H
#define RSI_H

#include "../Indicator.h"
#include <queue>

using namespace std;

// This indicator does NOT need to be reset after a position is closed.
// RSI tracks broad market momentum across the full price history and
// should continue accumulating data between trades.
class RSI : public Indicator {
    private:
        int period;
        int dayCount;
        double prevClose;
        double avgGain;
        double avgLoss;
        bool ready;
        queue<double> seedCloses; // used only during the seed phase (first period candles)

    public:
        RSI(int period);

        void processNextDay(double close);

        double getRSI() const;

        bool isReady() const;

        void clear() override;
};

#endif
