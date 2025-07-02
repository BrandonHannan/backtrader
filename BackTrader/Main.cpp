#include "DataReader/DataReader.h"
#include "TradingStrategy/TradingStrategy.h"
#include "TradingStrategy/CustomChannelBreakout/CustomChannelBreakout.h"
#include "Position/Position.h"
#include <iostream>
#include <algorithm>
#include <time.h>
#include <fstream>

int main(){
    // Use this For MacOS
    unordered_map<string, StockData> data = ReadData("../data.txt");
    // Use this For Windows
    //unordered_map<string, StockData> data = ReadData("C:\\Users\\BrandonHannan\\source\\repos\\backtrader\\data.txt");
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

    vector<int> lookbackPeriodArray = {5, 10, 15, 20, 25, 30, 35, 40, 45, 50, 55, 60, 65, 70, 75, 80, 85, 90, 95, 100};
    vector<int> ATRPeriodArray = {5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 25, 30, 35, 40, 45, 50, 55, 60};
    vector<double> ATRMultiplierArray = {0.25, 0.5, 0.75, 1, 1.5, 2.0, 2.5, 3.0, 3.5, 4.0};
    vector<double> RiskAmountArray = {0.01, 0.015, 0.02, 0.025, 0.03, 0.035, 0.04, 0.045, 0.05};
    vector<double> volatilityThreshold1Array = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20};
    vector<double> volumeComparisonArray = {0.1, 0.15, 0.2, 0.225, 0.25, 0.275, 0.3, 0.325, 0.35, 0.375, 0.4, 0.45, 0.5};
    vector<double> volumeDropComparisonArray = {0.1, 0.15, 0.2, 0.25, 0.3, 0.35, 0.4, 0.45, 0.5};
    vector<double> priceSurgeArray = {1.05, 1.1, 1.15, 1.2, 1.25, 1.3, 1.35, 1.4, 1.45, 1.5, 1.55, 1.6, 1.65, 1.7, 1.75};
    vector<double> dropPriceSurgeArray = {0.55, 0.6, 0.65, 0.7, 0.75, 0.8, 0.85, 0.9, 0.95, 0.99};
    vector<double> volumeComparisonPriceSurgeArray = {0.01, 0.05, 0.075, 0.1, 0.15, 0.2, 0.25, 0.3, 0.4, 0.45, 0.5, 0.55, 0.6};
    vector<double> priceDiffCompareArray = {0.2, 0.25, 0.3, 0.35, 0.4, 0.45, 0.5, 0.55, 0.6, 0.65, 0.7, 0.75, 0.8, 0.85, 0.9, 0.95, 0.99};
    vector<int> waitingPeriodArray = {1, 2, 3, 4, 5, 6, 7, 8 ,9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20};
    vector<double> priceComparisonArray = {0.01, 0.025, 0.05, 0.075, 0.1, 0.125, 0.15, 0.175, 0.2, 0.225, 0.25, 0.275, 0.3, 0.325, 0.35, 0.375, 0.4, 0.45, 0.5};
    vector<double> HVSVolumeDropComparisonArray = {0.2, 0.25, 0.3, 0.35, 0.4, 0.45, 0.5, 0.55, 0.6, 0.65, 0.7, 0.75, 0.8, 0.85, 0.9};
    vector<double> ExitComparisonArray = {1.01, 1.025, 1.05, 1.075, 1.1, 1.125, 1.15, 1.175, 1.2, 1.25, 1.3, 1.35, 1.4, 1.45, 1.5};

    int lookbackPeriod = lookbackPeriodArray[6];

    double ATRMultiplier = ATRMultiplierArray[5];
    int ATRPeriod = 60;
    double RiskAmount = RiskAmountArray[3];

    double volumeComparison = volumeComparisonArray[4];
    double volumeDropComparison = volumeComparisonArray[2];
    double priceSurge = priceSurgeArray[1];
    double dropPriceSurge = dropPriceSurgeArray[7];
    double volumeComparisonPriceSurge = volumeComparisonPriceSurgeArray[4];
    double volumeComparisonDropSurge = volumeComparisonPriceSurgeArray[4];
    double priceDiffLongCompare = priceDiffCompareArray[4];
    double priceDiffShortCompare = priceDiffCompareArray[4];
    int HVSWaitingPeriod = waitingPeriodArray[6];
    double HVSVolumeDropComparison = volumeComparisonArray[2];
    int LVLWaitingPeriod = waitingPeriodArray[6];
    double LVLVolumeDropComparison = volumeComparisonArray[2];
    double HVSExitComparison = priceComparisonArray[4];
    double HNVSLExitThreshold = priceDiffCompareArray[9];
    double HNVSLExitComparison = priceSurgeArray[0];
    double HNVHHVLExitComparison = priceComparisonArray[4];
    double LVLExitComparison = priceComparisonArray[4];
    double LNVSSExitThreshold = priceDiffCompareArray[9];
    double LNVSSExitComparison = priceSurgeArray[0];
    double LNVLHVSExitComparison = priceComparisonArray[4];

    double pricePercentageLongComparison = priceSurgeArray[0];
    double pricePercentageLongThreshold = priceDiffCompareArray[9];
    double pricePercentageShortComparison = priceSurgeArray[0];
    double pricePercentageShortThreshold = priceDiffCompareArray[9];
    double volumePercentageLongComparison = priceSurgeArray[0];
    double volumePercentageShortComparison = priceSurgeArray[0];
    double volumePercentageLongThreshold = 0.99;
    double volumePercentageShortThreshold = priceDiffCompareArray[9];

    double balance = 10000;
    Position emptyPosition;

    ofstream file("Returns.txt");

    CustomChannelBreakout strategy(balance, false, emptyPosition, {}, lookbackPeriod, ATRPeriod, ATRMultiplier, RiskAmount,
    volumeComparison, volumeDropComparison, priceSurge, volumeComparisonPriceSurge, volumeComparisonDropSurge, 
    priceDiffLongCompare, priceDiffShortCompare, dropPriceSurge, HVSWaitingPeriod, HVSVolumeDropComparison, 
    LVLWaitingPeriod, LVLVolumeDropComparison, HVSExitComparison, HNVSLExitThreshold, HNVSLExitComparison, 
    HNVHHVLExitComparison, LVLExitComparison, LNVSSExitComparison, LNVSSExitThreshold, LNVLHVSExitComparison, 
    pricePercentageLongComparison, pricePercentageLongThreshold, pricePercentageShortComparison, pricePercentageShortThreshold,
    volumePercentageLongComparison, volumePercentageShortComparison, volumePercentageLongThreshold, volumePercentageShortThreshold);

    vector<string> stocks = {"CL=F", "BZ=F", "NG=F", "HO=F", "GC=F", "SI=F", "PL=F", "PA=F", "HG=F", "ZC=F"};
    for (int i = 7; i<stocks.size(); i++){
        strategy.ExecuteStrategy(data[stocks[i]]);
    }

    clock_t start = clock();
    // Look Back
    /*file << "Lookback\n";
    for (int i = 0; i < lookbackPeriodArray.size(); i++){
        file << lookbackPeriodArray[i] << "\n^\n";
        CustomChannelBreakout specific(balance, false, emptyPosition, {}, lookbackPeriodArray[i], ATRPeriod, ATRMultiplier, RiskAmount,
        volumeComparison, volumeDropComparison, priceSurge, volumeComparisonPriceSurge, volumeComparisonDropSurge, 
        priceDiffLongCompare, priceDiffShortCompare, dropPriceSurge, HVSWaitingPeriod, HVSVolumeDropComparison, 
        LVLWaitingPeriod, LVLVolumeDropComparison, HVSExitComparison, HNVSLExitThreshold, HNVSLExitComparison, 
        HNVHHVLExitComparison, LVLExitComparison, LNVSSExitComparison, LNVSSExitThreshold, LNVLHVSExitComparison, 
        pricePercentageLongComparison, pricePercentageLongThreshold, pricePercentageShortComparison, pricePercentageShortThreshold,
        volumePercentageLongComparison, volumePercentageShortComparison, volumePercentageLongThreshold, volumePercentageShortThreshold);

        for (int j = 0; j<stocks.size(); j++){
            specific.ExecuteStrategy(data[stocks[j]]);
        }
        map<int, vector<double>> returns = specific.getYearlyReturns();
        for (auto const& x : returns){
            file << x.first << "\n$\n";
            for (int i = 0; i < x.second.size(); i++){
                file << x.second[i] << "\n";
                if (i == x.second.size() - 1){
                    file << "&\n";
                }
            }
        }
        file << "%\n";
    }

    // ATR Period
    file << "ATR Period\n";
    for (int i = 0; i < ATRPeriodArray.size(); i++){
        file << ATRPeriodArray[i] << "\n^\n";
        CustomChannelBreakout specific(balance, false, emptyPosition, {}, lookbackPeriod, ATRPeriodArray[i], ATRMultiplier, RiskAmount,
        volumeComparison, volumeDropComparison, priceSurge, volumeComparisonPriceSurge, volumeComparisonDropSurge, 
        priceDiffLongCompare, priceDiffShortCompare, dropPriceSurge, HVSWaitingPeriod, HVSVolumeDropComparison, 
        LVLWaitingPeriod, LVLVolumeDropComparison, HVSExitComparison, HNVSLExitThreshold, HNVSLExitComparison, 
        HNVHHVLExitComparison, LVLExitComparison, LNVSSExitComparison, LNVSSExitThreshold, LNVLHVSExitComparison, 
        pricePercentageLongComparison, pricePercentageLongThreshold, pricePercentageShortComparison, pricePercentageShortThreshold,
        volumePercentageLongComparison, volumePercentageShortComparison, volumePercentageLongThreshold, volumePercentageShortThreshold);

        for (int j = 0; j<stocks.size(); j++){
            specific.ExecuteStrategy(data[stocks[j]]);
        }
        map<int, vector<double>> returns = specific.getYearlyReturns();
        for (auto const& x : returns){
            file << x.first << "\n$\n";
            for (int i = 0; i < x.second.size(); i++){
                file << x.second[i] << "\n";
                if (i == x.second.size() - 1){
                    file << "&\n";
                }
            }
        }
        file << "%\n";
    }

    // ATR Multiplier
    file << "ATR Multiplier\n";
    for (int i = 0; i < ATRMultiplierArray.size(); i++){
        file << ATRMultiplierArray[i] << "\n^\n";
        CustomChannelBreakout specific(balance, false, emptyPosition, {}, lookbackPeriod, ATRPeriod, ATRMultiplierArray[i], RiskAmount,
        volumeComparison, volumeDropComparison, priceSurge, volumeComparisonPriceSurge, volumeComparisonDropSurge, 
        priceDiffLongCompare, priceDiffShortCompare, dropPriceSurge, HVSWaitingPeriod, HVSVolumeDropComparison, 
        LVLWaitingPeriod, LVLVolumeDropComparison, HVSExitComparison, HNVSLExitThreshold, HNVSLExitComparison, 
        HNVHHVLExitComparison, LVLExitComparison, LNVSSExitComparison, LNVSSExitThreshold, LNVLHVSExitComparison, 
        pricePercentageLongComparison, pricePercentageLongThreshold, pricePercentageShortComparison, pricePercentageShortThreshold,
        volumePercentageLongComparison, volumePercentageShortComparison, volumePercentageLongThreshold, volumePercentageShortThreshold);

        for (int j = 0; j<stocks.size(); j++){
            specific.ExecuteStrategy(data[stocks[j]]);
        }
        map<int, vector<double>> returns = specific.getYearlyReturns();
        for (auto const& x : returns){
            file << x.first << "\n$\n";
            for (int i = 0; i < x.second.size(); i++){
                file << x.second[i] << "\n";
                if (i == x.second.size() - 1){
                    file << "&\n";
                }
            }
        }
        file << "%\n";
    }

    // Risk Amount
    file << "Risk Amount\n";
    for (int i = 0; i < RiskAmountArray.size(); i++){
        file << RiskAmountArray[i] << "\n^\n";
        CustomChannelBreakout specific(balance, false, emptyPosition, {}, lookbackPeriod, ATRPeriod, ATRMultiplier, RiskAmountArray[i],
        volumeComparison, volumeDropComparison, priceSurge, volumeComparisonPriceSurge, volumeComparisonDropSurge, 
        priceDiffLongCompare, priceDiffShortCompare, dropPriceSurge, HVSWaitingPeriod, HVSVolumeDropComparison, 
        LVLWaitingPeriod, LVLVolumeDropComparison, HVSExitComparison, HNVSLExitThreshold, HNVSLExitComparison, 
        HNVHHVLExitComparison, LVLExitComparison, LNVSSExitComparison, LNVSSExitThreshold, LNVLHVSExitComparison, 
        pricePercentageLongComparison, pricePercentageLongThreshold, pricePercentageShortComparison, pricePercentageShortThreshold,
        volumePercentageLongComparison, volumePercentageShortComparison, volumePercentageLongThreshold, volumePercentageShortThreshold);

        for (int j = 0; j<stocks.size(); j++){
            specific.ExecuteStrategy(data[stocks[j]]);
        }
        map<int, vector<double>> returns = specific.getYearlyReturns();
        for (auto const& x : returns){
            file << x.first << "\n$\n";
            for (int i = 0; i < x.second.size(); i++){
                file << x.second[i] << "\n";
                if (i == x.second.size() - 1){
                    file << "&\n";
                }
            }
        }
        file << "%\n";
    }

    // Volume Comparison
    file << "Volume Comparison\n";
    for (int i = 0; i < volumeComparisonArray.size(); i++){
        file << volumeComparisonArray[i] << "\n^\n";
        CustomChannelBreakout specific(balance, false, emptyPosition, {}, lookbackPeriod, ATRPeriod, ATRMultiplier, RiskAmount,
        volumeComparisonArray[i], volumeDropComparison, priceSurge, volumeComparisonPriceSurge, volumeComparisonDropSurge, 
        priceDiffLongCompare, priceDiffShortCompare, dropPriceSurge, HVSWaitingPeriod, HVSVolumeDropComparison, 
        LVLWaitingPeriod, LVLVolumeDropComparison, HVSExitComparison, HNVSLExitThreshold, HNVSLExitComparison, 
        HNVHHVLExitComparison, LVLExitComparison, LNVSSExitComparison, LNVSSExitThreshold, LNVLHVSExitComparison, 
        pricePercentageLongComparison, pricePercentageLongThreshold, pricePercentageShortComparison, pricePercentageShortThreshold,
        volumePercentageLongComparison, volumePercentageShortComparison, volumePercentageLongThreshold, volumePercentageShortThreshold);

        for (int j = 0; j<stocks.size(); j++){
            specific.ExecuteStrategy(data[stocks[j]]);
        }
        map<int, vector<double>> returns = specific.getYearlyReturns();
        for (auto const& x : returns){
            file << x.first << "\n$\n";
            for (int i = 0; i < x.second.size(); i++){
                file << x.second[i] << "\n";
                if (i == x.second.size() - 1){
                    file << "&\n";
                }
            }
        }
        file << "%\n";
    }

    // Volume Drop Comparison
    file << "Volume Drop Comparison\n";
    for (int i = 0; i < volumeDropComparisonArray.size(); i++){
        file << volumeDropComparisonArray[i] << "\n^\n";
        CustomChannelBreakout specific(balance, false, emptyPosition, {}, lookbackPeriod, ATRPeriod, ATRMultiplier, RiskAmount,
        volumeComparison, volumeDropComparisonArray[i], priceSurge, volumeComparisonPriceSurge, volumeComparisonDropSurge, 
        priceDiffLongCompare, priceDiffShortCompare, dropPriceSurge, HVSWaitingPeriod, HVSVolumeDropComparison, 
        LVLWaitingPeriod, LVLVolumeDropComparison, HVSExitComparison, HNVSLExitThreshold, HNVSLExitComparison, 
        HNVHHVLExitComparison, LVLExitComparison, LNVSSExitComparison, LNVSSExitThreshold, LNVLHVSExitComparison, 
        pricePercentageLongComparison, pricePercentageLongThreshold, pricePercentageShortComparison, pricePercentageShortThreshold,
        volumePercentageLongComparison, volumePercentageShortComparison, volumePercentageLongThreshold, volumePercentageShortThreshold);

        for (int j = 0; j<stocks.size(); j++){
            specific.ExecuteStrategy(data[stocks[j]]);
        }
        map<int, vector<double>> returns = specific.getYearlyReturns();
        for (auto const& x : returns){
            file << x.first << "\n$\n";
            for (int i = 0; i < x.second.size(); i++){
                file << x.second[i] << "\n";
                if (i == x.second.size() - 1){
                    file << "&\n";
                }
            }
        }
        file << "%\n";
    }

    // Price Surge
    file << "Price Surge\n";
    for (int i = 0; i < priceSurgeArray.size(); i++){
        file << priceSurgeArray[i] << "\n^\n";
        CustomChannelBreakout specific(balance, false, emptyPosition, {}, lookbackPeriod, ATRPeriod, ATRMultiplier, RiskAmount,
        volumeComparison, volumeDropComparison, priceSurgeArray[i], volumeComparisonPriceSurge, volumeComparisonDropSurge, 
        priceDiffLongCompare, priceDiffShortCompare, dropPriceSurge, HVSWaitingPeriod, HVSVolumeDropComparison, 
        LVLWaitingPeriod, LVLVolumeDropComparison, HVSExitComparison, HNVSLExitThreshold, HNVSLExitComparison, 
        HNVHHVLExitComparison, LVLExitComparison, LNVSSExitComparison, LNVSSExitThreshold, LNVLHVSExitComparison, 
        pricePercentageLongComparison, pricePercentageLongThreshold, pricePercentageShortComparison, pricePercentageShortThreshold,
        volumePercentageLongComparison, volumePercentageShortComparison, volumePercentageLongThreshold, volumePercentageShortThreshold);

        for (int j = 0; j<stocks.size(); j++){
            specific.ExecuteStrategy(data[stocks[j]]);
        }
        map<int, vector<double>> returns = specific.getYearlyReturns();
        for (auto const& x : returns){
            file << x.first << "\n$\n";
            for (int i = 0; i < x.second.size(); i++){
                file << x.second[i] << "\n";
                if (i == x.second.size() - 1){
                    file << "&\n";
                }
            }
        }
        file << "%\n";
    }

    // Drop Price Surge
    file << "Drop Price Surge\n";
    for (int i = 0; i < dropPriceSurgeArray.size(); i++){
        file << dropPriceSurgeArray[i] << "\n^\n";
        CustomChannelBreakout specific(balance, false, emptyPosition, {}, lookbackPeriod, ATRPeriod, ATRMultiplier, RiskAmount,
        volumeComparison, volumeDropComparison, priceSurge, volumeComparisonPriceSurge, volumeComparisonDropSurge, 
        priceDiffLongCompare, priceDiffShortCompare, dropPriceSurgeArray[i], HVSWaitingPeriod, HVSVolumeDropComparison, 
        LVLWaitingPeriod, LVLVolumeDropComparison, HVSExitComparison, HNVSLExitThreshold, HNVSLExitComparison, 
        HNVHHVLExitComparison, LVLExitComparison, LNVSSExitComparison, LNVSSExitThreshold, LNVLHVSExitComparison, 
        pricePercentageLongComparison, pricePercentageLongThreshold, pricePercentageShortComparison, pricePercentageShortThreshold,
        volumePercentageLongComparison, volumePercentageShortComparison, volumePercentageLongThreshold, volumePercentageShortThreshold);

        for (int j = 0; j<stocks.size(); j++){
            specific.ExecuteStrategy(data[stocks[j]]);
        }
        map<int, vector<double>> returns = specific.getYearlyReturns();
        for (auto const& x : returns){
            file << x.first << "\n$\n";
            for (int i = 0; i < x.second.size(); i++){
                file << x.second[i] << "\n";
                if (i == x.second.size() - 1){
                    file << "&\n";
                }
            }
        }
        file << "%\n";
    }

    // Volume Comparison Price Surge
    file << "Volume Comparison Price Surge\n";
    for (int i = 0; i < volumeComparisonPriceSurgeArray.size(); i++){
        file << volumeComparisonPriceSurgeArray[i] << "\n^\n";
        CustomChannelBreakout specific(balance, false, emptyPosition, {}, lookbackPeriod, ATRPeriod, ATRMultiplier, RiskAmount,
        volumeComparison, volumeDropComparison, priceSurge, volumeComparisonPriceSurgeArray[i], volumeComparisonDropSurge, 
        priceDiffLongCompare, priceDiffShortCompare, dropPriceSurge, HVSWaitingPeriod, HVSVolumeDropComparison, 
        LVLWaitingPeriod, LVLVolumeDropComparison, HVSExitComparison, HNVSLExitThreshold, HNVSLExitComparison, 
        HNVHHVLExitComparison, LVLExitComparison, LNVSSExitComparison, LNVSSExitThreshold, LNVLHVSExitComparison, 
        pricePercentageLongComparison, pricePercentageLongThreshold, pricePercentageShortComparison, pricePercentageShortThreshold,
        volumePercentageLongComparison, volumePercentageShortComparison, volumePercentageLongThreshold, volumePercentageShortThreshold);

        for (int j = 0; j<stocks.size(); j++){
            specific.ExecuteStrategy(data[stocks[j]]);
        }
        map<int, vector<double>> returns = specific.getYearlyReturns();
        for (auto const& x : returns){
            file << x.first << "\n$\n";
            for (int i = 0; i < x.second.size(); i++){
                file << x.second[i] << "\n";
                if (i == x.second.size() - 1){
                    file << "&\n";
                }
            }
        }
        file << "%\n";
    }

    // Volume Comparison Drop Price Surge
    file << "Volume Comparison Drop Price Surge\n";
    for (int i = 0; i < volumeComparisonPriceSurgeArray.size(); i++){
        file << volumeComparisonPriceSurgeArray[i] << "\n^\n";
        CustomChannelBreakout specific(balance, false, emptyPosition, {}, lookbackPeriod, ATRPeriod, ATRMultiplier, RiskAmount,
        volumeComparison, volumeDropComparison, priceSurge, volumeComparisonPriceSurge, volumeComparisonPriceSurgeArray[i], 
        priceDiffLongCompare, priceDiffShortCompare, dropPriceSurge, HVSWaitingPeriod, HVSVolumeDropComparison, 
        LVLWaitingPeriod, LVLVolumeDropComparison, HVSExitComparison, HNVSLExitThreshold, HNVSLExitComparison, 
        HNVHHVLExitComparison, LVLExitComparison, LNVSSExitComparison, LNVSSExitThreshold, LNVLHVSExitComparison, 
        pricePercentageLongComparison, pricePercentageLongThreshold, pricePercentageShortComparison, pricePercentageShortThreshold,
        volumePercentageLongComparison, volumePercentageShortComparison, volumePercentageLongThreshold, volumePercentageShortThreshold);

        for (int j = 0; j<stocks.size(); j++){
            specific.ExecuteStrategy(data[stocks[j]]);
        }
        map<int, vector<double>> returns = specific.getYearlyReturns();
        for (auto const& x : returns){
            file << x.first << "\n$\n";
            for (int i = 0; i < x.second.size(); i++){
                file << x.second[i] << "\n";
                if (i == x.second.size() - 1){
                    file << "&\n";
                }
            }
        }
        file << "%\n";
    }

    // Price Difference Long Compare
    file << "Price Difference Long Compare\n";
    for (int i = 0; i < priceDiffCompareArray.size(); i++){
        file << priceDiffCompareArray[i] << "\n^\n";
        CustomChannelBreakout specific(balance, false, emptyPosition, {}, lookbackPeriod, ATRPeriod, ATRMultiplier, RiskAmount,
        volumeComparison, volumeDropComparison, priceSurge, volumeComparisonPriceSurge, volumeComparisonDropSurge, 
        priceDiffCompareArray[i], priceDiffShortCompare, dropPriceSurge, HVSWaitingPeriod, HVSVolumeDropComparison, 
        LVLWaitingPeriod, LVLVolumeDropComparison, HVSExitComparison, HNVSLExitThreshold, HNVSLExitComparison, 
        HNVHHVLExitComparison, LVLExitComparison, LNVSSExitComparison, LNVSSExitThreshold, LNVLHVSExitComparison, 
        pricePercentageLongComparison, pricePercentageLongThreshold, pricePercentageShortComparison, pricePercentageShortThreshold,
        volumePercentageLongComparison, volumePercentageShortComparison, volumePercentageLongThreshold, volumePercentageShortThreshold);

        for (int j = 0; j<stocks.size(); j++){
            specific.ExecuteStrategy(data[stocks[j]]);
        }
        map<int, vector<double>> returns = specific.getYearlyReturns();
        for (auto const& x : returns){
            file << x.first << "\n$\n";
            for (int i = 0; i < x.second.size(); i++){
                file << x.second[i] << "\n";
                if (i == x.second.size() - 1){
                    file << "&\n";
                }
            }
        }
        file << "%\n";
    }

    // Price Difference Short Compare
    file << "Price Difference Short Compare\n";
    for (int i = 0; i < priceDiffCompareArray.size(); i++){
        file << priceDiffCompareArray[i] << "\n^\n";
        CustomChannelBreakout specific(balance, false, emptyPosition, {}, lookbackPeriod, ATRPeriod, ATRMultiplier, RiskAmount,
        volumeComparison, volumeDropComparison, priceSurge, volumeComparisonPriceSurge, volumeComparisonDropSurge, 
        priceDiffLongCompare, priceDiffCompareArray[i], dropPriceSurge, HVSWaitingPeriod, HVSVolumeDropComparison, 
        LVLWaitingPeriod, LVLVolumeDropComparison, HVSExitComparison, HNVSLExitThreshold, HNVSLExitComparison, 
        HNVHHVLExitComparison, LVLExitComparison, LNVSSExitComparison, LNVSSExitThreshold, LNVLHVSExitComparison, 
        pricePercentageLongComparison, pricePercentageLongThreshold, pricePercentageShortComparison, pricePercentageShortThreshold,
        volumePercentageLongComparison, volumePercentageShortComparison, volumePercentageLongThreshold, volumePercentageShortThreshold);

        for (int j = 0; j<stocks.size(); j++){
            specific.ExecuteStrategy(data[stocks[j]]);
        }
        map<int, vector<double>> returns = specific.getYearlyReturns();
        for (auto const& x : returns){
            file << x.first << "\n$\n";
            for (int i = 0; i < x.second.size(); i++){
                file << x.second[i] << "\n";
                if (i == x.second.size() - 1){
                    file << "&\n";
                }
            }
        }
        file << "%\n";
    }

    // HVS Waiting Period
    file << "HVS Waiting Period\n";
    for (int i = 0; i < waitingPeriodArray.size(); i++){
        file << waitingPeriodArray[i] << "\n^\n";
        CustomChannelBreakout specific(balance, false, emptyPosition, {}, lookbackPeriod, ATRPeriod, ATRMultiplier, RiskAmount,
        volumeComparison, volumeDropComparison, priceSurge, volumeComparisonPriceSurge, volumeComparisonDropSurge, 
        priceDiffLongCompare, priceDiffShortCompare, dropPriceSurge, waitingPeriodArray[i], HVSVolumeDropComparison, 
        LVLWaitingPeriod, LVLVolumeDropComparison, HVSExitComparison, HNVSLExitThreshold, HNVSLExitComparison, 
        HNVHHVLExitComparison, LVLExitComparison, LNVSSExitComparison, LNVSSExitThreshold, LNVLHVSExitComparison, 
        pricePercentageLongComparison, pricePercentageLongThreshold, pricePercentageShortComparison, pricePercentageShortThreshold,
        volumePercentageLongComparison, volumePercentageShortComparison, volumePercentageLongThreshold, volumePercentageShortThreshold);

        for (int j = 0; j<stocks.size(); j++){
            specific.ExecuteStrategy(data[stocks[j]]);
        }
        map<int, vector<double>> returns = specific.getYearlyReturns();
        for (auto const& x : returns){
            file << x.first << "\n$\n";
            for (int i = 0; i < x.second.size(); i++){
                file << x.second[i] << "\n";
                if (i == x.second.size() - 1){
                    file << "&\n";
                }
            }
        }
        file << "%\n";
    }

    // HVS Waiting Period
    file << "HVS Waiting Period\n";
    for (int i = 0; i < waitingPeriodArray.size(); i++){
        file << waitingPeriodArray[i] << "\n^\n";
        CustomChannelBreakout specific(balance, false, emptyPosition, {}, lookbackPeriod, ATRPeriod, ATRMultiplier, RiskAmount,
        volumeComparison, volumeDropComparison, priceSurge, volumeComparisonPriceSurge, volumeComparisonDropSurge, 
        priceDiffLongCompare, priceDiffShortCompare, dropPriceSurge, waitingPeriodArray[i], HVSVolumeDropComparison, 
        LVLWaitingPeriod, LVLVolumeDropComparison, HVSExitComparison, HNVSLExitThreshold, HNVSLExitComparison, 
        HNVHHVLExitComparison, LVLExitComparison, LNVSSExitComparison, LNVSSExitThreshold, LNVLHVSExitComparison, 
        pricePercentageLongComparison, pricePercentageLongThreshold, pricePercentageShortComparison, pricePercentageShortThreshold,
        volumePercentageLongComparison, volumePercentageShortComparison, volumePercentageLongThreshold, volumePercentageShortThreshold);

        for (int j = 0; j<stocks.size(); j++){
            specific.ExecuteStrategy(data[stocks[j]]);
        }
        map<int, vector<double>> returns = specific.getYearlyReturns();
        for (auto const& x : returns){
            file << x.first << "\n$\n";
            for (int i = 0; i < x.second.size(); i++){
                file << x.second[i] << "\n";
                if (i == x.second.size() - 1){
                    file << "&\n";
                }
            }
        }
        file << "%\n";
    }

    // HVS Volume Drop Comparison
    file << "HVS Volume Drop Comparison\n";
    for (int i = 0; i < HVSVolumeDropComparisonArray.size(); i++){
        file << HVSVolumeDropComparisonArray[i] << "\n^\n";
        CustomChannelBreakout specific(balance, false, emptyPosition, {}, lookbackPeriod, ATRPeriod, ATRMultiplier, RiskAmount,
        volumeComparison, volumeDropComparison, priceSurge, volumeComparisonPriceSurge, volumeComparisonDropSurge, 
        priceDiffLongCompare, priceDiffShortCompare, dropPriceSurge, HVSWaitingPeriod, HVSVolumeDropComparisonArray[i], 
        LVLWaitingPeriod, LVLVolumeDropComparison, HVSExitComparison, HNVSLExitThreshold, HNVSLExitComparison, 
        HNVHHVLExitComparison, LVLExitComparison, LNVSSExitComparison, LNVSSExitThreshold, LNVLHVSExitComparison, 
        pricePercentageLongComparison, pricePercentageLongThreshold, pricePercentageShortComparison, pricePercentageShortThreshold,
        volumePercentageLongComparison, volumePercentageShortComparison, volumePercentageLongThreshold, volumePercentageShortThreshold);

        for (int j = 0; j<stocks.size(); j++){
            specific.ExecuteStrategy(data[stocks[j]]);
        }
        map<int, vector<double>> returns = specific.getYearlyReturns();
        for (auto const& x : returns){
            file << x.first << "\n$\n";
            for (int i = 0; i < x.second.size(); i++){
                file << x.second[i] << "\n";
                if (i == x.second.size() - 1){
                    file << "&\n";
                }
            }
        }
        file << "%\n";
    }

    // LVL Waiting Period
    file << "LVL Waiting Period\n";
    for (int i = 0; i < waitingPeriodArray.size(); i++){
        file << waitingPeriodArray[i] << "\n^\n";
        CustomChannelBreakout specific(balance, false, emptyPosition, {}, lookbackPeriod, ATRPeriod, ATRMultiplier, RiskAmount,
        volumeComparison, volumeDropComparison, priceSurge, volumeComparisonPriceSurge, volumeComparisonDropSurge, 
        priceDiffLongCompare, priceDiffShortCompare, dropPriceSurge, HVSWaitingPeriod, HVSVolumeDropComparison, 
        waitingPeriodArray[i], LVLVolumeDropComparison, HVSExitComparison, HNVSLExitThreshold, HNVSLExitComparison, 
        HNVHHVLExitComparison, LVLExitComparison, LNVSSExitComparison, LNVSSExitThreshold, LNVLHVSExitComparison, 
        pricePercentageLongComparison, pricePercentageLongThreshold, pricePercentageShortComparison, pricePercentageShortThreshold,
        volumePercentageLongComparison, volumePercentageShortComparison, volumePercentageLongThreshold, volumePercentageShortThreshold);

        for (int j = 0; j<stocks.size(); j++){
            specific.ExecuteStrategy(data[stocks[j]]);
        }
        map<int, vector<double>> returns = specific.getYearlyReturns();
        for (auto const& x : returns){
            file << x.first << "\n$\n";
            for (int i = 0; i < x.second.size(); i++){
                file << x.second[i] << "\n";
                if (i == x.second.size() - 1){
                    file << "&\n";
                }
            }
        }
        file << "%\n";
    }

    // LVL Volume Drop Comparison
    file << "LVL Volume Drop Comparison\n";
    for (int i = 0; i < volumeDropComparisonArray.size(); i++){
        file << volumeDropComparisonArray[i] << "\n^\n";
        CustomChannelBreakout specific(balance, false, emptyPosition, {}, lookbackPeriod, ATRPeriod, ATRMultiplier, RiskAmount,
        volumeComparison, volumeDropComparison, priceSurge, volumeComparisonPriceSurge, volumeComparisonDropSurge, 
        priceDiffLongCompare, priceDiffShortCompare, dropPriceSurge, HVSWaitingPeriod, HVSVolumeDropComparison, 
        LVLWaitingPeriod, volumeDropComparisonArray[i], HVSExitComparison, HNVSLExitThreshold, HNVSLExitComparison, 
        HNVHHVLExitComparison, LVLExitComparison, LNVSSExitComparison, LNVSSExitThreshold, LNVLHVSExitComparison, 
        pricePercentageLongComparison, pricePercentageLongThreshold, pricePercentageShortComparison, pricePercentageShortThreshold,
        volumePercentageLongComparison, volumePercentageShortComparison, volumePercentageLongThreshold, volumePercentageShortThreshold);

        for (int j = 0; j<stocks.size(); j++){
            specific.ExecuteStrategy(data[stocks[j]]);
        }
        map<int, vector<double>> returns = specific.getYearlyReturns();
        for (auto const& x : returns){
            file << x.first << "\n$\n";
            for (int i = 0; i < x.second.size(); i++){
                file << x.second[i] << "\n";
                if (i == x.second.size() - 1){
                    file << "&\n";
                }
            }
        }
        file << "%\n";
    }

    // HVS Exit Comparison
    file << "HVS Exit Comparison\n";
    for (int i = 0; i < volumeComparisonPriceSurgeArray.size(); i++){
        file << volumeComparisonPriceSurgeArray[i] << "\n^\n";
        CustomChannelBreakout specific(balance, false, emptyPosition, {}, lookbackPeriod, ATRPeriod, ATRMultiplier, RiskAmount,
        volumeComparison, volumeDropComparison, priceSurge, volumeComparisonPriceSurge, volumeComparisonDropSurge, 
        priceDiffLongCompare, priceDiffShortCompare, dropPriceSurge, HVSWaitingPeriod, HVSVolumeDropComparison, 
        LVLWaitingPeriod, LVLVolumeDropComparison, volumeComparisonPriceSurgeArray[i], HNVSLExitThreshold, HNVSLExitComparison, 
        HNVHHVLExitComparison, LVLExitComparison, LNVSSExitComparison, LNVSSExitThreshold, LNVLHVSExitComparison, 
        pricePercentageLongComparison, pricePercentageLongThreshold, pricePercentageShortComparison, pricePercentageShortThreshold,
        volumePercentageLongComparison, volumePercentageShortComparison, volumePercentageLongThreshold, volumePercentageShortThreshold);

        for (int j = 0; j<stocks.size(); j++){
            specific.ExecuteStrategy(data[stocks[j]]);
        }
        map<int, vector<double>> returns = specific.getYearlyReturns();
        for (auto const& x : returns){
            file << x.first << "\n$\n";
            for (int i = 0; i < x.second.size(); i++){
                file << x.second[i] << "\n";
                if (i == x.second.size() - 1){
                    file << "&\n";
                }
            }
        }
        file << "%\n";
    }

    // HNVSL Exit Threshold
    file << "HNVSL Exit Threshold\n";
    for (int i = 0; i < priceDiffCompareArray.size(); i++){
        file << priceDiffCompareArray[i] << "\n^\n";
        CustomChannelBreakout specific(balance, false, emptyPosition, {}, lookbackPeriod, ATRPeriod, ATRMultiplier, RiskAmount,
        volumeComparison, volumeDropComparison, priceSurge, volumeComparisonPriceSurge, volumeComparisonDropSurge, 
        priceDiffLongCompare, priceDiffShortCompare, dropPriceSurge, HVSWaitingPeriod, HVSVolumeDropComparison, 
        LVLWaitingPeriod, LVLVolumeDropComparison, HVSExitComparison, priceDiffCompareArray[i], HNVSLExitComparison, 
        HNVHHVLExitComparison, LVLExitComparison, LNVSSExitComparison, LNVSSExitThreshold, LNVLHVSExitComparison, 
        pricePercentageLongComparison, pricePercentageLongThreshold, pricePercentageShortComparison, pricePercentageShortThreshold,
        volumePercentageLongComparison, volumePercentageShortComparison, volumePercentageLongThreshold, volumePercentageShortThreshold);

        for (int j = 0; j<stocks.size(); j++){
            specific.ExecuteStrategy(data[stocks[j]]);
        }
        map<int, vector<double>> returns = specific.getYearlyReturns();
        for (auto const& x : returns){
            file << x.first << "\n$\n";
            for (int i = 0; i < x.second.size(); i++){
                file << x.second[i] << "\n";
                if (i == x.second.size() - 1){
                    file << "&\n";
                }
            }
        }
        file << "%\n";
    }

    // HNVSL Exit Comparison
    file << "HNVSL Exit Comparison\n";
    for (int i = 0; i < ExitComparisonArray.size(); i++){
        file << ExitComparisonArray[i] << "\n^\n";
        CustomChannelBreakout specific(balance, false, emptyPosition, {}, lookbackPeriod, ATRPeriod, ATRMultiplier, RiskAmount,
        volumeComparison, volumeDropComparison, priceSurge, volumeComparisonPriceSurge, volumeComparisonDropSurge, 
        priceDiffLongCompare, priceDiffShortCompare, dropPriceSurge, HVSWaitingPeriod, HVSVolumeDropComparison, 
        LVLWaitingPeriod, LVLVolumeDropComparison, HVSExitComparison, HNVSLExitThreshold, ExitComparisonArray[i], 
        HNVHHVLExitComparison, LVLExitComparison, LNVSSExitComparison, LNVSSExitThreshold, LNVLHVSExitComparison, 
        pricePercentageLongComparison, pricePercentageLongThreshold, pricePercentageShortComparison, pricePercentageShortThreshold,
        volumePercentageLongComparison, volumePercentageShortComparison, volumePercentageLongThreshold, volumePercentageShortThreshold);

        for (int j = 0; j<stocks.size(); j++){
            specific.ExecuteStrategy(data[stocks[j]]);
        }
        map<int, vector<double>> returns = specific.getYearlyReturns();
        for (auto const& x : returns){
            file << x.first << "\n$\n";
            for (int i = 0; i < x.second.size(); i++){
                file << x.second[i] << "\n";
                if (i == x.second.size() - 1){
                    file << "&\n";
                }
            }
        }
        file << "%\n";
    }

    // HNVHHVL Exit Comparison
    file << "HNVHHVL Exit Comparison\n";
    for (int i = 0; i < ExitComparisonArray.size(); i++){
        file << ExitComparisonArray[i] << "\n^\n";
        CustomChannelBreakout specific(balance, false, emptyPosition, {}, lookbackPeriod, ATRPeriod, ATRMultiplier, RiskAmount,
        volumeComparison, volumeDropComparison, priceSurge, volumeComparisonPriceSurge, volumeComparisonDropSurge, 
        priceDiffLongCompare, priceDiffShortCompare, dropPriceSurge, HVSWaitingPeriod, HVSVolumeDropComparison, 
        LVLWaitingPeriod, LVLVolumeDropComparison, HVSExitComparison, HNVSLExitThreshold, HNVSLExitComparison, 
        ExitComparisonArray[i], LVLExitComparison, LNVSSExitComparison, LNVSSExitThreshold, LNVLHVSExitComparison, 
        pricePercentageLongComparison, pricePercentageLongThreshold, pricePercentageShortComparison, pricePercentageShortThreshold,
        volumePercentageLongComparison, volumePercentageShortComparison, volumePercentageLongThreshold, volumePercentageShortThreshold);

        for (int j = 0; j<stocks.size(); j++){
            specific.ExecuteStrategy(data[stocks[j]]);
        }
        map<int, vector<double>> returns = specific.getYearlyReturns();
        for (auto const& x : returns){
            file << x.first << "\n$\n";
            for (int i = 0; i < x.second.size(); i++){
                file << x.second[i] << "\n";
                if (i == x.second.size() - 1){
                    file << "&\n";
                }
            }
        }
        file << "%\n";
    }

    // LVL Exit Comparison
    file << "LVL Exit Comparison\n";
    for (int i = 0; i < ExitComparisonArray.size(); i++){
        file << ExitComparisonArray[i] << "\n^\n";
        CustomChannelBreakout specific(balance, false, emptyPosition, {}, lookbackPeriod, ATRPeriod, ATRMultiplier, RiskAmount,
        volumeComparison, volumeDropComparison, priceSurge, volumeComparisonPriceSurge, volumeComparisonDropSurge, 
        priceDiffLongCompare, priceDiffShortCompare, dropPriceSurge, HVSWaitingPeriod, HVSVolumeDropComparison, 
        LVLWaitingPeriod, LVLVolumeDropComparison, HVSExitComparison, HNVSLExitThreshold, HNVSLExitComparison, 
        HNVHHVLExitComparison, ExitComparisonArray[i], LNVSSExitComparison, LNVSSExitThreshold, LNVLHVSExitComparison, 
        pricePercentageLongComparison, pricePercentageLongThreshold, pricePercentageShortComparison, pricePercentageShortThreshold,
        volumePercentageLongComparison, volumePercentageShortComparison, volumePercentageLongThreshold, volumePercentageShortThreshold);

        for (int j = 0; j<stocks.size(); j++){
            specific.ExecuteStrategy(data[stocks[j]]);
        }
        map<int, vector<double>> returns = specific.getYearlyReturns();
        for (auto const& x : returns){
            file << x.first << "\n$\n";
            for (int i = 0; i < x.second.size(); i++){
                file << x.second[i] << "\n";
                if (i == x.second.size() - 1){
                    file << "&\n";
                }
            }
        }
        file << "%\n";
    }

    // LNVSS Exit Threshold
    file << "LNVSS Exit Threshold\n";
    for (int i = 0; i < priceDiffCompareArray.size(); i++){
        file << priceDiffCompareArray[i] << "\n^\n";
        CustomChannelBreakout specific(balance, false, emptyPosition, {}, lookbackPeriod, ATRPeriod, ATRMultiplier, RiskAmount,
        volumeComparison, volumeDropComparison, priceSurge, volumeComparisonPriceSurge, volumeComparisonDropSurge, 
        priceDiffLongCompare, priceDiffShortCompare, dropPriceSurge, HVSWaitingPeriod, HVSVolumeDropComparison, 
        LVLWaitingPeriod, LVLVolumeDropComparison, HVSExitComparison, HNVSLExitThreshold, HNVSLExitComparison, 
        HNVHHVLExitComparison, LVLExitComparison, LNVSSExitComparison, priceDiffCompareArray[i], LNVLHVSExitComparison, 
        pricePercentageLongComparison, pricePercentageLongThreshold, pricePercentageShortComparison, pricePercentageShortThreshold,
        volumePercentageLongComparison, volumePercentageShortComparison, volumePercentageLongThreshold, volumePercentageShortThreshold);

        for (int j = 0; j<stocks.size(); j++){
            specific.ExecuteStrategy(data[stocks[j]]);
        }
        map<int, vector<double>> returns = specific.getYearlyReturns();
        for (auto const& x : returns){
            file << x.first << "\n$\n";
            for (int i = 0; i < x.second.size(); i++){
                file << x.second[i] << "\n";
                if (i == x.second.size() - 1){
                    file << "&\n";
                }
            }
        }
        file << "%\n";
    }

    // LNVSS Exit Comparison
    file << "LNVSS Exit Comparison\n";
    for (int i = 0; i < ExitComparisonArray.size(); i++){
        file << ExitComparisonArray[i] << "\n^\n";
        CustomChannelBreakout specific(balance, false, emptyPosition, {}, lookbackPeriod, ATRPeriod, ATRMultiplier, RiskAmount,
        volumeComparison, volumeDropComparison, priceSurge, volumeComparisonPriceSurge, volumeComparisonDropSurge, 
        priceDiffLongCompare, priceDiffShortCompare, dropPriceSurge, HVSWaitingPeriod, HVSVolumeDropComparison, 
        LVLWaitingPeriod, LVLVolumeDropComparison, HVSExitComparison, HNVSLExitThreshold, HNVSLExitComparison, 
        HNVHHVLExitComparison, LVLExitComparison, ExitComparisonArray[i], LNVSSExitThreshold, LNVLHVSExitComparison, 
        pricePercentageLongComparison, pricePercentageLongThreshold, pricePercentageShortComparison, pricePercentageShortThreshold,
        volumePercentageLongComparison, volumePercentageShortComparison, volumePercentageLongThreshold, volumePercentageShortThreshold);

        for (int j = 0; j<stocks.size(); j++){
            specific.ExecuteStrategy(data[stocks[j]]);
        }
        map<int, vector<double>> returns = specific.getYearlyReturns();
        for (auto const& x : returns){
            file << x.first << "\n$\n";
            for (int i = 0; i < x.second.size(); i++){
                file << x.second[i] << "\n";
                if (i == x.second.size() - 1){
                    file << "&\n";
                }
            }
        }
        file << "%\n";
    }

    // LNVLHVS Exit Comparison
    file << "LNVLHVS Exit Comparison\n";
    for (int i = 0; i < ExitComparisonArray.size(); i++){
        file << ExitComparisonArray[i] << "\n^\n";
        CustomChannelBreakout specific(balance, false, emptyPosition, {}, lookbackPeriod, ATRPeriod, ATRMultiplier, RiskAmount,
        volumeComparison, volumeDropComparison, priceSurge, volumeComparisonPriceSurge, volumeComparisonDropSurge, 
        priceDiffLongCompare, priceDiffShortCompare, dropPriceSurge, HVSWaitingPeriod, HVSVolumeDropComparison, 
        LVLWaitingPeriod, LVLVolumeDropComparison, HVSExitComparison, HNVSLExitThreshold, HNVSLExitComparison, 
        HNVHHVLExitComparison, LVLExitComparison, LNVSSExitComparison, LNVSSExitThreshold, ExitComparisonArray[i], 
        pricePercentageLongComparison, pricePercentageLongThreshold, pricePercentageShortComparison, pricePercentageShortThreshold,
        volumePercentageLongComparison, volumePercentageShortComparison, volumePercentageLongThreshold, volumePercentageShortThreshold);

        for (int j = 0; j<stocks.size(); j++){
            specific.ExecuteStrategy(data[stocks[j]]);
        }
        map<int, vector<double>> returns = specific.getYearlyReturns();
        for (auto const& x : returns){
            file << x.first << "\n$\n";
            for (int i = 0; i < x.second.size(); i++){
                file << x.second[i] << "\n";
                if (i == x.second.size() - 1){
                    file << "&\n";
                }
            }
        }
        file << "%\n";
    }

    // Price Percentage Long Comparison
    file << "Price Percentage Long Comparison\n";
    for (int i = 0; i < ExitComparisonArray.size(); i++){
        file << ExitComparisonArray[i] << "\n^\n";
        CustomChannelBreakout specific(balance, false, emptyPosition, {}, lookbackPeriod, ATRPeriod, ATRMultiplier, RiskAmount,
        volumeComparison, volumeDropComparison, priceSurge, volumeComparisonPriceSurge, volumeComparisonDropSurge, 
        priceDiffLongCompare, priceDiffShortCompare, dropPriceSurge, HVSWaitingPeriod, HVSVolumeDropComparison, 
        LVLWaitingPeriod, LVLVolumeDropComparison, HVSExitComparison, HNVSLExitThreshold, HNVSLExitComparison, 
        HNVHHVLExitComparison, LVLExitComparison, LNVSSExitComparison, LNVSSExitThreshold, LNVLHVSExitComparison, 
        ExitComparisonArray[i], pricePercentageLongThreshold, pricePercentageShortComparison, pricePercentageShortThreshold,
        volumePercentageLongComparison, volumePercentageShortComparison, volumePercentageLongThreshold, volumePercentageShortThreshold);

        for (int j = 0; j<stocks.size(); j++){
            specific.ExecuteStrategy(data[stocks[j]]);
        }
        map<int, vector<double>> returns = specific.getYearlyReturns();
        for (auto const& x : returns){
            file << x.first << "\n$\n";
            for (int i = 0; i < x.second.size(); i++){
                file << x.second[i] << "\n";
                if (i == x.second.size() - 1){
                    file << "&\n";
                }
            }
        }
        file << "%\n";
    }

    // Price Percentage Long Threshold
    file << "Price Percentage Long Threshold\n";
    for (int i = 0; i < priceDiffCompareArray.size(); i++){
        file << priceDiffCompareArray[i] << "\n^\n";
        CustomChannelBreakout specific(balance, false, emptyPosition, {}, lookbackPeriod, ATRPeriod, ATRMultiplier, RiskAmount,
        volumeComparison, volumeDropComparison, priceSurge, volumeComparisonPriceSurge, volumeComparisonDropSurge, 
        priceDiffLongCompare, priceDiffShortCompare, dropPriceSurge, HVSWaitingPeriod, HVSVolumeDropComparison, 
        LVLWaitingPeriod, LVLVolumeDropComparison, HVSExitComparison, HNVSLExitThreshold, HNVSLExitComparison, 
        HNVHHVLExitComparison, LVLExitComparison, LNVSSExitComparison, LNVSSExitThreshold, LNVLHVSExitComparison, 
        pricePercentageLongComparison, priceDiffCompareArray[i], pricePercentageShortComparison, pricePercentageShortThreshold,
        volumePercentageLongComparison, volumePercentageShortComparison, volumePercentageLongThreshold, volumePercentageShortThreshold);

        for (int j = 0; j<stocks.size(); j++){
            specific.ExecuteStrategy(data[stocks[j]]);
        }
        map<int, vector<double>> returns = specific.getYearlyReturns();
        for (auto const& x : returns){
            file << x.first << "\n$\n";
            for (int i = 0; i < x.second.size(); i++){
                file << x.second[i] << "\n";
                if (i == x.second.size() - 1){
                    file << "&\n";
                }
            }
        }
        file << "%\n";
    }

    // Price Percentage Short Comparison
    file << "Price Percentage Short Comparison\n";
    for (int i = 0; i < ExitComparisonArray.size(); i++){
        file << ExitComparisonArray[i] << "\n^\n";
        CustomChannelBreakout specific(balance, false, emptyPosition, {}, lookbackPeriod, ATRPeriod, ATRMultiplier, RiskAmount,
        volumeComparison, volumeDropComparison, priceSurge, volumeComparisonPriceSurge, volumeComparisonDropSurge, 
        priceDiffLongCompare, priceDiffShortCompare, dropPriceSurge, HVSWaitingPeriod, HVSVolumeDropComparison, 
        LVLWaitingPeriod, LVLVolumeDropComparison, HVSExitComparison, HNVSLExitThreshold, HNVSLExitComparison, 
        HNVHHVLExitComparison, LVLExitComparison, LNVSSExitComparison, LNVSSExitThreshold, LNVLHVSExitComparison, 
        pricePercentageLongComparison, pricePercentageLongThreshold, ExitComparisonArray[i], pricePercentageShortThreshold,
        volumePercentageLongComparison, volumePercentageShortComparison, volumePercentageLongThreshold, volumePercentageShortThreshold);

        for (int j = 0; j<stocks.size(); j++){
            specific.ExecuteStrategy(data[stocks[j]]);
        }
        map<int, vector<double>> returns = specific.getYearlyReturns();
        for (auto const& x : returns){
            file << x.first << "\n$\n";
            for (int i = 0; i < x.second.size(); i++){
                file << x.second[i] << "\n";
                if (i == x.second.size() - 1){
                    file << "&\n";
                }
            }
        }
        file << "%\n";
    }

    // Price Percentage Short Threshold
    file << "Price Percentage Short Threshold\n";
    for (int i = 0; i < priceDiffCompareArray.size(); i++){
        file << priceDiffCompareArray[i] << "\n^\n";
        CustomChannelBreakout specific(balance, false, emptyPosition, {}, lookbackPeriod, ATRPeriod, ATRMultiplier, RiskAmount,
        volumeComparison, volumeDropComparison, priceSurge, volumeComparisonPriceSurge, volumeComparisonDropSurge, 
        priceDiffLongCompare, priceDiffShortCompare, dropPriceSurge, HVSWaitingPeriod, HVSVolumeDropComparison, 
        LVLWaitingPeriod, LVLVolumeDropComparison, HVSExitComparison, HNVSLExitThreshold, HNVSLExitComparison, 
        HNVHHVLExitComparison, LVLExitComparison, LNVSSExitComparison, LNVSSExitThreshold, LNVLHVSExitComparison, 
        pricePercentageLongComparison, pricePercentageLongThreshold, pricePercentageShortComparison, priceDiffCompareArray[i],
        volumePercentageLongComparison, volumePercentageShortComparison, volumePercentageLongThreshold, volumePercentageShortThreshold);

        for (int j = 0; j<stocks.size(); j++){
            specific.ExecuteStrategy(data[stocks[j]]);
        }
        map<int, vector<double>> returns = specific.getYearlyReturns();
        for (auto const& x : returns){
            file << x.first << "\n$\n";
            for (int i = 0; i < x.second.size(); i++){
                file << x.second[i] << "\n";
                if (i == x.second.size() - 1){
                    file << "&\n";
                }
            }
        }
        file << "%\n";
    }

    // Volume Percentage Long Comparison
    file << "Volume Percentage Long Comparison\n";
    for (int i = 0; i < ExitComparisonArray.size(); i++){
        file << ExitComparisonArray[i] << "\n^\n";
        CustomChannelBreakout specific(balance, false, emptyPosition, {}, lookbackPeriod, ATRPeriod, ATRMultiplier, RiskAmount,
        volumeComparison, volumeDropComparison, priceSurge, volumeComparisonPriceSurge, volumeComparisonDropSurge, 
        priceDiffLongCompare, priceDiffShortCompare, dropPriceSurge, HVSWaitingPeriod, HVSVolumeDropComparison, 
        LVLWaitingPeriod, LVLVolumeDropComparison, HVSExitComparison, HNVSLExitThreshold, HNVSLExitComparison, 
        HNVHHVLExitComparison, LVLExitComparison, LNVSSExitComparison, LNVSSExitThreshold, LNVLHVSExitComparison, 
        pricePercentageLongComparison, pricePercentageLongThreshold, pricePercentageShortComparison, pricePercentageShortThreshold,
        ExitComparisonArray[i], volumePercentageShortComparison, volumePercentageLongThreshold, volumePercentageShortThreshold);

        for (int j = 0; j<stocks.size(); j++){
            specific.ExecuteStrategy(data[stocks[j]]);
        }
        map<int, vector<double>> returns = specific.getYearlyReturns();
        for (auto const& x : returns){
            file << x.first << "\n$\n";
            for (int i = 0; i < x.second.size(); i++){
                file << x.second[i] << "\n";
                if (i == x.second.size() - 1){
                    file << "&\n";
                }
            }
        }
        file << "%\n";
    }

    // Volume Percentage Short Comparison
    file << "Volume Percentage Short Comparison\n";
    for (int i = 0; i < ExitComparisonArray.size(); i++){
        file << ExitComparisonArray[i] << "\n^\n";
        CustomChannelBreakout specific(balance, false, emptyPosition, {}, lookbackPeriod, ATRPeriod, ATRMultiplier, RiskAmount,
        volumeComparison, volumeDropComparison, priceSurge, volumeComparisonPriceSurge, volumeComparisonDropSurge, 
        priceDiffLongCompare, priceDiffShortCompare, dropPriceSurge, HVSWaitingPeriod, HVSVolumeDropComparison, 
        LVLWaitingPeriod, LVLVolumeDropComparison, HVSExitComparison, HNVSLExitThreshold, HNVSLExitComparison, 
        HNVHHVLExitComparison, LVLExitComparison, LNVSSExitComparison, LNVSSExitThreshold, LNVLHVSExitComparison, 
        pricePercentageLongComparison, pricePercentageLongThreshold, pricePercentageShortComparison, pricePercentageShortThreshold,
        volumePercentageLongComparison, ExitComparisonArray[i], volumePercentageLongThreshold, volumePercentageShortThreshold);

        for (int j = 0; j<stocks.size(); j++){
            specific.ExecuteStrategy(data[stocks[j]]);
        }
        map<int, vector<double>> returns = specific.getYearlyReturns();
        for (auto const& x : returns){
            file << x.first << "\n$\n";
            for (int i = 0; i < x.second.size(); i++){
                file << x.second[i] << "\n";
                if (i == x.second.size() - 1){
                    file << "&\n";
                }
            }
        }
        file << "%\n";
    }

    // Volume Percentage Long Threshold
    file << "Volume Percentage Long Threshold\n";
    for (int i = 0; i < priceDiffCompareArray.size(); i++){
        file << priceDiffCompareArray[i] << "\n^\n";
        CustomChannelBreakout specific(balance, false, emptyPosition, {}, lookbackPeriod, ATRPeriod, ATRMultiplier, RiskAmount,
        volumeComparison, volumeDropComparison, priceSurge, volumeComparisonPriceSurge, volumeComparisonDropSurge, 
        priceDiffLongCompare, priceDiffShortCompare, dropPriceSurge, HVSWaitingPeriod, HVSVolumeDropComparison, 
        LVLWaitingPeriod, LVLVolumeDropComparison, HVSExitComparison, HNVSLExitThreshold, HNVSLExitComparison, 
        HNVHHVLExitComparison, LVLExitComparison, LNVSSExitComparison, LNVSSExitThreshold, LNVLHVSExitComparison, 
        pricePercentageLongComparison, pricePercentageLongThreshold, pricePercentageShortComparison, pricePercentageShortThreshold,
        volumePercentageLongComparison, volumePercentageShortComparison, priceDiffCompareArray[i], volumePercentageShortThreshold);

        for (int j = 0; j<stocks.size(); j++){
            specific.ExecuteStrategy(data[stocks[j]]);
        }
        map<int, vector<double>> returns = specific.getYearlyReturns();
        for (auto const& x : returns){
            file << x.first << "\n$\n";
            for (int i = 0; i < x.second.size(); i++){
                file << x.second[i] << "\n";
                if (i == x.second.size() - 1){
                    file << "&\n";
                }
            }
        }
        file << "%\n";
    }

    // Volume Percentage Short Threshold
    file << "Volume Percentage Short Threshold\n";
    for (int i = 0; i < priceDiffCompareArray.size(); i++){
        file << priceDiffCompareArray[i] << "\n^\n";
        CustomChannelBreakout specific(balance, false, emptyPosition, {}, lookbackPeriod, ATRPeriod, ATRMultiplier, RiskAmount,
        volumeComparison, volumeDropComparison, priceSurge, volumeComparisonPriceSurge, volumeComparisonDropSurge, 
        priceDiffLongCompare, priceDiffShortCompare, dropPriceSurge, HVSWaitingPeriod, HVSVolumeDropComparison, 
        LVLWaitingPeriod, LVLVolumeDropComparison, HVSExitComparison, HNVSLExitThreshold, HNVSLExitComparison, 
        HNVHHVLExitComparison, LVLExitComparison, LNVSSExitComparison, LNVSSExitThreshold, LNVLHVSExitComparison, 
        pricePercentageLongComparison, pricePercentageLongThreshold, pricePercentageShortComparison, pricePercentageShortThreshold,
        volumePercentageLongComparison, volumePercentageShortComparison, volumePercentageLongThreshold, priceDiffCompareArray[i]);

        for (int j = 0; j<stocks.size(); j++){
            specific.ExecuteStrategy(data[stocks[j]]);
        }
        map<int, vector<double>> returns = specific.getYearlyReturns();
        for (auto const& x : returns){
            file << x.first << "\n$\n";
            for (int i = 0; i < x.second.size(); i++){
                file << x.second[i] << "\n";
                if (i == x.second.size() - 1){
                    file << "&\n";
                }
            }
        }
        file << "%\n";
    }*/


    file.close();

    clock_t end = clock();
    double timeSpent = (double)(end - start)/CLOCKS_PER_SEC;

    vector<Position> r = strategy.getClosedPositions();
    double sum = 0;
    for (int i = 0; i<r.size(); i++){
        cout << "Position " << i << ":" << endl;
        cout << "Trade Type: " << r[i].getTradeType() << " | ";
        cout << "Position Type: " << r[i].getPositionType() << endl;
        cout << "Purchase Date: " << r[i].getPurchaseDate() << " | " << "Sell Date: " << r[i].getSellDate() << endl;
        cout << "Profit/Loss: ";
        double profit = 0;
        if (r[i].getPositionType() == LONG){
            profit = (r[i].getSellPrice() - r[i].getPurchasePrice()) *
                        r[i].getNumShares();
        }
        else if (r[i].getPositionType() == SHORT){
            profit = (r[i].getPurchasePrice() - r[i].getSellPrice()) *
                        r[i].getNumShares();
        }
        cout << "$" << profit << endl << endl;
        sum = sum + profit;
    }
    cout << "Total Profit/Loss: $" << sum << endl;
    cout << "Balance: $" << strategy.balance << endl << endl;
    cout << "Execution Time: " << timeSpent << "s" << endl;
    // map<int, vector<double>> results = strategy.getYearlyReturns();

    // for (auto const& x : results){
    //     cout << "Year: " << x.first << ":" << endl;
    //     double sum = 0;
    //     for (int i = 0; i < x.second.size(); i++){
    //         sum = sum + x.second[i];
    //         cout << "Trade return: " << x.second[i] << endl;
    //     }
    //     cout << "Total Return: " << sum << endl << endl;
    // }
    return 0;
}