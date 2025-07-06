#ifndef CUSTOMCHANNELBREAKOUT_H
#define CUSTOMCHANNELBREAKOUT_H

#include "../TradingStrategy.h"
#include "boost/math/distributions/students_t.hpp"

class CustomChannelBreakout: public TradingStrategy{
    private:
        double ATRMultiplier; // E.g. 1.1 - 3 Multiplier
        double RiskAmount; // E.g. 0.001 - 0.1 Multiplier

        LookBack lookBack;
        double volumeComparison; // E.g. 0.01 - 0.25 Multiplier
        double volumeDropComparison; // E.g. 0.05 - 0.5 Multiplier
        double priceSurge; // E.g. 1.05 - 1.5 Multiplier
        double dropPriceSurge; // E.g. 0.75 - 0.99 Multiplier
        double volumeComparisonPriceSurge; // E.g. 0.01 - 0.5 Multiplier
        double volumeComparisonDropSurge; // E.g. 0.01 - 0.5 Multiplier

        double priceDiffLongCompare; // E.g. 0.5 - 0.9 Comparison
        double priceDiffShortCompare; // E.g. 0.5 - 0.9 Comparison

        bool HVSSignal = false;
        int HVSCount;
        int HVSWaitingPeriod;
        double HVSVolumeDropComparison; // E.g. 0.05 - 0.5 Multiplier

        bool LVLSignal = false;
        int LVLCount;
        int LVLWaitingPeriod;
        double LVLVolumeDropComparison; // E.g. 0.05 - 0.5 Multiplier

        double HVSExitComparison; // E.g. 0.01 - 0.25 Multiplier
        double HVSPrevMean;
        double HNVSLExitComparison; // E.g. 1.01 - 1.1 Multiplier
        double HNVSLExitThreshold; // E.g. 0.05 - 0.99 Comparison
        int HNVSLCounter;
        double HNVSLRunningSum;
        double HNVSLRunningSumSquared;
        double HNVHHVLExitComparison; // E.g. 0.01 - 0.25 Multiplier
        int HNVHHVLHighCounter;
        double HNVHHVLHighRunningSum;
        double LVLExitComparison; // E.g. 0.01 - 0.25 Multiplier
        double LVLPrevMean;
        double LNVSSExitComparison; // E.g. 1.01 - 1.1 Multiplier
        double LNVSSExitThreshold; // E.g. 0.05 - 0.99 Comparison
        int LNVSSCounter;
        double LNVSSRunningSum;
        double LNVSSRunningSumSquared;
        double LNVLHVSExitComparison; // E.g. 0.01 - 0.25 Multiplier
        int LNVLHVSLowCounter;
        double LNVLHVSLowRunningSum;

        double pricePercentageLongComparison; // E.g. 1.01 - 1.1 Multiplier
        double pricePercentageLongThreshold; // E.g. 0.05 - 0.99 Comparison
        double pricePercentageShortComparison; // E.g. 1.01 - 1.1 Multiplier
        double pricePercentageShortThreshold; // E.g. 0.05 - 0.99 Comparison

        double volumePercentageLongComparison; // E.g. 1.01 - 1.1 Multiplier
        double volumePercentageShortComparison; // E.g. 1.01 - 1.1 Multiplier
        double volumePercentageLongThreshold; // E.g. 0.05 - 0.99 Comparison
        double volumePercentageShortThreshold; // E.g. 0.05 - 0.99 Comparison

        double pricePercentageLongNVComparison; // E.g. 1.01 - 1.1 Multiplier
        double pricePercentageLongNVThreshold; // E.g. 0.05 - 0.99 Comparison
        double pricePercentageShortNVComparison; // E.g. 1.01 - 1.1 Multiplier
        double pricePercentageShortNVThreshold; // E.g. 0.05 - 0.99 Comparison

        double DetermineShares(double currentPrice);

        double ProbabilityVolatility(double mean, double std, double val);

        void sellPosition(double sellPrice, string sellDate);
    public:
        CustomChannelBreakout(double bal, bool cOP, Position pos, vector<Position> cPoses, int lbPeriod,
        int ATRP, double ATRM, double rM, double vC, double vDC, double pS, double vCPS, double vCDS,
        double pDLC, double pDSC, double dPS, int HVSWP, double HVSVDC, int LVLWP, double LVLVC, double HVSEC,
        double HNVSLET, double HNVSLEC, double HNVHHVLEC, double LVLEC, double LNVSSEC, double LNVSSET, 
        double LNVLHVSEC, double pPLC, double pPLT, double pPSC, double pPST, double vPLC, double vPSC, 
        double vPLT, double vPST, double pPLNVC, double pPLNVT, double pPSNVC, double pPSNVT);

        void ExecuteStrategy(const StockData &data) override;
};

#endif