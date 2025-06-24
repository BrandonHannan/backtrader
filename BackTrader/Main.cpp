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

    vector<double> ATRMultiplierArray = {0.25, 0.5, 0.75, 1, 1.5, 2.0, 2.5, 3.0, 3.5, 4.0};
    vector<double> RiskAmountArray = {0.01, 0.015, 0.02, 0.025, 0.03, 0.035, 0.04, 0.045, 0.05};
    vector<double> volatilityThreshold1Array = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20};
    vector<double> volatilityThreshold2Array = {1, 5, 10, 15, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35};
    vector<double> volumeDropThresholdArray = {0.1, 0.15, 0.2, 0.225, 0.25, 0.275, 0.3, 0.325, 0.35, 0.375, 0.4, 0.45, 0.5};
    vector<double> volumeComparisonArray = {0.1, 0.15, 0.2, 0.225, 0.25, 0.275, 0.3, 0.325, 0.35, 0.375, 0.4, 0.45, 0.5};
    vector<double> priceSurgeArray = {1.05, 1.1, 1.15, 1.2, 1.25, 1.3, 1.35, 1.4, 1.45, 1.5, 1.55, 1.6, 1.65, 1.7, 1.75};
    vector<double> dropPriceSurgeArray = {0.55, 0.6, 0.65, 0.7, 0.75, 0.8, 0.85, 0.9, 0.95, 0.99};
    vector<double> volumeComparisonPriceSurge = {0.01, 0.05, 0.75, 0.1, 0.15, 0.2, 0.25, 0.3, 0.4};

    double ATRMultiplier = ATRMultiplierArray[5];
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
    double volumeComparisonPriceSurge = volumeComparisonPriceSurge[4];
    double volumeComparisonDropSurge = volumeComparisonPriceSurge[4];
    double priceDiffLongCompare; // E.g. 0.5 - 0.9 Comparison
    double priceDiffShortCompare; // E.g. 0.5 - 0.9 Comparison
    double HVSComparison; // E.g. 0.01 - 0.25 Multiplier
    double HNVSLComparison; // E.g. 1 - 100 Comparison
    double HNVHHVLComparison; // E.g. 0.01 - 0.25 Multiplier
    double LVLComparison; // E.g. 0.01 - 0.24 Multiplier
    double LNVSSComparison; // E.g. 1 - 100 Comparison
    double LNVLHVSComparison; // E.g. 0.01 - 0.25 Multiplier
    int HVSWaitingPeriod = 0;
    double HVSVolumeDropComparison; // E.g. 0.05 - 0.5 Multiplier
    int LVLWaitingPeriod = 0;
    double LVLVolumeDropComparison; // E.g. 0.05 - 0.5 Multiplier
    double HVSExitComparison; // E.g. 0.01 - 0.25 Multiplier
    double HNVSLExitThreshold; // E.g. 1 - 100 Comparison
    double HNVHHVLExitComparison; // E.g. 0.01 - 0.25 Multiplier
    double LVLExitComparison; // E.g. 0.01 - 0.25 Multiplier
    double LNVSSExitThreshold; // E.g. 1 - 100 Comparison
    double LNVLHVSExitComparison; // E.g. 0.01 - 0.25 Multiplier
    return 0;
}