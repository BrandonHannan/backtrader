#ifndef BREAKOUTCONTEXT_H
#define BREAKOUTCONTEXT_H

#include "../BaseContext.h"
#include <memory>
#include <cmath>
#include <queue>

using namespace std;

struct Statistic{
    double sum;
    double sumSQ;

    double sumSTD;
    double sumXSTD;
    unique_ptr<double> mean;
    unique_ptr<double> std;
    unique_ptr<double> stdSlope;
    unique_ptr<double> stdSE;

    Statistic(double sum): sum(sum), sumSQ(sum) {}
};

class BreakoutContext: public BaseContext {
    private:
        double indexSum;
        double indexSumSQ;

        double ADXThreshold;
        Statistic priceStat;
        Statistic volumeStat;
        queue<double> prices;
        queue<double> volume;
        queue<double> priceSTDs;
        queue<double> volumeSTDs;

    public:
        BreakoutContext(double ADXThreshold);

        void updateContext(StockDataInstance &currentData, StockDataInstance &previousData) override;
        unique_ptr<string> shouldExecuteTrade(StockDataInstance &data) const override;
};

#endif