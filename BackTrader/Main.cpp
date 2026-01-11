#include "DataReader/DataReader.h"
#include "./TradingStrategy/Custom/CustomStrategy.h"
#include "./PositionSizing/ATRPositionSize/ATRPositionSize.h"
#include "./TradingContext/BreakoutContext/BreakoutContext.h"
#include <iostream>
#include <algorithm>
#include <time.h>
#include <fstream>


int main(){
    // Use this For MacOS
    // unordered_map<string, StockData> data = ReadData("../data.txt");
    // Use this For Windows
    unordered_map<string, StockData> data = ReadData("C:\\Users\\brand\\Documents\\Repos\\backtrader\\data.txt");
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
    vector<double> percentageArray = {0.1, 0.15, 0.2, 0.25, 0.3, 0.35, 0.4, 0.45, 0.5, 0.55, 0.6, 0.65, 0.7, 0.75, 0.8, 0.85, 0.9, 0.95, 0.99};

    int lookbackPeriod = lookbackPeriodArray[6];

    double ATRMultiplier = ATRMultiplierArray[5];
    int ATRPeriod = ATRPeriodArray[10];
    double RiskAmount = RiskAmountArray[3];

    double priceHighPercentageThreshold = percentageArray[8];
    double priceLowPercentageThreshold = percentageArray[8];
    double volumeHighPercentageThreshold = percentageArray[8];
    double volumeLowPercentageThreshold = percentageArray[8];
    double priceMediumPercentageTreshold = percentageArray[6];

    double balance = 10000;

    unique_ptr<BasePositionSize> sizer = make_unique<ATRPositionSize>(RiskAmount, ATRPeriod, ATRMultiplier);

    unique_ptr<BaseContext> context = make_unique<BreakoutContext>(lookbackPeriod, priceHighPercentageThreshold, volumeHighPercentageThreshold, priceLowPercentageThreshold, volumeLowPercentageThreshold, priceMediumPercentageTreshold);

    CustomStrategy strategy = CustomStrategy(balance, move(sizer), move(context));

    //ofstream file("Returns.txt");

    clock_t start = clock();

    // vector<string> stocks = {"CL=F", "BZ=F", "NG=F", "HO=F", "RB=F",
    //                          "GC=F", "SI=F", "PL=F", "PA=F", "HG=F",
    //                          "ZC=F", "ZW=F", "ZS=F", "ZM=F", "ZL=F",
    //                          "KC=F", "CC=F", "SB=F", "CT=F", "OJ=F",
    //                          "LBS=F", "ZO=F", "ZR=F", "LE=F", "HE=F",
    //                          "GF=F", "ALI=F", "TIO=F", "ETH=F", "DC=F", "CSC=F", "GNF=F"};
    // for (int i = 0; i<stocks.size(); i++){
    //     strategy.ExecuteStrategy(stocks[i], data[stocks[i]]);
    // }

    for (const auto& [ticker, stockData] : data) {
        // Optional: Safety check to ensure data is valid before running
        if (stockData.close.empty() || stockData.close.size() != stockData.volume.size()) {
            cout << "Skipping invalid data for: " << ticker << endl;
            continue;
        }

        cout << "Executing strategy for: " << ticker << "..." << endl; // Debug print
        strategy.ExecuteStrategy(ticker, stockData);
    }

    // Look Back
    // file << "Lookback\n";
    // for (int i = 0; i<lookbackPeriodArray.size(); i++){
    //     file << lookbackPeriodArray[i] << "\n^\n";
    //     CustomStrategy specificStrategy = CustomStrategy(balance, move(sizer), make_unique<BreakoutContext>(lookbackPeriodArray[i], priceHighPercentageThreshold, 
    //         volumeHighPercentageThreshold, priceLowPercentageThreshold, volumeLowPercentageThreshold, priceMediumPercentageTreshold));
    //     for (int j = 0; j<stocks.size(); j++){
    //         specificStrategy.ExecuteStrategy(stocks[j], data[stocks[j]]);
    //     }
    //     map<int, vector<double>> returns = specificStrategy.getYearlyReturns();
    //     for (auto const& x : returns){
    //         file << x.first << "\n$\n";
    //         for (int i = 0; i < x.second.size(); i++){
    //             file << x.second[i] << "\n";
    //             if (i == x.second.size() - 1){
    //                 file << "&\n";
    //             }
    //         }
    //     }
    //     file << "%\n";
    // }

    //file.close();

    clock_t end = clock();
    double timeSpent = (double)(end - start)/CLOCKS_PER_SEC;

    vector<Position> r = strategy.getClosedPositions();
    double sum = 0;

    for (int i = 0; i<10; i++){
        cout << "Position " << i << ":" << endl;
        cout << "Position Type: " << r[i].getPositionType().getPositiontype() << endl;
        cout << "Trade Type: " << r[i].getTradeType() << " | ";
        cout << "Purchase Date: " << r[i].getPurchaseDate() << " | " << "Sell Date: " << r[i].getSellDate() << endl;
        cout << "Profit/Loss: ";
        double profit = 0;
        if (r[i].getPositionType().getPositiontype() == "LONG"){
            profit = (r[i].getSellPrice() - r[i].getPurchasePrice()) *
                        r[i].getNumShares();
        }
        else if (r[i].getPositionType().getPositiontype() == "SHORT"){
            profit = (r[i].getPurchasePrice() - r[i].getSellPrice()) *
                        r[i].getNumShares();
        }
        cout << "$" << profit << endl;
        cout << "Stats: " << endl;
        cout << r[i].getStats() << endl << endl;
        sum = sum + profit;
    }

    cout << "Total Profit/Loss: $" << sum << endl;
    cout << "Balance: $" << strategy.getBalance() << endl << endl;
    cout << "Execution Time: " << timeSpent << "s" << endl;

    // ofstream file1("Analysis.csv");
    // ostringstream oss;
    // file1 << "Trade Number,Trade Type,Position Type,Purchase Date,Sell Date,LookBack Period,Slope P Value,Previous Slope P Value,Price Slope,Previous Price Slope,Old Maximum Price in LookBack Period (Before trade was made),New Maximum PRice in LookBack Period (When trade was made),Old Minimum Price in LookBack Period (Before trade was made),New Minimum Price in LookBack Period (When trade was made),Profit/Loss\n";

    // vector<Position> r = strategy.getClosedPositions();
    // double sum = 0;
    // for (int i = 0; i<r.size(); i++){
    //     string stringResult = "";
    //     cout << "Position " << i << ":" << endl;
    //     stringResult += (to_string(i + 1) + ",");
    //     cout << "Trade Type: " << TradeTypeReader(r[i].getTradeType()) << " | ";
    //     stringResult += (TradeTypeReader(r[i].getTradeType()) + ",");
    //     cout << "Position Type: " << PositionTypeReader(r[i].getPositionType()) << endl;
    //     stringResult += (PositionTypeReader(r[i].getPositionType()) + ",");
    //     cout << "Purchase Date: " << r[i].getPurchaseDate() << " | " << "Sell Date: " << r[i].getSellDate() << endl;
    //     stringResult += (r[i].getPurchaseDate() + ",");
    //     stringResult += (r[i].getSellDate() + ",");
    //     stringResult += (to_string(lookbackPeriod) + ",");
    //     if (r[i].getTradeType() == 0 || r[i].getTradeType() == 4){
    //         PositionStats x = r[i].getStats();
    //         cout << "P Value: " << x.pValue << " P Value Prev: " << x.pValuePrev << endl;
    //         oss.str(""); oss.clear();
    //         oss << std::scientific << std::setprecision(6) << x.pValue;
    //         stringResult += (oss.str() + ",");
    //         oss.str(""); oss.clear();
    //         oss << std::scientific << std::setprecision(6) << x.pValuePrev;
    //         stringResult += (oss.str() + ",");
    //         cout << "Price Slope: " << x.priceSlope << " Price Slope Prev: " << x.priceSlopePrev << endl;
    //         oss.str(""); oss.clear();
    //         oss << std::scientific << std::setprecision(6) << x.priceSlope;
    //         stringResult += (oss.str() + ",");
    //         oss.str(""); oss.clear();
    //         oss << std::scientific << std::setprecision(6) << x.priceSlopePrev;
    //         stringResult += (oss.str() + ",");
    //         if (r[i].getTradeType() == 0){
    //             cout << "Old Maximum Price in LookBack Period (Before trade was made): " << x.prev << endl;
    //             cout << "New Maximum Price in LookBack Period (When trade was made): " << x.current << endl;
    //             stringResult += (to_string(x.prev) + "," + to_string(x.current) + ",,,");
    //         }
    //         else{
    //             cout << "Old Minimum Price in LookBack Period (Before trade was made): " << x.prev << endl;
    //             cout << "New Minimum Price in LookBack Period (When trade was made): " << x.current << endl;
    //             stringResult += (",," + to_string(x.prev) + "," + to_string(x.current) + ",");
    //         }
    //     }
    //     else{
    //         stringResult += ",,,,,,,,";
    //     }
    //     cout << "Profit/Loss: ";
    //     double profit = 0;
    //     if (r[i].getPositionType() == LONG){
    //         profit = (r[i].getSellPrice() - r[i].getPurchasePrice()) *
    //                     r[i].getNumShares();
    //     }
    //     else if (r[i].getPositionType() == SHORT){
    //         profit = (r[i].getPurchasePrice() - r[i].getSellPrice()) *
    //                     r[i].getNumShares();
    //     }
    //     cout << "$" << profit << endl << endl;
    //     stringResult += (to_string(profit) + "\n");
    //     file1 << stringResult;
    //     sum = sum + profit;
    // }
    // cout << "Total Profit/Loss: $" << sum << endl;
    // cout << "Balance: $" << strategy.balance << endl << endl;
    // cout << "Execution Time: " << timeSpent << "s" << endl;
    // file1.close();


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