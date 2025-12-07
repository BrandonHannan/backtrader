#include "BreakoutContext.h"

BreakoutContext::BreakoutContext(int lookbackPeriod, double priceHighPercentageThreshold, double volumeHighPercentageThreshold, double priceLowPercentageThreshold, double volumeLowPercentageThreshold): 
                                                                                                          BaseContext(lookbackPeriod), priceStatistics(lookBackPeriod), volumeStatistics(lookBackPeriod) {
    this->priceHighZ = inverseNormalCDF(priceHighPercentageThreshold);
    this->priceLowZ = inverseNormalCDF(1 - priceLowPercentageThreshold);
    this->volumeHighZ = inverseNormalCDF(volumeHighPercentageThreshold);
    this->volumeLowZ = inverseNormalCDF(1 - volumeLowPercentageThreshold);
}

void BreakoutContext::updateContext(const StockDataInstance &currentData, const StockDataInstance &previousData){
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

Trade BreakoutContext::shouldExecuteTrade(const StockDataInstance &data) const {
    if (!priceStatistics.isReady() || !volumeStatistics.isReady()){
        return Trade();
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

    // Check if the current price is greater than the maximum price of the lookback period
    if (currentClose > maxPrice){

        // Check if the current price is greater than the specified percentage of prices possible based on a normal distribution
        if (currentClose > (meanPrice + (priceHighZ * stdPrice))){

            // Check if the current volume is greater than the maximum volume of the lookback period
            if (currentVolume > maxVol){
                // Check if the current volume is greater than the specified percentage of volume possible based on a normal distribution
                // NOT IMPLEMENTED YET

                // Trade Type: LONG BREAKTHROUGH
                return Trade("LONG", "LONG BREAKTHROUGH");
            }
            else{

                // Trade Type: SHORT REVERSAL
                return Trade("SHORT", "SHORT REVERSAL");
            }
        }
    }
    else if (currentClose < minPrice){
        
        // Check if the current price is less than the specified percentage of prices possible based on a normal distribution
        if (currentClose < (meanPrice - (priceLowZ * stdPrice))){
            // Check if the current volume is greater than the specified percentage of volume possible based on a normal distribution
            // NOT IMPLEMENTED YET

            // Trade Type: SHORT BREAKTHROUGH
            return Trade("SHORT", "SHORT BREAKTHROUGH");
        }
        else{

            // Trade Type: LONG REVERSAL
            return Trade("LONG", "LONG REVERSAL");
        }
    }
    return Trade();
}

bool BreakoutContext::isValid() const {
    if (priceStatistics.isReady()){
        return true;
    }
    return false;
}