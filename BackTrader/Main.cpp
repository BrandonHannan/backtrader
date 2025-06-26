#include "DataReader/DataReader.h"
#include "TradingStrategy/TradingStrategy.h"
#include "TradingStrategy/CustomChannelBreakout/CustomChannelBreakout.h"
#include "Position/Position.h"
#include <iostream>
#include <algorithm>
#include <time.h>

int main(){
    // Use this For MacOS
    //unordered_map<string, StockData> data = ReadData("../data.txt");
    // Use this For Windows
    unordered_map<string, StockData> data = ReadData("C:\\Users\\BrandonHannan\\source\\repos\\backtrader\\data.txt");
    cout << "Number of Stocks: " << data.size() << endl;

    // for (auto stockData : data){
    //     cout << "Stock: " << stockData.first << endl;
    //     StockData x = stockData.second;
    //     cout << "Open: " << x.open[0] << endl;
    //     cout << "Close: " << x.close[0] << endl;
    //     cout << "Low: " << x.low[0] << endl;
    //     cout << "High: " << x.high[0] << endl;
    //     cout << "Volume: " << x.volume[0] << endl;
    //     cout << "Date: " << x.date[0] << endl << endl;
    // }

    vector<int> lookbackPeriodArray = {1, 5, 10, 15, 20, 25, 30, 35, 40, 45, 50, 55, 60, 65, 70, 75, 80, 85, 90, 95, 100};
    vector<double> ATRMultiplierArray = {0.25, 0.5, 0.75, 1, 1.5, 2.0, 2.5, 3.0, 3.5, 4.0};
    vector<double> RiskAmountArray = {0.01, 0.015, 0.02, 0.025, 0.03, 0.035, 0.04, 0.045, 0.05};
    vector<double> volatilityThreshold1Array = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20};
    vector<double> volatilityThreshold2Array = {1, 5, 10, 15, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35};
    vector<double> volumeDropThresholdArray = {0.1, 0.15, 0.2, 0.225, 0.25, 0.275, 0.3, 0.325, 0.35, 0.375, 0.4, 0.45, 0.5};
    vector<double> volumeComparisonArray = {0.1, 0.15, 0.2, 0.225, 0.25, 0.275, 0.3, 0.325, 0.35, 0.375, 0.4, 0.45, 0.5};
    vector<double> priceSurgeArray = {1.05, 1.1, 1.15, 1.2, 1.25, 1.3, 1.35, 1.4, 1.45, 1.5, 1.55, 1.6, 1.65, 1.7, 1.75};
    vector<double> dropPriceSurgeArray = {0.55, 0.6, 0.65, 0.7, 0.75, 0.8, 0.85, 0.9, 0.95, 0.99};
    vector<double> volumeComparisonPriceSurgeArray = {0.01, 0.05, 0.75, 0.1, 0.15, 0.2, 0.25, 0.3, 0.4};
    vector<double> priceDiffCompareArray = {0.5, 0.55, 0.6, 0.65, 0.7, 0.75, 0.8, 0.85, 0.9, 0.95, 0.99};
    vector<int> waitingPeriodArray = {1, 2, 3, 4, 5, 6, 7, 8 ,9, 10, 11, 12, 14, 15, 16, 17, 18, 19, 20};
    vector<double> priceComparisonArray = {0.01, 0.025, 0.05, 0.075, 0.1, 0.125, 0.15, 0.175, 0.2, 0.225, 0.25, 0.275, 0.3, 0.325, 0.35, 0.375, 0.4, 0.45, 0.5};

    int lookbackPeriod = lookbackPeriodArray[6];

    double ATRMultiplier = ATRMultiplierArray[5];
    int ATRPeriod = waitingPeriodArray[15];
    double RiskAmount = RiskAmountArray[3];

    double priceVolatilityLongThreshold1 = volatilityThreshold1Array[11];
    double priceVolatilityLongThreshold2 = volatilityThreshold2Array[4];
    double priceVolatilityShortThreshold1 = volatilityThreshold1Array[11];
    double priceVolatilityShortThreshold2 = volatilityThreshold2Array[4];
    double volumeVolatilityLongThreshold = volatilityThreshold2Array[4];
    double volumeVolatilityShortThreshold = volatilityThreshold2Array[4];
    double volumeDropThreshold = volumeDropThresholdArray[4];
    double volumeComparison = volumeComparisonArray[4];
    double volumeDropComparison = volumeComparisonArray[4];
    double priceSurge = priceSurgeArray[5];
    double dropPriceSurge = dropPriceSurgeArray[3];
    double volumeComparisonPriceSurge = volumeComparisonPriceSurgeArray[4];
    double volumeComparisonDropSurge = volumeComparisonPriceSurgeArray[4];
    double priceDiffLongCompare = priceDiffCompareArray[5];
    double priceDiffShortCompare = priceDiffCompareArray[5];
    int HVSWaitingPeriod = waitingPeriodArray[6];
    double HVSVolumeDropComparison = volumeComparisonArray[4];
    int LVLWaitingPeriod = waitingPeriodArray[6];
    double LVLVolumeDropComparison = volumeComparisonArray[4];
    double HVSExitComparison = priceComparisonArray[4];
    double HNVSLExitThreshold = volatilityThreshold1Array[11];
    double HNVHHVLExitComparison = priceComparisonArray[4];
    double LVLExitComparison = priceComparisonArray[4];
    double LNVSSExitThreshold = volatilityThreshold1Array[11];
    double LNVLHVSExitComparison = priceComparisonArray[4];

    double balance = 10000;
    Position emptyPosition;

    // CustomChannelBreakout(double bal, bool cOP, Position pos, vector<Position> cPoses, int lbPeriod,
    //     int ATRP, double ATRM, double rM, double pVLT1, double pVLT2, double pVST1, double pVST2, double vVLT, 
    //     double vVST, double vDT, double vC, double vDC, double pS, double vCPS, double vCDS, 
    //     double pDLC, double pDSC, double dPS, int HVSWP, double HVSVDC, int LVLWP, double LVLVC, double HVSEC, 
    //     double HNVSLET, double HNVHHVLEC, double LVLEC, double LNVSSET, double LNVLHVSEC):
    //     TradingStrategy(bal, cOP, pos, cPoses), ATRMultiplier(ATRM), RiskAmount(rM), lookBack(LookBack(lbPeriod, ATRP)), 
    //     priceVolatilityLongThreshold1(pVLT1), priceVolatilityLongThreshold2(pVLT2), priceVolatilityShortThreshold1(pVST1), 
    //     priceVolatilityShortThreshold2(pVST2), volumeVolatilityLongThreshold(vVLT), volumeVolatilityShortThreshold(vVST), 
    //     volumeDropThreshold(vDT), volumeComparison(vC), volumeDropComparison(vDC), priceSurge(pS), 
    //     dropPriceSurge(dPS), volumeComparisonPriceSurge(vCPS), volumeComparisonDropSurge(vCDS), priceDiffLongCompare(pDLC), 
    //     priceDiffShortCompare(pDSC), HVSWaitingPeriod(HVSWP), HVSCount(0), HVSVolumeDropComparison(HVSVDC), 
    //     LVLWaitingPeriod(LVLWP), LVLSignal(false), LVLCount(0), LVLVolumeDropComparison(LVLVC), 
    //     HVSExitComparison(HVSEC), HNVSLExitThreshold(HNVSLET), HNVHHVLExitComparison(HNVHHVLEC), 
    //     LVLExitComparison(LVLEC), LNVSSExitThreshold(LNVSSET), LNVLHVSExitComparison(LNVLHVSEC) {}

    CustomChannelBreakout strategy(balance, false, emptyPosition, {}, lookbackPeriod, ATRPeriod, ATRMultiplier,
    RiskAmount, priceVolatilityLongThreshold1, priceVolatilityLongThreshold2, priceVolatilityShortThreshold1,
    priceVolatilityShortThreshold2, volumeVolatilityLongThreshold, volumeVolatilityShortThreshold, volumeDropThreshold,
    volumeComparison, volumeDropComparison, priceSurge, volumeComparisonPriceSurge, volumeComparisonDropSurge,
    priceDiffLongCompare, priceDiffShortCompare, dropPriceSurge, HVSWaitingPeriod, HVSVolumeDropComparison, 
    LVLWaitingPeriod, LVLVolumeDropComparison, HVSExitComparison, HNVSLExitThreshold, HNVHHVLExitComparison, LVLExitComparison,
    LNVSSExitThreshold, LNVLHVSExitComparison);

    strategy.ExecuteStrategy(data["CL=F"]);
    map<int, vector<double>> results = strategy.getYearlyReturns();

    for (auto const& x : results){
        cout << "Year: " << x.first << ":" << endl;
        double sum = 0;
        for (int i = 0; i < x.second.size(); i++){
            sum = sum + x.second[i];
            cout << "Trade return: " << x.second[i] << endl;
        }
        cout << "Total Return: " << sum << endl << endl;
    }
    return 0;
}