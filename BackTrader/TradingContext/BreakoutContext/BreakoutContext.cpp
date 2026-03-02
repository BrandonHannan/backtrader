#include "BreakoutContext.h"

BreakoutContext::BreakoutContext(int lookbackPeriod, double priceHighPercentageThreshold, double volumeHighPercentageThreshold, double priceLowPercentageThreshold, double volumeLowPercentageThreshold, double priceMediumPercentageThreshold): 
                                                                                                          BaseContext(lookbackPeriod), priceStatistics(lookBackPeriod), volumeStatistics(lookBackPeriod), priceHighPct(priceHighPercentageThreshold),
                                                                                                          volumeHighPct(volumeHighPercentageThreshold), priceLowPct(priceLowPercentageThreshold), volumeLowPct(volumeLowPercentageThreshold), 
                                                                                                          priceMedPct(priceMediumPercentageThreshold) {
    this->priceHighZ = inverseNormalCDF(priceHighPercentageThreshold);
    this->priceLowZ = inverseNormalCDF(1 - priceLowPercentageThreshold);
    this->volumeHighZ = inverseNormalCDF(volumeHighPercentageThreshold);
    this->volumeLowZ = inverseNormalCDF(1 - volumeLowPercentageThreshold);
    this->priceMedZ = inverseNormalCDF(priceMediumPercentageThreshold);
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

Trade BreakoutContext::shouldExecuteTrade(const StockDataInstance &currentData) const {
    if (!priceStatistics.isReady() || !volumeStatistics.isReady()){
        return Trade();
    }

    if (currentData.date == "2008-03-06"){
        double stuff = this->priceStatistics.getMax();
    }
    double currentClose = currentData.close;
    double currentVolume = currentData.volume;
    double maxPrice = this->priceStatistics.getMax();
    double meanPrice = this->priceStatistics.getMean();
    double stdPrice = this->priceStatistics.getStd();
    double minPrice = this->priceStatistics.getMin();
    double slopePrice = this->priceStatistics.getSlope();
    double rSqPrice = this->priceStatistics.getSlopeRSQ();
    bool isPriceSlopeSig = this->priceStatistics.getSlopeSignificance();
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
                if (currentVolume > (meanVol + (volumeHighZ * stdVol))){
                    // Trade Type: LONG BREAKTHROUGH
                    return Trade("LONG", "LONG BREAKTHROUGH");
                }
            }
            else{
                // Check if the current volume is greater than the specified percentage of volume possible based on a normal distribution
                if (currentVolume > (meanVol + (volumeHighZ * stdVol))){
                    // Trade Type: LONG BREAKTHROUGH
                    return Trade("LONG", "LONG BREAKTHROUGH");
                }

                // Trade Type: SHORT REVERSAL
                return Trade("SHORT", "SHORT REVERSAL");
            }
        }
    }
    else if (currentClose < minPrice){
        
        // Check if the current price is less than the specified percentage of prices possible based on a normal distribution
        if (currentClose < (meanPrice - (priceLowZ * stdPrice))){

            // Check if the current volume is greater than the maximum volume of the lookback period
            if (currentVolume > maxVol){
                // Check if the current volume is greater than the specified percentage of volume possible based on a normal distribution
                if (currentVolume > (meanVol + (volumeHighZ * stdVol))){
                    // Trade Type: SHORT BREAKTHROUGH
                    return Trade("SHORT", "SHORT BREAKTHROUGH");
                }
            }
            else{
                if (currentVolume > (meanVol + (volumeHighZ * stdVol))){
                    // Trade Type: SHORT BREAKTHROUGH
                    return Trade("SHORT", "SHORT BREAKTHROUGH");
                }

                // Trade Type: LONG REVERSAL
                return Trade("LONG", "LONG REVERSAL");
            }
        }
    }
    return Trade();
}

bool BreakoutContext::shouldSellTrade(const Position &currentPosition, const StockDataInstance &currentData) const {
    bool shouldSell = this->checkStopLossPrice(currentPosition, currentData);

    double currentClose = currentData.close;
    double currentVolume = currentData.volume;
    string currentDate = currentData.date;
    string tradeType = currentPosition.getTradeType();
    int currentTradeLength = currentPosition.currentLengthOfTrade(currentDate);

    double maxPrice = this->priceStatistics.getMax();
    double meanPrice = this->priceStatistics.getMean();
    double stdPrice = this->priceStatistics.getStd();
    double minPrice = this->priceStatistics.getMin();
    double maxVol = this->volumeStatistics.getMax();
    double meanVol = this->volumeStatistics.getMean();
    double stdVol = this->volumeStatistics.getStd();
    double minVol = this->volumeStatistics.getMin();

    // if (currentTradeLength > this->lookBackPeriod){
    //     if (tradeType == "LONG BREAKTHROUGH"){
    //         if (currentPosition.getStopLossPrice() < currentPosition.getPurchasePrice()){
    //             shouldSell = true;
    //         }
    //     }
    //     else if (tradeType == "LONG REVERSAL"){
    //         if (currentPosition.getStopLossPrice() < currentPosition.getPurchasePrice()){
    //             shouldSell = true;
    //         }
    //     }
    //     else if (tradeType == "SHORT REVERSAL"){
    //         if (currentPosition.getStopLossPrice() > currentPosition.getPurchasePrice()){
    //             shouldSell = true;
    //         }
    //     }
    //     else if (tradeType == "SHORT BREAKTHROUGH"){
    //         if (currentPosition.getStopLossPrice() > currentPosition.getPurchasePrice()){
    //             shouldSell = true;
    //         }
    //     }
    // }

    // if (tradeType == "LONG BREAKTHROUGH"){
    //     // If there is no new high in price
    //     if (currentClose < maxPrice){
    //         // And if the volume is low, then sell
    //         if ((currentVolume < minVol) || (currentVolume < (meanVol + (volumeLowZ * stdVol)))){
    //             shouldSell = true;
    //         }
    //     }
    // }
    // else if (tradeType == "SHORT REVERSAL"){
    //     // If the current price is within the medium average price of the lookback period, then sell
    //     if ((currentClose < (meanPrice + (priceMedZ * stdPrice))) && (currentClose > (meanPrice - (priceMedZ * stdPrice)))){
    //         shouldSell = true;
    //     }

    //     // If the position has not reached the stop loss price yet
    //     if (!shouldSell){
    //         // And if the volume has spiked, then sell
    //         if ((currentVolume > maxVol) || (currentVolume > (meanVol + (volumeHighZ * stdVol)))){
    //             shouldSell = true;
    //         }
    //     }
    // }
    // else if (tradeType == "SHORT BREAKTHROUGH"){
    //     // If there is no new low in price
    //     if (currentClose > minPrice){
    //         // And if the volume is low, then sell
    //         if ((currentVolume < minVol) || (currentVolume < (meanVol + (volumeLowZ * stdVol)))){
    //             shouldSell = true;
    //         }
    //     }
    // }
    // else if (tradeType == "LONG REVERSAL"){
    //     // If the current price is within the medium average price of the lookback period, the sell
    //     if ((currentClose < (meanPrice + (priceMedZ * stdPrice))) && (currentClose > (meanPrice - (priceMedZ * stdPrice)))){
    //         shouldSell = true;
    //     }

    //     // If the position has not reached the stop loss price yet
    //     if (!shouldSell){
    //         // And if the volume has spiked, then sell
    //         if ((currentVolume > maxVol) || (currentVolume > (meanVol + (volumeHighZ * stdVol)))){
    //             shouldSell = true;
    //         }
    //     }
    // }
    return shouldSell;
}

string BreakoutContext::getStats() const {
    string stats = "";
    int maxLength = 35;

    double maxPrice = this->priceStatistics.getMax();
    double meanPrice = this->priceStatistics.getMean();
    double stdPrice = this->priceStatistics.getStd();
    double minPrice = this->priceStatistics.getMin();
    double slopePrice = this->priceStatistics.getSlope();
    double slopeSEPrice = this->priceStatistics.getSlopeSE();
    double slopeRSQPrice = this->priceStatistics.getSlopeRSQ();
    bool slopeSigPrice = this->priceStatistics.getSlopeSignificance();

    double maxVol = this->volumeStatistics.getMax();
    double meanVol = this->volumeStatistics.getMean();
    double stdVol = this->volumeStatistics.getStd();
    double minVol = this->volumeStatistics.getMin();
    double slopeVolume = this->volumeStatistics.getSlope();
    double slopeSEVolume = this->volumeStatistics.getSlopeSE();
    double slopeRSQVolume = this->volumeStatistics.getSlopeRSQ();
    bool slopeSigVolume = this->volumeStatistics.getSlopeSignificance();

    stats.append(formatDoubleStat("Mean Price", meanPrice, maxLength));
    stats.append(formatDoubleStat("STD Price", stdPrice, maxLength));
    stats.append(formatDoubleStat("Min Price", minPrice, maxLength));
    stats.append(formatDoubleStat("Max Price", maxPrice, maxLength));
    stats.append(formatDoubleStat("Slope Price", slopePrice, maxLength));
    stats.append(formatDoubleStat("Slope Standard Error Price", slopeSEPrice, maxLength));
    stats.append(formatDoubleStat("Slope R^2 Price", slopeRSQPrice, maxLength));
    stats.append(formatBoolStat("Is Price Slope Significant", slopeSigPrice, maxLength));

    stats.append(formatDoubleStat("Mean Volume", meanVol, maxLength));
    stats.append(formatDoubleStat("STD Volume", stdVol, maxLength));
    stats.append(formatDoubleStat("Min Volume", minVol, maxLength));
    stats.append(formatDoubleStat("Max Volume", maxVol, maxLength));
    stats.append(formatDoubleStat("Slope Volume", slopeVolume, maxLength));
    stats.append(formatDoubleStat("Slope Standard Error Volume", slopeSEVolume, maxLength));
    stats.append(formatDoubleStat("Slope R^2 Volume", slopeRSQVolume, maxLength));
    stats.append(formatBoolStat("Is Volume Slope Significant", slopeSigVolume, maxLength));

    return stats;
}

bool BreakoutContext::isValid() const {
    if (priceStatistics.isReady() && volumeStatistics.isReady()){
        return true;
    }
    return false;
}

void BreakoutContext::clear() {
    this->clearBase();
    priceStatistics = WindowStatistics(this->lookBackPeriod);
    volumeStatistics = WindowStatistics(this->lookBackPeriod);

    priceHighZ  = inverseNormalCDF(priceHighPct);
    priceLowZ   = inverseNormalCDF(1 - priceLowPct);
    volumeHighZ = inverseNormalCDF(volumeHighPct);
    volumeLowZ  = inverseNormalCDF(1 - volumeLowPct);
    priceMedZ   = inverseNormalCDF(priceMedPct);
}