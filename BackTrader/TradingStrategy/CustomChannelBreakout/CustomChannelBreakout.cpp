#include "CustomChannelBreakout.h"
#include <algorithm>
#include <initializer_list>
#include <cmath>

using namespace std;

CustomChannelBreakout::CustomChannelBreakout(double bal, bool cOP, Position pos, vector<Position> cPoses, int lbPeriod,
        int ATRP, double ATRM, double rM, double vC, double vDC, double pS, double vCPS, double vCDS,
        double pDLC, double pDSC, double dPS, int HVSWP, double HVSVDC, int LVLWP, double LVLVC, double HVSEC,
        double HNVSLET, double HNVSLEC, double HNVHHVLEC, double LVLEC, double LNVSSEC, double LNVSSET, 
        double LNVLHVSEC, double pPLC, double pPLT, double pPSC, double pPST, double vPLC, double vPSC, 
        double vPLT, double vPST)
    // Initialize in the order of declaration in the .h file
    : TradingStrategy(bal, cOP, pos, cPoses),
      ATRMultiplier(ATRM),
      RiskAmount(rM),
      lookBack(LookBack(lbPeriod, ATRP)),
      volumeComparison(vC),
      volumeDropComparison(vDC),
      priceSurge(pS),
      dropPriceSurge(dPS),
      volumeComparisonPriceSurge(vCPS),
      volumeComparisonDropSurge(vCDS),
      priceDiffLongCompare(pDLC),
      priceDiffShortCompare(pDSC),
      HVSSignal(false),
      HVSCount(0),
      HVSWaitingPeriod(HVSWP),
      HVSVolumeDropComparison(HVSVDC),
      LVLSignal(false),
      LVLCount(0),
      LVLWaitingPeriod(LVLWP),
      LVLVolumeDropComparison(LVLVC),
      HVSExitComparison(HVSEC),
      HVSPrevMean(0),
      HNVSLExitComparison(HNVSLEC),
      HNVSLExitThreshold(HNVSLET),
      HNVSLCounter(0),
      HNVSLRunningSum(0),
      HNVSLRunningSumSquared(0),
      HNVHHVLExitComparison(HNVHHVLEC),
      HNVHHVLHighCounter(0),
      HNVHHVLHighRunningSum(0),
      LVLExitComparison(LVLEC),
      LVLPrevMean(0),
      LNVSSExitComparison(LNVSSEC),
      LNVSSExitThreshold(LNVSSET),
      LNVSSCounter(0),
      LNVSSRunningSum(0),
      LNVSSRunningSumSquared(0),
      LNVLHVSExitComparison(LNVLHVSEC),
      LNVLHVSLowCounter(0),
      LNVLHVSLowRunningSum(0),
      pricePercentageLongComparison(pPLC),
      pricePercentageLongThreshold(pPLT),
      pricePercentageShortComparison(pPSC),
      pricePercentageShortThreshold(pPST),
      volumePercentageLongComparison(vPLC),
      volumePercentageShortComparison(vPSC),
      volumePercentageLongThreshold(vPLT),
      volumePercentageShortThreshold(vPST) {}


double CustomChannelBreakout::DetermineShares(double currentPrice) {
    double dollarRisk = this->balance * this->RiskAmount;
    if (dollarRisk <= 0) {
        return 0.0;
    }
    double stopLoss = currentPrice - (lookBack.DetermineATR() * this->ATRMultiplier);
    double riskPerShare = currentPrice - stopLoss;
    if (riskPerShare <= 0) {
        return 0.0;
    }
    return (dollarRisk / riskPerShare);
}

double CustomChannelBreakout::ProbabilityVolatility(double mean, double std, double val){
    if (std == 0){
        return 1;
    }
    double zScore = (val - mean)/std;
    return 2 * (1.0 - 0.5 * (1.0 + erf(zScore / sqrt(2.0))));
}

void CustomChannelBreakout::sellPosition(double sellPrice, string sellDate) {
    Position currentPosition = this->getOpenPosition();
    currentPosition.setIsClosed(true);
    currentPosition.setSellDate(sellDate);
    currentPosition.setSellPrice(sellPrice);
    if (currentPosition.getPositionType() == LONG) {
        this->balance += currentPosition.getSellPrice() * currentPosition.getNumShares();
    }
    else if (currentPosition.getPositionType() == SHORT) {
        this->balance -= currentPosition.getSellPrice() * currentPosition.getNumShares();
    }
    this->setOpenPosition(currentPosition);
    this->setContainsOpenPosition(false);
    this->appendClosedPosition(currentPosition);
}

void CustomChannelBreakout::ExecuteStrategy(const StockData &data){
    int length = data.close.size();
    int lookbackPeriod = this->lookBack.lookbackPeriod;
    int doubleLookbackPeriod = 2 * lookbackPeriod;
    int ATRLookbackPeriod = this->lookBack.ATRLookbackPeriod;
    if (length <= doubleLookbackPeriod || lookbackPeriod <= 1 
        || length <= ATRLookbackPeriod || ATRLookbackPeriod < 1){
        return;
    }

    // Determine mean and standard deviation and max and mins
    for (int i = 0; i < lookbackPeriod; i++){
        lookBack.sumPricePrev += data.close[i];
        lookBack.sumSQPricePrev += data.close[i] * data.close[i];
        lookBack.sumVolPrev += data.volume[i];
        lookBack.sumSQVolPrev += data.volume[i] * data.volume[i];
        lookBack.sumDiffPricePrev += (data.high[i] - data.low[i])/2;

        double high_minus_low = data.high[i] - data.low[i];
        double high_minus_prev_close = abs(data.high[i] - data.close[i - 1]);
        double low_minus_prev_close = abs(data.low[i] - data.close[i - 1]);
        lookBack.updateATR(max({high_minus_low, high_minus_prev_close, low_minus_prev_close}));

        lookBack.updateMaxPrice(data.close, i);
        lookBack.updateMinPrice(data.close, i);
        lookBack.updateMaxVolume(data.volume, i);
        lookBack.updateMinVolume(data.volume, i);
    }

    for (int i = lookbackPeriod; i < doubleLookbackPeriod; i++){
        lookBack.sumPrice += data.close[i];
        lookBack.sumSQPrice += data.close[i] * data.close[i];
        lookBack.sumVol += data.volume[i];
        lookBack.sumSQVol += data.volume[i] * data.volume[i];
        lookBack.sumDiffPrice += (data.high[i] - data.low[i])/2;

        double high_minus_low = data.high[i] - data.low[i];
        double high_minus_prev_close = abs(data.high[i] - data.close[i - 1]);
        double low_minus_prev_close = abs(data.low[i] - data.close[i - 1]);
        lookBack.updateATR(max({high_minus_low, high_minus_prev_close, low_minus_prev_close}));

        lookBack.updateMaxPrice(data.close, i);
        lookBack.updateMinPrice(data.close, i);
        lookBack.updateMaxVolume(data.volume, i);
        lookBack.updateMinVolume(data.volume, i);
    }

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

            lookBack.updateMaxPrice(data.close, i);
            lookBack.updateMinPrice(data.close, i);
            lookBack.updateMaxVolume(data.volume, i);
            lookBack.updateMinVolume(data.volume, i);

            // Update ATR
            double high_minus_low = data.high[i] - data.low[i];
            double high_minus_prev_close = abs(data.high[i] - data.close[i - 1]);
            double low_minus_prev_close = abs(data.low[i] - data.close[i - 1]);
            lookBack.updateATR(max({high_minus_low, high_minus_prev_close, low_minus_prev_close}));
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
        lookBack.updateATR(max({high_minus_low, high_minus_prev_close, low_minus_prev_close}));

        if (i >= length - 1){
            if (this->getContainsOpenPosition()){
                this->sellPosition(currentPrice, data.date[i]);
            }
            continue;
        }

        // Determine if a trade should be executed
        if (this->getContainsOpenPosition()){
            Position currentPosition = this->getOpenPosition();
            double stopLossPrice = currentPosition.getStopLossPrice();
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
                lookBack.updateMaxPrice(data.close, i);
                lookBack.updateMinPrice(data.close, i);
                lookBack.updateMaxVolume(data.volume, i);
                lookBack.updateMinVolume(data.volume, i);
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
                if (this->HNVSLCounter == 1) { this->HNVSLCounter = 2; }
                double HNVSLVariance = (this->HNVSLRunningSumSquared - (this->HNVSLRunningSum * 
                this->HNVSLRunningSum)/this->HNVSLCounter)/(this->HNVSLCounter - 1);
                double HNVSLSTD = sqrt(HNVSLVariance);
                if (this->ProbabilityVolatility(HNVSLMean, HNVSLSTD, HNVSLMean * this->HNVSLExitComparison) > 
                    this->HNVSLExitThreshold){
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
                if (this->LNVSSCounter == 1) { this->LNVSSCounter = 2; }
                double LNVSSVariance = (this->LNVSSRunningSumSquared - (this->LNVSSRunningSum * 
                this->LNVSSRunningSum)/this->LNVSSCounter)/(this->LNVSSCounter - 1);
                double LNVSSSTD = sqrt(LNVSSVariance);
                if (this->ProbabilityVolatility(LNVSSMean, LNVSSSTD, LNVSSMean * this->LNVSSExitComparison) > 
                    this->LNVSSExitThreshold){
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
            if (this->HVSSignal || currentPrice > lookBack.maxPrice){
                // Update Max Price
                lookBack.updateMaxPrice(data.close, i);
                double meanPrice = lookBack.DetermineMeanPrice();
                double stdPrice = lookBack.DetermineSTDPrice();
                double meanDiffPrice = lookBack.DetermineMeanDiffPrice();
                double meanCompare = lookBack.DetermineProbGreaterDiffPrice(meanPrice + meanDiffPrice);
                if (this->HVSSignal || (this->ProbabilityVolatility(meanPrice, stdPrice, 
                    meanPrice * this->pricePercentageLongComparison)) > this->pricePercentageLongThreshold){
                    // HVS
                    if (!this->HVSSignal){
                        this->HVSSignal = true;
                        lookBack.updateMaxVolume(data.volume, i);
                        lookBack.updateMinVolume(data.volume, i);
                        continue;
                    }
                    this->HVSCount = this->HVSCount + 1;
                    if (this->HVSCount > this->HVSWaitingPeriod){
                        this->HVSSignal = false;
                        this->HVSCount = 0;
                        lookBack.updateMinPrice(data.close, i);
                        lookBack.updateMaxVolume(data.volume, i);
                        lookBack.updateMinVolume(data.volume, i);
                        continue;
                    }
                    double bottomMeanPrice = meanPrice + meanPrice * this->HVSExitComparison;
                    if (currentPrice < lookBack.maxPrice && currentPrice > bottomMeanPrice){
                        double prevVolume = data.volume[i - 1];
                        if (currentVol <= this->HVSVolumeDropComparison * prevVolume){
                            double numShares = this->DetermineShares(currentPrice);
                            if (numShares == 0){ 
                                this->HVSSignal = false;
                                this->HVSCount = 0;
                                lookBack.updateMinPrice(data.close, i);
                                lookBack.updateMaxVolume(data.volume, i);
                                lookBack.updateMinVolume(data.volume, i);
                                continue; 
                            }
                            double stopLossPrice = currentPrice + lookBack.DetermineATR() * ATRMultiplier;
                            Position newPosition(SHORT, HVS, data.date[i], "", currentPrice, stopLossPrice, 0, 
                                    numShares, false);
                            this->setOpenPosition(newPosition);
                            this->setContainsOpenPosition(true);
                            this->balance += numShares * currentPrice;
                            this->HVSPrevMean = meanPrice;
                            this->HVSCount = 0;
                            this->HVSSignal = false;
                        }
                    }
                }
                else if (((this->ProbabilityVolatility(meanPrice, stdPrice,
                        meanPrice * this->pricePercentageLongComparison)) < this->pricePercentageLongThreshold) && 
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
                                lookBack.updateMaxVolume(data.volume, i);
                                lookBack.updateMinVolume(data.volume, i);
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
                                lookBack.updateMaxVolume(data.volume, i);
                                lookBack.updateMinVolume(data.volume, i);
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
                        if ((this->ProbabilityVolatility(meanVol, stdVol, 
                            meanVol * this->volumePercentageLongComparison)) < this->volumePercentageLongThreshold){
                            double meanVolPrev = lookBack.DetermineMeanVolumePrev();
                            double topComparisonVol = meanVolPrev + meanVolPrev * this->volumeComparison;
                            double bottomComparisonVol = meanVolPrev - meanVolPrev * this->volumeComparison;
                            if (bottomComparisonVol <= meanVol && meanVol <= topComparisonVol){
                                double prevVolume = this->volumeDropComparison * data.volume[i - 1];
                                if (currentVol < prevVolume){
                                    // HNVSL
                                    double numShares = this->DetermineShares(currentPrice);
                                    if (numShares == 0){ 
                                        lookBack.updateMaxVolume(data.volume, i);
                                        lookBack.updateMinVolume(data.volume, i);
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
            else if (this->LVLSignal || currentPrice < lookBack.minPrice){
                // Update Min Price
                lookBack.updateMinPrice(data.close, i);
                double meanPrice = lookBack.DetermineMeanPrice();
                double stdPrice = lookBack.DetermineSTDPrice();
                double meanDiffPrice = lookBack.DetermineMeanDiffPrice();
                double meanCompare = lookBack.DetermineProbGreaterDiffPrice(meanPrice + meanDiffPrice);
                if (this->LVLSignal || (this->ProbabilityVolatility(meanPrice, stdPrice, 
                    meanPrice * this->pricePercentageShortComparison)) > this->pricePercentageShortThreshold){
                    // LVL
                    if (!this->LVLSignal){
                        this->LVLSignal = true;
                        lookBack.updateMaxVolume(data.volume, i);
                        lookBack.updateMinVolume(data.volume, i);
                        continue;
                    }
                    this->LVLCount = this->LVLCount + 1;
                    if (this->LVLCount > this->LVLWaitingPeriod){
                        this->LVLSignal = false;
                        this->LVLCount = 0;
                        lookBack.updateMaxPrice(data.close, i);
                        lookBack.updateMaxVolume(data.volume, i);
                        lookBack.updateMinVolume(data.volume, i);
                        continue;
                    }
                    double bottomMeanPrice = meanPrice - meanPrice * this->LVLExitComparison;
                    if (currentPrice > lookBack.minPrice && currentPrice < bottomMeanPrice){
                        double prevVolume = data.volume[i - 1];
                        if (currentVol <= this->LVLVolumeDropComparison * prevVolume){
                            double numShares = this->DetermineShares(currentPrice);
                            if (numShares == 0){ 
                                lookBack.updateMaxPrice(data.close, i);
                                lookBack.updateMaxVolume(data.volume, i);
                                lookBack.updateMinVolume(data.volume, i);
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
                else if (((this->ProbabilityVolatility(meanPrice, stdPrice, 
                        meanPrice * this->pricePercentageShortComparison)) < this->pricePercentageShortThreshold) && 
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
                                lookBack.updateMaxVolume(data.volume, i);
                                lookBack.updateMinVolume(data.volume, i);
                                continue; 
                            }
                            double stopLossPrice = currentPrice + lookBack.DetermineATR() * ATRMultiplier;
                            Position newPosition(SHORT, LNVLS, data.date[i], "", currentPrice, stopLossPrice, 0, 
                                    numShares, false);
                            this->setOpenPosition(newPosition);
                            this->setContainsOpenPosition(true);
                            this->balance += numShares * currentPrice;
                        }
                        else{
                            // LNVLHVS
                            double numShares = this->DetermineShares(currentPrice);
                            if (numShares == 0){
                                lookBack.updateMaxVolume(data.volume, i);
                                lookBack.updateMinVolume(data.volume, i);
                                continue; 
                            }
                            double stopLossPrice = currentPrice + lookBack.DetermineATR() * ATRMultiplier;
                            Position newPosition(SHORT, LNVLHVS, data.date[i], "", currentPrice, stopLossPrice, 0, 
                                    numShares, false);
                            this->setOpenPosition(newPosition);
                            this->setContainsOpenPosition(true);
                            this->balance += numShares * currentPrice;
                            this->LNVLHVSLowCounter += 1;
                            this->LNVLHVSLowRunningSum += currentVol;
                        }
                    }
                    else if (meanPrice < meanPrev){
                        if ((this->ProbabilityVolatility(meanVol, stdVol, 
                            meanVol * this->volumePercentageShortComparison)) < this->volumePercentageShortThreshold){
                            double meanVolPrev = lookBack.DetermineMeanVolumePrev();
                            double topComparisonVol = meanVolPrev + meanVolPrev * this->volumeDropComparison;
                            double bottomComparisonVol = meanVolPrev - meanVolPrev * this->volumeDropComparison;
                            if (bottomComparisonVol <= meanVol && meanVol <= topComparisonVol){
                                double prevVolume = this->volumeDropComparison * data.volume[i - 1];
                                if (currentVol < prevVolume){
                                    // LNVSS
                                    double numShares = this->DetermineShares(currentPrice);
                                    if (numShares == 0){ 
                                        lookBack.updateMaxVolume(data.volume, i);
                                        lookBack.updateMinVolume(data.volume, i);
                                        continue; 
                                    }
                                    double stopLossPrice = currentPrice + lookBack.DetermineATR() * ATRMultiplier;
                                    Position newPosition(SHORT, LNVSS, data.date[i], "", currentPrice, stopLossPrice, 0, 
                                            numShares, false);
                                    this->setOpenPosition(newPosition);
                                    this->setContainsOpenPosition(true);
                                    this->balance += numShares * currentPrice;
                                    this->LNVSSCounter += 1;
                                    this->LNVSSRunningSum += currentVol;
                                    this->LNVSSRunningSumSquared += currentVol * currentVol;
                                }
                            }
                        }
                    }
                }
            }
        }

        lookBack.updateMaxPrice(data.close, i);
        lookBack.updateMinPrice(data.close, i);
        lookBack.updateMaxVolume(data.volume, i);
        lookBack.updateMinVolume(data.volume, i);
    }
    lookBack = LookBack(lookBack.lookbackPeriod, lookBack.ATRLookbackPeriod);
}