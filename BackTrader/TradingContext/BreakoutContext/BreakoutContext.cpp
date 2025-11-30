#include "BreakoutContext.h"

BreakoutContext::BreakoutContext(int lookbackPeriod, double priceHighPercentageThreshold, double volumeHighPercentageThreshold, double priceLowPercentageThreshold, double volumeLowPercentageThreshold): 
                                                                                                          BaseContext(lookbackPeriod), priceStatistics(lookBackPeriod), volumeStatistics(lookBackPeriod) {
    this->priceHighZ = inverseNormalCDF(priceHighPercentageThreshold);
    this->priceLowZ = inverseNormalCDF(1 - priceLowPercentageThreshold);
    this->volumeHighZ = inverseNormalCDF(volumeHighPercentageThreshold);
    this->volumeLowZ = inverseNormalCDF(1 - volumeLowPercentageThreshold);
}

void BreakoutContext::updateContext(StockDataInstance &currentData, StockDataInstance &previousData){
    double currentClose = currentData.close;
    double previousClose = previousData.close;
    double currentVolume = currentData.volume;
    double previousVolume = previousData.volume;
    
    if (priceStatistics.getSize() == 0){
        priceStatistics.addDataPoint(previousClose);
        priceStatistics.addDataPoint(currentClose);
        volumeStatistics.addDataPoint(previousVolume);
        volumeStatistics.addDataPoint(currentVolume);
    }
    else{
        priceStatistics.addDataPoint(currentClose);
        volumeStatistics.addDataPoint(currentVolume);
    }
    return;
}

string BreakoutContext::shouldExecuteTrade(StockDataInstance &data) const {
    if (!priceStatistics.isReady() || !volumeStatistics.isReady()){
        return "";
    }
    double currentClose = data.close;
    double currentVolume = data.volume;
    double maxPrice = this->priceStatistics.getMax();
    double meanPrice = this->priceStatistics.getMean();
    double stdPrice = this->priceStatistics.getStd();
    double minPrice = this->priceStatistics.getMin();
    double maxVol = this->volumeStatistics.getMax();
    double meanVol = this->volumeStatistics.getMean();
    double stdVol = this->volumeStatistics.getStd();
    double minVol = this->volumeStatistics.getMin();

    if (currentClose > maxPrice){
        if (currentClose > (meanPrice + (priceHighZ * stdPrice))){
            ;
        }
    }
    else if (currentClose < minPrice){
        ;
    }
}