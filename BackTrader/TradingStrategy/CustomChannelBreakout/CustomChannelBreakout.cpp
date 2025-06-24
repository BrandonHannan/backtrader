#include "../TradingStrategy.h"
#include "../../StockData/StockData.h"

class CustomChannelBreakout: private TradingStrategy{
    private:
        double ATRMultiplier; // E.g. 1.1 - 3 Multiplier
        double RiskAmount; // E.g. 0.001 - 0.1 Multiplier

        LookBack lookBack;
        double priceVolatilityLongThreshold1; // E.g. 1 - 100 Comparison
        double priceVolatilityLongThreshold2; // E.g. 1 - 100 Comparison
        double priceVolatilityShortThreshold1; // E.g. 1 - 100 Comparison
        double priceVolatilityShortThreshold2; // E.g. 1 - 100 Comparison
        double volumeVolatilityLongThreshold; // E.g. 1 - 100 Comparison
        double volumeVolatilityShortThreshold; // E.g. 1 - 100 Comparison
        double volumeDropThreshold; // E.g. 0 - 1 Multiplier
        int waitingPeriod = 0; // E.g. 1 - 10 Days
        double volumeComparison; // E.g. 0.01 - 0.25 Multiplier
        double volumeDropComparison; // E.g. 0.05 - 0.5 Multiplier
        double priceSurge; // E.g. 1.05 - 1.5 Multiplier
        double dropPriceSurge; // E.g. 0.75 - 0.99 Multiplier
        double volumeComparisonPriceSurge; // E.g. 0.01 - 0.5 Multiplier
        double volumeComparisonDropSurge; // E.g. 0.01 - 0.5 Multiplier

        double priceDiffLongCompare; // E.g. 0.5 - 0.9 Comparison
        double priceDiffShortCompare; // E.g. 0.5 - 0.9 Comparison

        double HVSComparison; // E.g. 0.01 - 0.25 Multiplier
        double HNVSLComparison; // E.g. 1 - 100 Comparison
        double HNVHHVLComparison; // E.g. 0.01 - 0.25 Multiplier
        double LVLComparison; // E.g. 0.01 - 0.24 Multiplier
        double LNVSSComparison; // E.g. 1 - 100 Comparison
        double LNVLHVSComparison; // E.g. 0.01 - 0.25 Multiplier

        bool HVSSignal = false;
        int HVSCount = 0;
        int HVSWaitingPeriod = 0;
        double HVSVolumeDropComparison; // E.g. 0.05 - 0.5 Multiplier

        bool LVLSignal = false;
        int LVLCount = 0;
        int LVLWaitingPeriod = 0;
        double LVLVolumeDropComparison; // E.g. 0.05 - 0.5 Multiplier

        double HVSExitComparison; // E.g. 0.01 - 0.25 Multiplier
        double HVSPrevMean = 0;
        double HNVSLExitThreshold; // E.g. 1 - 100 Comparison
        int HNVSLCounter = 0;
        double HNVSLRunningSum = 0;
        double HNVSLRunningSumSquared = 0;
        double HNVHHVLExitComparison; // E.g. 0.01 - 0.25 Multiplier
        int HNVHHVLHighCounter = 0;
        double HNVHHVLHighRunningSum = 0;
        double LVLExitComparison; // E.g. 0.01 - 0.25 Multiplier
        double LVLPrevMean = 0;
        double LNVSSExitThreshold; // E.g. 1 - 100 Comparison
        int LNVSSCounter = 0;
        double LNVSSRunningSum = 0;
        double LNVSSRunningSumSquared = 0;
        double LNVLHVSExitComparison; // E.g. 0.01 - 0.25 Multiplier
        int LNVLHVSLowCounter = 0;
        double LNVLHVSLowRunningSum = 0;
    public:
        CustomChannelBreakout(double bal, bool cOP, Position pos, vector<Position> cPoses, int lbPeriod,
        int ATRP, double ATRM, double rM, double pVLT1, double pVLT2, double pVST1, double pVST2, double vVLT, 
        double vVST, double vDT, int wPeriod, double vC, double vDC, double pS, double vCPS, double vCDS, 
        double pDLC, double pDSC, double dPS, double HVS, double HNVSL, double HNVHHVL, double LVL, double LNVSS, 
        double LNVLHVS, int HVSWP, double HVSVDC, int LVLWP, double LVLVC, double HVSEC, double HNVSLET, 
        double HNVHHVLEC, double LVLEC, double LNVSSET, double LNVLHVSEC):
        TradingStrategy(bal, cOP, pos, cPoses), ATRMultiplier(ATRM), RiskAmount(rM), lookBack(LookBack(lbPeriod, ATRP)), 
        priceVolatilityLongThreshold1(pVLT1), priceVolatilityLongThreshold2(pVLT2), priceVolatilityShortThreshold1(pVST1), 
        priceVolatilityShortThreshold2(pVST2), volumeVolatilityLongThreshold(vVLT), volumeVolatilityShortThreshold(vVST), 
        volumeDropThreshold(vDT), waitingPeriod(wPeriod), volumeComparison(vC), volumeDropComparison(vDC), priceSurge(pS), 
        dropPriceSurge(dPS), volumeComparisonPriceSurge(vCPS), volumeComparisonDropSurge(vCDS), priceDiffLongCompare(pDLC), 
        priceDiffShortCompare(pDSC), HVSComparison(HVS), HNVSLComparison(HNVSL), HNVHHVLComparison(HNVHHVL), 
        LVLComparison(LVL), LNVSSComparison(LNVSS), LNVLHVSComparison(LNVLHVS), HVSWaitingPeriod(HVSWP), 
        HVSCount(0), HVSVolumeDropComparison(HVSVDC), LVLWaitingPeriod(LVLWP), LVLSignal(false), LVLCount(0), 
        LVLVolumeDropComparison(LVLVC), HVSExitComparison(HVSEC), HNVSLExitThreshold(HNVSLET), 
        HNVHHVLExitComparison(HNVHHVLEC), LVLExitComparison(LVLEC), LNVSSExitThreshold(LNVSSET), 
        LNVLHVSExitComparison(LNVLHVSEC) {}

        double DetermineShares(double currentPrice){
            double dollarRisk = this->balance * this->RiskAmount;
            if (dollarRisk <= 0){
                return 0.0;
            }
            double stopLoss = currentPrice - (lookBack.DetermineATR() * this->ATRMultiplier);
            double riskPerShare = currentPrice - stopLoss;
            if (riskPerShare <= 0){
                return 0.0;
            }
            return (dollarRisk/riskPerShare);
        }

        void sellPosition(double sellPrice, string sellDate){
            Position currentPosition = this->getOpenPosition();
            currentPosition.setIsClosed(true);
            currentPosition.setSellDate(sellDate);
            currentPosition.setSellPrice(sellPrice);
            if (currentPosition.getPositionType() == LONG){
                    this->balance += (currentPosition.getSellPrice() - currentPosition.getPurchasePrice()) 
                    * currentPosition.getNumShares();
            }
            else if (currentPosition.getPositionType() == SHORT){
                this->balance += (currentPosition.getPurchasePrice() - currentPosition.getSellPrice()) 
                * currentPosition.getNumShares();
            }
            this->setOpenPosition(currentPosition);
            this->setContainsOpenPosition(false);
            this->appendClosedPosition(currentPosition);
        }

        void ExecuteStrategy(const StockData &data) override {
            int length = data.close.size();
            int lookbackPeriod = this->lookBack.lookbackPeriod;
            int doubleLookbackPeriod = 2 * lookbackPeriod;
            int ATRLookbackPeriod = this->lookBack.ATRLookbackPeriod;
            if (length <= doubleLookbackPeriod || lookbackPeriod <= 1 
                || length <= ATRLookbackPeriod || ATRLookbackPeriod < 1){
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
                lookBack.sumDiffPricePrev += (data.high[i] - data.low[i])/2;
                
                maxPrice = max(maxPrice, data.close[i]);
                minPrice = min(minPrice, data.close[i]);
                maxVol = max(maxVol, data.volume[i]);
                minVol = min(minVol, data.volume[i]);

                double high_minus_low = data.high[i] - data.low[i];
                double high_minus_prev_close = abs(data.high[i] - data.close[i - 1]);
                double low_minus_prev_close = abs(data.low[i] - data.close[i - 1]);
                lookBack.updateATR(max(high_minus_low, high_minus_prev_close, low_minus_prev_close));
            }

            for (int i = lookbackPeriod; i < doubleLookbackPeriod; i++){
                lookBack.sumPrice += data.close[i];
                lookBack.sumSQPrice += data.close[i] * data.close[i];
                lookBack.sumVol += data.volume[i];
                lookBack.sumSQVol += data.volume[i] * data.volume[i];
                lookBack.sumDiffPrice += (data.high[i] - data.low[i])/2;

                maxPrice = max(maxPrice, data.close[i]);
                minPrice = min(minPrice, data.close[i]);
                maxVol = max(maxVol, data.volume[i]);
                minVol = min(minVol, data.volume[i]);

                double high_minus_low = data.high[i] - data.low[i];
                double high_minus_prev_close = abs(data.high[i] - data.close[i - 1]);
                double low_minus_prev_close = abs(data.low[i] - data.close[i - 1]);
                lookBack.updateATR(max(high_minus_low, high_minus_prev_close, low_minus_prev_close));
            }

            lookBack.updateMaxPrice(maxPrice);
            lookBack.updateMinPrice(minPrice);
            lookBack.updateMaxVolume(maxVol);
            lookBack.updateMinVolume(minVol);

            int startingIndex = doubleLookbackPeriod;

            if (ATRLookbackPeriod > doubleLookbackPeriod){
                for (int i = startingIndex; i < ATRLookbackPeriod; i++){
                    double currentPrice = data.close[i];
                    double currentVol = data.volume[i];
                    double prevPrice = data.close[i - lookbackPeriod];
                    double prevVol = data.volume[i - lookbackPeriod];
                    double prevPrevPrice = data.close[i - doubleLookbackPeriod];
                    double prevPrevVol = data.volume[i - doubleLookbackPeriod];
                    double currentDiffPrice = (data.high[i] - data.low[i])/2;
                    double prevDiffPrice = (data.high[i - lookbackPeriod] - data.low[i - lookbackPeriod])/2;
                    double prevPrevDiffPrice = (data.high[i - doubleLookbackPeriod] - data.low[i - doubleLookbackPeriod])/2;

                    // Update LookBack
                    lookBack.updateLookBackSumPrice(currentPrice, prevPrice, prevPrevPrice);
                    lookBack.updateLookBackSumVolume(currentVol, prevVol, prevPrevVol);
                    lookBack.updateLookBackSumDiffPrice(currentDiffPrice, prevDiffPrice, prevPrevDiffPrice);

                    lookBack.updateMaxPrice(currentPrice);
                    lookBack.updateMinPrice(currentPrice);
                    lookBack.updateMaxVolume(currentVol);
                    lookBack.updateMinVolume(currentVol);

                    // Update ATR
                    double high_minus_low = data.high[i] - data.low[i];
                    double high_minus_prev_close = abs(data.high[i] - data.close[i - 1]);
                    double low_minus_prev_close = abs(data.low[i] - data.close[i - 1]);
                    lookBack.updateATR(max(high_minus_low, high_minus_prev_close, low_minus_prev_close));
                }
                startingIndex = ATRLookbackPeriod;
            }

            // Execute the trades
            for (int i = startingIndex; i < length; i++){
                double currentPrice = data.close[i];
                double currentVol = data.volume[i];
                double prevPrice = data.close[i - lookbackPeriod];
                double prevVol = data.volume[i - lookbackPeriod];
                double prevPrevPrice = data.close[i - doubleLookbackPeriod];
                double prevPrevVol = data.volume[i - doubleLookbackPeriod];
                double currentDiffPrice = (data.high[i] - data.low[i])/2;
                double prevDiffPrice = (data.high[i - lookbackPeriod] - data.low[i - lookbackPeriod])/2;
                double prevPrevDiffPrice = (data.high[i - doubleLookbackPeriod] - data.low[i - doubleLookbackPeriod])/2;

                // Update LookBack
                lookBack.updateLookBackSumPrice(currentPrice, prevPrice, prevPrevPrice);
                lookBack.updateLookBackSumVolume(currentVol, prevVol, prevPrevVol);
                lookBack.updateLookBackSumDiffPrice(currentDiffPrice, prevDiffPrice, prevPrevDiffPrice);

                // Update ATR
                double high_minus_low = data.high[i] - data.low[i];
                double high_minus_prev_close = abs(data.high[i] - data.close[i - 1]);
                double low_minus_prev_close = abs(data.low[i] - data.close[i - 1]);
                lookBack.updateATR(max(high_minus_low, high_minus_prev_close, low_minus_prev_close));

                // Determine if a trade should be executed
                if (this->getContainsOpenPosition()){
                    Position currentPosition = this->getOpenPosition();
                    double stopLossPrice = currentPosition.getStopLossPrice();
                    if (i >= length - 1){
                        this->sellPosition(currentPrice, data.date[i]);
                        // Goes to the next day
                        lookBack.updateMaxPrice(maxPrice);
                        lookBack.updateMinPrice(minPrice);
                        lookBack.updateMaxVolume(maxVol);
                        lookBack.updateMinVolume(minVol);
                    }
                    // Sells position based on its stop loss price
                    if ((currentPosition.getPositionType() == LONG && currentPrice <= stopLossPrice) ||
                        (currentPosition.getPositionType() == SHORT && currentPrice >= stopLossPrice)){
                        double sellPrice = currentPrice;
                        if ((currentPosition.getPositionType() == LONG && data.open[i] < currentPrice && data.open[i] <= stopLossPrice) ||
                            (currentPosition.getPositionType() == SHORT && data.open[i] > currentPrice && data.open[i] >= stopLossPrice)){
                            currentPosition.setSellPrice(data.open[i]);
                        }
                        this->sellPosition(sellPrice, data.date[i]);
                        // Goes to the next day
                        lookBack.updateMaxPrice(maxPrice);
                        lookBack.updateMinPrice(minPrice);
                        lookBack.updateMaxVolume(maxVol);
                        lookBack.updateMinVolume(minVol);
                        continue;
                    }

                    TradeType tradeType = currentPosition.getTradeType();

                    if (tradeType == HVS){
                        double bottomMeanPrice = this->HVSPrevMean + this->HVSPrevMean * this->HVSExitComparison;
                        if (bottomMeanPrice >= currentPrice){
                            double sellPrice = currentPrice;
                            if ((currentPosition.getPositionType() == LONG && data.open[i] < currentPrice && data.open[i] <= stopLossPrice) ||
                                (currentPosition.getPositionType() == SHORT && data.open[i] > currentPrice && data.open[i] >= stopLossPrice)){
                                currentPosition.setSellPrice(data.open[i]);
                            }
                            this->sellPosition(sellPrice, data.date[i]);
                        }
                    }
                    else if (tradeType == HNVHL){
                        ;
                    }
                    else if (tradeType == HNVHHVL){
                        this->HNVHHVLHighCounter += 1;
                        this->HNVHHVLHighRunningSum += currentVol;
                        double meanHigh = (this->HNVHHVLHighRunningSum/this->HNVHHVLHighCounter) + 
                               (this->HNVHHVLHighRunningSum/this->HNVHHVLHighCounter) * this->HNVHHVLExitComparison;
                        if (currentVol <= meanHigh){
                            double sellPrice = currentPrice;
                            if ((currentPosition.getPositionType() == LONG && data.open[i] < currentPrice && data.open[i] <= stopLossPrice) ||
                                (currentPosition.getPositionType() == SHORT && data.open[i] > currentPrice && data.open[i] >= stopLossPrice)){
                                currentPosition.setSellPrice(data.open[i]);
                            }
                            this->sellPosition(sellPrice, data.date[i]);
                            this->HNVHHVLHighCounter = 0;
                            this->HNVHHVLHighRunningSum = 0;
                        }
                    }
                    else if (tradeType == HNVSL){
                        this->HNVSLCounter += 1;
                        this->HNVSLRunningSum += currentVol;
                        this->HNVSLRunningSumSquared += currentVol * currentVol;
                        double HNVSLMean = (this->HNVSLRunningSum/this->HNVSLCounter);
                        double HNVSLVariance = (this->HNVSLRunningSumSquared - (this->HNVSLRunningSum * 
                        this->HNVSLRunningSum)/this->HNVSLCounter)/(this->HNVSLCounter - 1);
                        double HNVSLSTD = sqrt(HNVSLVariance);
                        if (HNVSLMean/HNVSLSTD < HNVSLExitThreshold){
                            double sellPrice = currentPrice;
                            if ((currentPosition.getPositionType() == LONG && data.open[i] < currentPrice && data.open[i] <= stopLossPrice) ||
                                (currentPosition.getPositionType() == SHORT && data.open[i] > currentPrice && data.open[i] >= stopLossPrice)){
                                currentPosition.setSellPrice(data.open[i]);
                            }
                            this->sellPosition(sellPrice, data.date[i]);
                            this->HNVSLCounter = 0;
                            this->HNVSLRunningSum = 0;
                            this->HNVSLRunningSumSquared = 0;
                        }
                    }
                    else if (tradeType == LVL){
                        double bottomMeanPrice = this->LVLPrevMean - this->LVLPrevMean * this->LVLExitComparison;
                        if (bottomMeanPrice < currentPrice){
                            double sellPrice = currentPrice;
                            if ((currentPosition.getPositionType() == LONG && data.open[i] < currentPrice && data.open[i] <= stopLossPrice) ||
                                (currentPosition.getPositionType() == SHORT && data.open[i] > currentPrice && data.open[i] >= stopLossPrice)){
                                currentPosition.setSellPrice(data.open[i]);
                            }
                            this->sellPosition(sellPrice, data.date[i]);
                        }
                    }
                    else if (tradeType == LNVLS){
                        ;
                    }
                    else if (tradeType == LNVLHVS){
                        this->LNVLHVSLowCounter += 1;
                        this->LNVLHVSLowRunningSum += currentVol;
                        double meanHigh = (this->LNVLHVSLowRunningSum/this->LNVLHVSLowCounter) + 
                               (this->LNVLHVSLowRunningSum/this->LNVLHVSLowCounter) * this->LNVLHVSExitComparison;
                        if (currentVol <= meanHigh){
                            double sellPrice = currentPrice;
                            if ((currentPosition.getPositionType() == LONG && data.open[i] < currentPrice && data.open[i] <= stopLossPrice) ||
                                (currentPosition.getPositionType() == SHORT && data.open[i] > currentPrice && data.open[i] >= stopLossPrice)){
                                currentPosition.setSellPrice(data.open[i]);
                            }
                            this->sellPosition(sellPrice, data.date[i]);
                            this->LNVLHVSLowCounter = 0;
                            this->LNVLHVSLowRunningSum = 0;
                        }
                    }
                    else if (tradeType == LNVSS){
                        this->LNVSSCounter += 1;
                        this->LNVSSRunningSum += currentVol;
                        this->LNVSSRunningSumSquared += currentVol * currentVol;
                        double LNVSSMean = (this->LNVSSRunningSum/this->LNVSSCounter);
                        double LNVSSVariance = (this->LNVSSRunningSumSquared - (this->LNVSSRunningSum * 
                        this->LNVSSRunningSum)/this->LNVSSCounter)/(this->LNVSSCounter - 1);
                        double LNVSSSTD = sqrt(LNVSSVariance);
                        if (LNVSSMean/LNVSSSTD < LNVSSExitThreshold){
                            double sellPrice = currentPrice;
                            if ((currentPosition.getPositionType() == LONG && data.open[i] < currentPrice && data.open[i] <= stopLossPrice) ||
                                (currentPosition.getPositionType() == SHORT && data.open[i] > currentPrice && data.open[i] >= stopLossPrice)){
                                currentPosition.setSellPrice(data.open[i]);
                            }
                            this->sellPosition(sellPrice, data.date[i]);
                            this->LNVSSCounter = 0;
                            this->LNVSSRunningSum = 0;
                            this->LNVSSRunningSumSquared = 0;
                        }
                    }
                }
                else{ // May Possibly Purchase a Position
                    if (this->HVSSignal || currentPrice > lookBack.maxPrice.maxOrMin){
                        // Update Max Price
                        lookBack.updateMaxPrice(currentPrice);
                        double meanPrice = lookBack.DetermineMeanPrice();
                        double stdPrice = lookBack.DetermineSTDPrice();
                        double meanDiffPrice = lookBack.DetermineMeanDiffPrice();
                        double meanCompare = lookBack.DetermineProbGreaterDiffPrice(meanPrice + meanDiffPrice);
                        if (this->HVSSignal || (meanPrice/stdPrice) < this->priceVolatilityLongThreshold1){
                            // HVS
                            if (!this->HVSSignal){
                                this->HVSSignal = true;
                                lookBack.updateMaxVolume(maxVol);
                                lookBack.updateMinVolume(minVol);
                                continue;
                            }
                            this->HVSCount = this->HVSCount + 1;
                            if (this->HVSCount > this->HVSWaitingPeriod){
                                this->HVSSignal = false;
                                this->HVSCount = 0;
                                lookBack.updateMinPrice(minPrice);
                                lookBack.updateMaxVolume(maxVol);
                                lookBack.updateMinVolume(minVol);
                                continue;
                            }
                            double bottomMeanPrice = meanPrice + meanPrice * this->HVSExitComparison;
                            if (currentPrice < lookBack.maxPrice.maxOrMin && currentPrice > bottomMeanPrice){
                                double prevVolume = data.volume[i - 1];
                                if (currentVol <= this->HVSVolumeDropComparison * prevVolume){
                                    double numShares = this->DetermineShares(currentPrice);
                                    if (numShares == 0){ 
                                        this->HVSSignal = false;
                                        this->HVSCount = 0;
                                        lookBack.updateMinPrice(minPrice);
                                        lookBack.updateMaxVolume(maxVol);
                                        lookBack.updateMinVolume(minVol);
                                        continue; 
                                    }
                                    double stopLossPrice = currentPrice + lookBack.DetermineATR() * ATRMultiplier;
                                    Position newPosition(SHORT, HVS, data.date[i], "", currentPrice, stopLossPrice, 0, 
                                            numShares, false);
                                    this->setOpenPosition(newPosition);
                                    this->setContainsOpenPosition(true);
                                    this->balance -= numShares * currentPrice;
                                    this->HVSPrevMean = meanPrice;
                                    this->HVSCount = 0;
                                    this->HVSSignal = false;
                                }
                            }
                        }
                        else if (((meanPrice/stdPrice) >= this->priceVolatilityLongThreshold2) && 
                                meanCompare < this->priceDiffLongCompare){
                            double meanVol = lookBack.DetermineMeanVolume();
                            double stdVol = lookBack.DetermineSTDVolume();
                            double meanPrev = lookBack.DetermineMeanPricePrev();
                            if (currentPrice >= (meanPrice * priceSurge)){
                                double comparisonVol = meanVol + meanVol * this->volumeComparisonPriceSurge;
                                if (currentVol <= comparisonVol){
                                    // HNVHL
                                    double numShares = this->DetermineShares(currentPrice);
                                    if (numShares == 0){ 
                                        lookBack.updateMaxVolume(maxVol);
                                        lookBack.updateMinVolume(minVol);
                                        continue; 
                                    }
                                    double stopLossPrice = currentPrice - lookBack.DetermineATR() * ATRMultiplier;
                                    Position newPosition(LONG, HNVHL, data.date[i], "", currentPrice, stopLossPrice, 0, 
                                            numShares, false);
                                    this->setOpenPosition(newPosition);
                                    this->setContainsOpenPosition(true);
                                    this->balance -= numShares * currentPrice;
                                }
                                else{
                                    // HNVHHVL
                                    double numShares = this->DetermineShares(currentPrice);
                                    if (numShares == 0){ 
                                        lookBack.updateMaxVolume(maxVol);
                                        lookBack.updateMinVolume(minVol);
                                        continue; 
                                    }
                                    double stopLossPrice = currentPrice - lookBack.DetermineATR() * ATRMultiplier;
                                    Position newPosition(LONG, HNVHHVL, data.date[i], "", currentPrice, stopLossPrice, 0, 
                                            numShares, false);
                                    this->setOpenPosition(newPosition);
                                    this->setContainsOpenPosition(true);
                                    this->balance -= numShares * currentPrice;
                                    this->HNVHHVLHighCounter += 1;
                                    this->HNVHHVLHighRunningSum += currentVol;
                                }
                            }
                            else if (meanPrice > meanPrev){
                                if ((meanVol/stdVol) >= this->volumeVolatilityLongThreshold){
                                    double meanVolPrev = lookBack.DetermineMeanVolumePrev();
                                    double topComparisonVol = meanVolPrev + meanVolPrev * this->volumeComparison;
                                    double bottomComparisonVol = meanVolPrev - meanVolPrev * this->volumeComparison;
                                    if (bottomComparisonVol <= meanVol && meanVol <= topComparisonVol){
                                        double prevVolume = this->volumeDropComparison * data.volume[i - 1];
                                        if (currentVol < prevVolume){
                                            // HNVSL
                                            double numShares = this->DetermineShares(currentPrice);
                                            if (numShares == 0){ 
                                                lookBack.updateMaxVolume(maxVol);
                                                lookBack.updateMinVolume(minVol);
                                                continue; 
                                            }
                                            double stopLossPrice = currentPrice - lookBack.DetermineATR() * ATRMultiplier;
                                            Position newPosition(LONG, HNVSL, data.date[i], "", currentPrice, stopLossPrice, 0, 
                                                    numShares, false);
                                            this->setOpenPosition(newPosition);
                                            this->setContainsOpenPosition(true);
                                            this->balance -= numShares * currentPrice;
                                            this->HNVSLCounter += 1;
                                            this->HNVSLRunningSum += currentVol;
                                            this->HNVSLRunningSumSquared += currentVol * currentVol;
                                        }
                                    }
                                }
                            }
                        }
                    }
                    else if (this->LVLSignal || currentPrice < lookBack.minPrice.maxOrMin){
                        // Update Min Price
                        lookBack.updateMinPrice(currentPrice);
                        double meanPrice = lookBack.DetermineMeanPrice();
                        double stdPrice = lookBack.DetermineSTDPrice();
                        double meanDiffPrice = lookBack.DetermineMeanDiffPrice();
                        double meanCompare = lookBack.DetermineProbGreaterDiffPrice(meanPrice + meanDiffPrice);
                        if (this->LVLSignal || (meanPrice/stdPrice) < this->priceVolatilityShortThreshold1){
                            // LVL
                            if (!this->LVLSignal){
                                this->LVLSignal = true;
                                lookBack.updateMaxVolume(maxVol);
                                lookBack.updateMinVolume(minVol);
                                continue;
                            }
                            this->LVLCount = this->LVLCount + 1;
                            if (this->LVLCount > this->LVLWaitingPeriod){
                                this->LVLSignal = false;
                                this->LVLCount = 0;
                                lookBack.updateMaxPrice(minPrice);
                                lookBack.updateMaxVolume(maxVol);
                                lookBack.updateMinVolume(minVol);
                                continue;
                            }
                            double bottomMeanPrice = meanPrice - meanPrice * this->LVLExitComparison;
                            if (currentPrice > lookBack.minPrice.maxOrMin && currentPrice < bottomMeanPrice){
                                double prevVolume = data.volume[i - 1];
                                if (currentVol <= this->LVLVolumeDropComparison * prevVolume){
                                    double numShares = this->DetermineShares(currentPrice);
                                    if (numShares == 0){ 
                                        lookBack.updateMaxPrice(minPrice);
                                        lookBack.updateMaxVolume(maxVol);
                                        lookBack.updateMinVolume(minVol);
                                        this->LVLSignal = false;
                                        this->LVLCount = 0;
                                        continue; 
                                    }
                                    double stopLossPrice = currentPrice - lookBack.DetermineATR() * ATRMultiplier;
                                    Position newPosition(LONG, LVL, data.date[i], "", currentPrice, stopLossPrice, 0, 
                                            numShares, false);
                                    this->setOpenPosition(newPosition);
                                    this->setContainsOpenPosition(true);
                                    this->balance -= numShares * currentPrice;
                                    this->LVLPrevMean = meanPrice;
                                    this->LVLCount = 0;
                                    this->LVLSignal = false;
                                }
                            }
                        }
                        else if (((meanPrice/stdPrice) >= this->priceVolatilityShortThreshold2) && 
                                meanCompare < this->priceDiffShortCompare){
                            double meanVol = lookBack.DetermineMeanVolume();
                            double stdVol = lookBack.DetermineSTDVolume();
                            double meanPrev = lookBack.DetermineMeanPricePrev();
                            if (currentPrice <= (meanPrice * this->dropPriceSurge)){
                                double comparisonVol = meanVol + meanVol * this->volumeComparisonDropSurge;
                                if (currentVol <= comparisonVol){
                                    // LNVLS
                                    double numShares = this->DetermineShares(currentPrice);
                                    if (numShares == 0){ 
                                        lookBack.updateMaxVolume(maxVol);
                                        lookBack.updateMinVolume(minVol);
                                        continue; 
                                    }
                                    double stopLossPrice = currentPrice + lookBack.DetermineATR() * ATRMultiplier;
                                    Position newPosition(SHORT, LNVLS, data.date[i], "", currentPrice, stopLossPrice, 0, 
                                            numShares, false);
                                    this->setOpenPosition(newPosition);
                                    this->setContainsOpenPosition(true);
                                    this->balance -= numShares * currentPrice;
                                }
                                else{
                                    // LNVLHVS
                                    double numShares = this->DetermineShares(currentPrice);
                                    if (numShares == 0){
                                        lookBack.updateMaxVolume(maxVol);
                                        lookBack.updateMinVolume(minVol);
                                        continue; 
                                    }
                                    double stopLossPrice = currentPrice + lookBack.DetermineATR() * ATRMultiplier;
                                    Position newPosition(SHORT, LNVLHVS, data.date[i], "", currentPrice, stopLossPrice, 0, 
                                            numShares, false);
                                    this->setOpenPosition(newPosition);
                                    this->setContainsOpenPosition(true);
                                    this->balance -= numShares * currentPrice;
                                    this->LNVLHVSLowCounter += 1;
                                    this->LNVLHVSLowRunningSum += currentVol;
                                }
                            }
                            else if ((meanVol/stdVol) >= this->volumeVolatilityShortThreshold){
                                double meanVolPrev = lookBack.DetermineMeanVolumePrev();
                                double topComparisonVol = meanVolPrev + meanVolPrev * this->volumeDropComparison;
                                double bottomComparisonVol = meanVolPrev - meanVolPrev * this->volumeDropComparison;
                                if (bottomComparisonVol <= meanVol && meanVol <= topComparisonVol){
                                    double prevVolume = this->volumeDropComparison * data.volume[i - 1];
                                    if (currentVol < prevVolume){
                                        // LNVSS
                                        double numShares = this->DetermineShares(currentPrice);
                                        if (numShares == 0){ 
                                            lookBack.updateMaxVolume(maxVol);
                                            lookBack.updateMinVolume(minVol);
                                            continue; 
                                        }
                                        double stopLossPrice = currentPrice + lookBack.DetermineATR() * ATRMultiplier;
                                        Position newPosition(SHORT, LNVSS, data.date[i], "", currentPrice, stopLossPrice, 0, 
                                                numShares, false);
                                        this->setOpenPosition(newPosition);
                                        this->setContainsOpenPosition(true);
                                        this->balance -= numShares * currentPrice;
                                        this->LNVSSCounter += 1;
                                        this->LNVSSRunningSum += currentVol;
                                        this->LNVSSRunningSumSquared += currentVol * currentVol;
                                    }
                                }
                            }
                        }
                    }
                }

                lookBack.updateMaxPrice(maxPrice);
                lookBack.updateMinPrice(minPrice);
                lookBack.updateMaxVolume(maxVol);
                lookBack.updateMinVolume(minVol);

            }
        }
};