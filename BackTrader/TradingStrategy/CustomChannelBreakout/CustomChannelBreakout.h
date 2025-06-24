#ifndef CUSTOMCHANNELBREAKOUT_H
#define CUSTOMCHANNELBREAKOUT_H

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
        int waitingPeriod; // E.g. 1 - 10 Days
        double volumeComparison; // E.g. 0.01 - 0.25 Multiplier
        double volumeDropComparison; // E.g. 0.05 - 0.5 Multiplier
        double priceSurge; // E.g. 1.05 - 1.5 Multiplier
        double dropPriceSurge; // E.g. 0.75 - 0.99 Multiplier
        double volumeComparisonPriceSurge; // E.g. 0.01 - 0.5 Multiplier
        double volumeComparisonDropSurge; // E.g. 0.01 - 0.5 Multiplier

        double priceDiffLongCompare;
        double priceDiffShortCompare;

        double HVSComparison; // E.g. 0.01 - 0.25 Multiplier
        double HNVSLComparison; // E.g. 1 - 100 Comparison
        double HNVHHVLComparison; // E.g. 0.01 - 0.25 Multiplier
        double LVLComparison; // E.g. 0.01 - 0.24 Multiplier
        double LNVSSComparison; // E.g. 1 - 100 Comparison
        double LNVLHVSComparison; // E.g. 0.01 - 0.25 Multiplier

        bool HVSSignal;
        int HVSCount;
        int HVSWaitingPeriod;
        double HVSVolumeDropComparison; // E.g. 0.05 - 0.5 Multiplier

        bool LVLSignal;
        int LVLCount;
        int LVLWaitingPeriod;
        double LVLVolumeDropComparison; // E.g. 0.05 - 0.5 Multiplier

        double HVSExitComparison; // E.g. 0.01 - 0.25 Multiplier
        double HVSPrevMean;
        double HNVSLExitThreshold; // E.g. 1 - 100 Comparison
        int HNVSLCounter;
        double HNVSLRunningSum;
        double HNVSLRunningSumSquared;
        double HNVHHVLExitComparison; // E.g. 0.01 - 0.25 Multiplier
        int HNVHHVLHighCounter;
        double HNVHHVLHighRunningSum;
        double LVLExitComparison; // E.g. 0.01 - 0.25 Multiplier
        double LVLPrevMean;
        double LNVSSExitThreshold; // E.g. 1 - 100 Comparison
        int LNVSSCounter;
        double LNVSSRunningSum;
        double LNVSSRunningSumSquared;
        double LNVLHVSExitComparison; // E.g. 0.01 - 0.25 Multiplier
        int LNVLHVSLowCounter;
        double LNVLHVSLowRunningSum;
    public:
        CustomChannelBreakout(double bal, bool cOP, Position pos, vector<Position> cPoses, int lbPeriod,
        int ATRP, double ATRM, double rM, double pVLT1, double pVLT2, double pVST1, double pVST2, double vVLT, 
        double vVST, double vDT, int wPeriod, double vC, double vDC, double pS, double vCPS, double vCDS, 
        double pDLC, double pDSC, double dPS, double HVS, double HNVSL, double HNVHHVL, double LVL, double LNVSS, 
        double LNVLHVS, int HVSWP, double HVSVDC, int LVLWP, double LVLVC, double HVSEC, double HNVSLET, 
        double HNVHHVLEC, double LVLEC, double LNVSSET, double LNVLHVSEC);

        double DetermineShares(double currentPrice);

        void sellPosition(double sellPrice, string sellDate);

        void ExecuteStrategy(const StockData &data);
};

#endif