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
    vector<double> priceSurgeArray = {1.05, 1.1, 1.15, 1.2, 1.25, 1.3, 1.35, 1.4, 1.45, 1.5, 1.55, 1.6, 1.65, 1.7, 1.75};
    vector<double> dropPriceSurgeArray = {0.55, 0.6, 0.65, 0.7, 0.75, 0.8, 0.85, 0.9, 0.95, 0.99};
    vector<double> volumeComparisonPriceSurgeArray = {0.01, 0.05, 0.75, 0.1, 0.15, 0.2, 0.25, 0.3, 0.4};
    vector<double> priceDiffCompareArray = {0.2, 0.25, 0.3, 0.35, 0.4, 0.45, 0.5, 0.55, 0.6, 0.65, 0.7, 0.75, 0.8, 0.85, 0.9, 0.95, 0.99};
    vector<int> waitingPeriodArray = {1, 2, 3, 4, 5, 6, 7, 8 ,9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20};
    vector<double> priceComparisonArray = {0.01, 0.025, 0.05, 0.075, 0.1, 0.125, 0.15, 0.175, 0.2, 0.225, 0.25, 0.275, 0.3, 0.325, 0.35, 0.375, 0.4, 0.45, 0.5};

    int lookbackPeriod = lookbackPeriodArray[6];

    double ATRMultiplier = ATRMultiplierArray[5];
    int ATRPeriod = waitingPeriodArray[15];
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
    double volumePercentageLongThreshold = priceDiffCompareArray[9];
    double volumePercentageShortThreshold = priceDiffCompareArray[9];

    double balance = 10000;
    Position emptyPosition;

    ofstream file("Returns.txt");

    // CustomChannelBreakout(double bal, bool cOP, Position pos, vector<Position> cPoses, int lbPeriod,
    //     int ATRP, double ATRM, double rM, double vC, double vDC, double pS, double vCPS, double vCDS,
    //     double pDLC, double pDSC, double dPS, int HVSWP, double HVSVDC, int LVLWP, double LVLVC, double HVSEC,
    //     double HNVSLET, double HNVSLEC, double HNVHHVLEC, double LVLEC, double LNVSSEC, double LNVSSET, 
    //     double LNVLHVSEC, double pPLC, double pPLT, double pPSC, double pPST, double vPLC, double vPSC, 
    //     double vPLT, double vPST);

    CustomChannelBreakout strategy(balance, false, emptyPosition, {}, lookbackPeriod, ATRPeriod, ATRMultiplier, RiskAmount,
    volumeComparison, volumeDropComparison, priceSurge, volumeComparisonPriceSurge, volumeComparisonDropSurge, 
    priceDiffLongCompare, priceDiffShortCompare, dropPriceSurge, HVSWaitingPeriod, HVSVolumeDropComparison, 
    LVLWaitingPeriod, LVLVolumeDropComparison, HVSExitComparison, HNVSLExitThreshold, HNVSLExitComparison, 
    HNVHHVLExitComparison, LVLExitComparison, LNVSSExitComparison, LNVSSExitThreshold, LNVLHVSExitComparison, 
    pricePercentageLongComparison, pricePercentageLongThreshold, pricePercentageShortComparison, pricePercentageShortThreshold,
    volumePercentageLongComparison, volumePercentageShortComparison, volumePercentageLongThreshold, volumePercentageShortThreshold);

    vector<string> stocks = {"CL=F", "BZ=F", "NG=F", "HO=F", "GC=F", "SI=F", "PL=F", "PA=F", "HG=F", "ZC=F"};
    for (int i = 0; i<stocks.size(); i++){
        strategy.ExecuteStrategy(data[stocks[i]]);
    }

    // Look Back
    file << "Lookback\n";
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

    file.close();

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