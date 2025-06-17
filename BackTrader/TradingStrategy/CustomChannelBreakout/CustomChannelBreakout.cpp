#include "../TradingStrategy.h"
#include "../../StockData/StockData.h"

class CustomChannelBreakout: private TradingStrategy{
    private:
        LookBack lookBack;
        double priceVolatilityLongThreshold; // E.g. 1 - 100 Comparison
        double priceVolatilityShortThreshold; // E.g. 1 - 100 Comparison
        double volumeVolatilityLongThreshold; // E.g. 1 - 100 Comparison
        double volumeVolatilityShortThreshold; // E.g. 1 - 100 Comparison
        double volumeDropThreshold; // E.g. 0 - 1 Multiplier
        int waitingPeriod; // E.g. 1 - 10 Days
        double volumeComparison; // E.g. 0.01 - 0.25 Multiplier
        double volumeDropComparison; // E.g. 0.05 - 0.5 Multiplier
        double priceSurge; // E.g. 1.05 - 1.5 Multiplier
        double dropSurge; // E.g. 0.75 - 0.99 Multiplier
        double volumeComparisonPriceSurge; // E.g. 0.01 - 0.5 Multiplier
        double volumeComparisonDropSurge; // E.g. 0.01 - 0.5 Multiplier

        double HVSComparison; // E.g. 0.01 - 0.25 Multiplier
        double HNVSLComparison; // E.g. 1 - 100 Comparison
        double HNVHHVLComparison; // E.g. 0.01 - 0.25 Multiplier
        double LVLComparison; // E.g. 0.01 - 0.24 Multiplier
        double LNVSSComparison; // E.g. 1 - 100 Comparison
        double LNVLHVSComparison; // E.g. 0.01 - 0.25 Multiplier
    public:
        CustomChannelBreakout(double bal, bool cOP, Position pos, vector<Position> cPoses, int lbPeriod,
        double pVLT, double pVST, double vVLT, double vVST, double vDT, int wPeriod, double vC, double vDC, double pS, 
        double vCPS, double vCDS, double dS,
        double HVS, double HNVSL, double HNVHHVL, double LVL, double LNVSS, double LNVLHVS):
        TradingStrategy(bal, cOP, pos, cPoses), lookBack(LookBack(lbPeriod)), priceVolatilityLongThreshold(pVLT),
        priceVolatilityShortThreshold(pVST),
        volumeVolatilityLongThreshold(vVLT), volumeVolatilityShortThreshold(vVST), volumeDropThreshold(vDT), 
        waitingPeriod(wPeriod), volumeComparison(vC),
        volumeDropComparison(vDC), priceSurge(pS), dropSurge(dS), volumeComparisonPriceSurge(vCPS), 
        volumeComparisonDropSurge(vCDS), HVSComparison(HVS),
        HNVSLComparison(HNVSL), HNVHHVLComparison(HNVHHVL), LVLComparison(LVL), LNVSSComparison(LNVSS),
        LNVLHVSComparison(LNVLHVS) {}

        void ExecuteStrategy(const StockData &data) override {
            int length = data.close.size();
            int lookbackPeriod = this->lookBack.lookbackPeriod;
            int doubleLookbackPeriod = 2 * lookbackPeriod;
            if (length <= doubleLookbackPeriod || lookbackPeriod <= 1){
                return;
            }
            double maxPrice = -9999999;
            double minPrice = 9999999;
            double maxVol = -9999999;
            double minVol = 9999999;
            // Determine mean and standard deviation and max and mins
            for (int i = 0; i < lookbackPeriod; i++){
                lookBack.sumPricePrev += data.close[i];
                lookBack.sumSQPricePrev += data.close[i] * data.close[i];
                lookBack.sumVolPrev += data.volume[i];
                lookBack.sumSQVolPrev += data.volume[i] * data.volume[i];
                
                maxPrice = max(maxPrice, data.close[i]);
                minPrice = min(minPrice, data.close[i]);
                maxVol = max(maxVol, data.volume[i]);
                minVol = min(minVol, data.volume[i]);
            }

            for (int i = lookbackPeriod; i< doubleLookbackPeriod; i++){
                lookBack.sumPrice += data.close[i];
                lookBack.sumSQPrice += data.close[i] * data.close[i];
                lookBack.sumVol += data.volume[i];
                lookBack.sumSQVol += data.volume[i] * data.volume[i];

                maxPrice = max(maxPrice, data.close[i]);
                minPrice = min(minPrice, data.close[i]);
                maxVol = max(maxVol, data.volume[i]);
                minVol = min(minVol, data.volume[i]);
            }

            lookBack.updateMaxPrice(maxPrice);
            lookBack.updateMinPrice(minPrice);
            lookBack.updateMaxVolume(maxVol);
            lookBack.updateMinVolume(minVol);

            // Execute the trades

            for (int i = doubleLookbackPeriod; i<length; i++){
                double currentPrice = data.close[i];
                double currentVol = data.volume[i];
                double prevPrice = data.close[i - lookbackPeriod];
                double prevVol = data.volume[i - lookbackPeriod];
                double prevPrevPrice = data.close[i - doubleLookbackPeriod];
                double prevPrevVol = data.volume[i - doubleLookbackPeriod];

                // Update LookBack
                lookBack.updateLookBackSumPrice(currentPrice, prevPrice, prevPrevPrice);
                lookBack.updateLookBackSumVolume(currentVol, prevVol, prevPrevVol);

                // Determine if a trade should be executed
                if (this->getContainsOpenPosition()){
                    ;
                }
                else{
                    if (currentPrice > lookBack.maxPrice.maxOrMin){
                        // Update Max Price
                        lookBack.updateMaxPrice(currentPrice);
                        double meanPrice = lookBack.DetermineMeanPrice();
                        double stdPrice = lookBack.DetermineSTDPrice();
                        if ((meanPrice/stdPrice) > priceVolatilityLongThreshold){
                            // HVS
                            ;
                        }
                        else{
                            double meanVol = lookBack.DetermineMeanVolume();
                            double stdVol = lookBack.DetermineSTDVolume();
                            double meanPrev = lookBack.DetermineMeanPricePrev();
                            if (currentPrice * priceSurge > meanPrice){
                                double comparisonVol = meanVol + meanVol * volumeComparisonPriceSurge;
                                if (currentVol <= comparisonVol){
                                    // HNVHL
                                    ;
                                }
                                else{
                                    // HNVHHVL
                                    ;
                                }
                            }
                            else if (meanPrice > meanPrev){
                                if (meanVol/stdVol >= volumeVolatilityLongThreshold){
                                    double meanVolPrev = lookBack.DetermineMeanVolumePrev();
                                    double topComparisonVol = meanVolPrev + meanVolPrev * volumeDropComparison;
                                    double bottomComparisonVol = meanVolPrev - meanVolPrev * volumeDropComparison;
                                    if (bottomComparisonVol <= meanVol && meanVol <= topComparisonVol){
                                        double dropVolume = data.close[i - 1] * volumeDropComparison;
                                        if (dropVolume >=)
                                        // HNVSL
                                    }
                                }
                            }
                        }
                    }
                    else if (currentPrice < lookBack.minPrice.maxOrMin){
                        // Update Min Price
                        lookBack.updateMinPrice(currentPrice);
                        double meanPrice = lookBack.DetermineMeanPrice();
                        double stdPrice = lookBack.DetermineSTDPrice();
                        if ((meanPrice/stdPrice) > priceVolatilityShortThreshold){
                            // LVL
                        }
                        else{
                            double meanVol = lookBack.DetermineMeanVolume();
                            double stdVol = lookBack.DetermineSTDVolume();
                            double meanPrev = lookBack.DetermineMeanPricePrev();
                            if (currentPrice * dropSurge <= meanPrice){
                                double comparisonVol = meanVol + meanVol * this->volumeComparisonDropSurge;
                                if (currentVol <= comparisonVol){
                                    // LNVLS
                                    ;
                                }
                                else{
                                    // LNVLHVS
                                    ;
                                }
                            }
                            else if ((meanVol/stdVol) >= volumeVolatilityShortThreshold){
                                double meanVolPrev = lookBack.DetermineMeanVolumePrev();
                                double topComparisonVol = meanVolPrev + meanVolPrev * volumeDropComparison;
                                double bottomComparisonVol = meanVolPrev - meanVolPrev * volumeDropComparison;
                                if (bottomComparisonVol <= meanVol && meanVol <= topComparisonVol){
                                    // LNVSS
                                }
                            }
                        }
                    }
                }

            }

        }
};