#ifndef RSICONTEXT_H
#define RSICONTEXT_H

#include "../BaseContext.h"
#include <memory>
#include <set>
#include <iterator>
#include <queue>

using namespace std;

struct PriceStatistic {
    double price1;
    double price2;

    PriceStatistic(double p1, double p2): price1(p1), price2(p2) {}
};

class RSIContext: public BaseContext {
    private:
        int RSIPeriod;
        double gainSum;
        double lossSum;
        queue<double> AvgGain;
        queue<double> AvgLoss;
        unique_ptr<double> currentRSI;
        unique_ptr<double> previousRSI;

        unique_ptr<PriceStatistic> maxPrice;
        unique_ptr<PriceStatistic> minPrice;
        queue<double> prices;
        multiset<double> pricesWindow;
        
    public:
        RSIContext(int RSIPeriod);

        void updateContext(StockDataInstance &currentData, StockDataInstance &previousData) override;
        Trade shouldExecuteTrade(const StockDataInstance &currentData) const override;
        json getContextData() const override { return json::object(); }
};

#endif