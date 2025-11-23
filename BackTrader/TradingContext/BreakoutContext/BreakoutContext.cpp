#include "BreakoutContext.h"

BreakoutContext::BreakoutContext(double ADXThreshold): ADXThreshold(ADXThreshold), priceStat(0), volumeStat(0) {
    // LookbackPeriod must be >= 2
    for (int i = 0; i<this->lookBackPeriod; i++){
        double x = double(i);
        this->indexSum += x;
        this->indexSumSQ += x * x;
    }
}

void BreakoutContext::updateContext(StockDataInstance &currentData, StockDataInstance &previousData){
    double currentClose = currentData.close;
    double previousClose = previousData.close;
    double currentVolume = currentData.volume;
    double previousVolume = previousData.volume;
    
    if (this->prices.size() == 0){
        this->prices.push(previousClose);
        this->prices.push(currentClose);
        this->volume.push(previousVolume);
        this->volume.push(currentVolume);

        this->priceStat.sum += previousClose;
        this->priceStat.sum += currentClose;
        this->priceStat.sumSQ += previousClose * previousClose;
        this->priceStat.sumSQ += currentClose * currentClose;
        this->volumeStat.sum += previousVolume;
        this->volumeStat.sum += currentVolume;
        this->volumeStat.sumSQ += previousVolume * previousVolume;
        this->volumeStat.sumSQ += currentVolume * currentVolume;
    }
    else{
        this->prices.push(currentClose);
        this->volume.push(currentVolume);

        this->priceStat.sum += currentClose;
        this->priceStat.sumSQ += currentClose * currentClose;
        this->volumeStat.sum += currentVolume;
        this->volumeStat.sumSQ += currentVolume * currentVolume;
    }

    if (this->prices.size() > this->lookBackPeriod){
        double frontPrice = this->prices.front();
        double frontVolume = this->volume.front();

        this->priceStat.sum -= frontPrice;
        this->priceStat.sumSQ -= frontPrice * frontPrice;
        this->volumeStat.sum -= frontVolume;
        this->volumeStat.sumSQ -= frontVolume * frontVolume;

        this->volume.pop();
        this->prices.pop();
    }

    if (this->prices.size() == this->lookBackPeriod){
        double meanPrice = this->priceStat.sum/this->lookBackPeriod;
        double meanVolume = this->volumeStat.sum/this->lookBackPeriod;
        double varPrice = (this->priceStat.sumSQ/this->lookBackPeriod) - (meanPrice * meanPrice);
        double varVolume = (this->volumeStat.sumSQ/this->lookBackPeriod) - (meanVolume * meanVolume);

        this->priceStat.mean = make_unique<double>(meanPrice);
        this->volumeStat.mean = make_unique<double>(meanVolume);

        double sqrtPrice = sqrt(varPrice);
        double sqrtVolume = sqrt(varVolume);

        this->priceStat.std = make_unique<double>(sqrtPrice);
        this->volumeStat.std = make_unique<double>(sqrtVolume);

        if (this->priceSTDs.size() < this->lookBackPeriod){
            this->priceSTDs.push(sqrtPrice);
            this->volumeSTDs.push(sqrtVolume);

            this->priceStat.sumSTD += sqrtPrice;
            this->priceStat.sumXSTD += sqrtPrice * (this->priceSTDs.size() - 1);
            this->volumeStat.sumSTD += sqrtVolume;
            this->volumeStat.sumXSTD += sqrtVolume * (this->volumeSTDs.size() - 1);
        }
        else{
            double frontPriceSTD = this->priceSTDs.front();
            double frontVolumeSTD = this->volumeSTDs.front();
            this->priceSTDs.pop();
            this->volumeSTDs.pop();
            this->priceSTDs.push(sqrtPrice);
            this->volumeSTDs.push(sqrtVolume);

            this->priceStat.sumSTD += sqrtPrice - frontPriceSTD;
            this->priceStat.sumXSTD = this->priceStat.sumXSTD - this->priceStat.sumSTD + sqrtPrice * (this->lookBackPeriod - 1);
            this->volumeStat.sumSTD += sqrtVolume - frontVolumeSTD;
            this->volumeStat.sumXSTD = this->volumeStat.sumXSTD - this->volumeStat.sumSTD + sqrtVolume * (this->lookBackPeriod - 1);
            
            double denominator = this->lookBackPeriod * this->indexSumSQ - this->indexSum * this->indexSum;
            double prePriceSlope = this->lookBackPeriod * this->priceStat.sumXSTD - this->indexSum * this->priceStat.sumSTD;
            double preVolumeSlope = this->lookBackPeriod * this->volumeStat.sumXSTD - this->indexSum * this->volumeStat.sumSTD;
            this->priceStat.stdSlope = make_unique<double>(((prePriceSlope)/(denominator)));
            this->volumeStat.stdSlope = make_unique<double>(((preVolumeSlope)/(denominator)));
        }
    }
    return;
}