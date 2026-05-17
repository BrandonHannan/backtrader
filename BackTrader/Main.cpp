#include "DataReader/DataReader.h"
#include "StrategyRunner/DowATRStrategy/DowATRStrategy.h"
#include "StrategyRunner/DowATRStrategy/DowATRBaseCase.h"
#include "./TradingStrategy/Custom/CustomStrategy.h"
#include "./PositionSizing/ATRPositionSize/ATRPositionSize.h"
#include "./TradingContext/BreakoutContext/BreakoutContext.h"
#include "./Functions/MacroFeatures/MacroFeatures.h"
#include "./include/nlohmann/json.hpp"
#include <iostream>
#include <algorithm>
#include <time.h>
#include <fstream>
#include <sstream>
#include <unordered_set>
#include <vector>
#include <string>
#include <filesystem>
#include <limits>
#include <utility>


int main(){
    // Use this For MacOS
    // unordered_map<string, StockData> data = ReadData("../data.txt");
    // Use this For Windows
    //unordered_map<string, StockData> data = ReadData("C:\\Users\\BrandonHannan\\source\\repos\\backtrader\\data.txt");
    unordered_map<string, StockData> data = ReadData("../data.txt");

    // Dukas Data
    unordered_map<string, StockData> dailyData = ReadData("../DailyData.txt");
    unordered_map<string, StockData> fifteenMinuteData = ReadData("../15MinuteData.txt");
    unordered_map<string, StockData> minuteData = ReadData("../MinuteData.txt");

    // Load cross-asset related-stocks mapping written by DownloadData.py.
    MacroFeatures::RelatedMap relatedMap;
    {
        ifstream relFile("../output/related_stocks.json");
        if (relFile.is_open()) {
            try {
                nlohmann::json relJson;
                relFile >> relJson;
                for (auto it = relJson.begin(); it != relJson.end(); ++it) {
                    const string& primary = it.key();
                    vector<pair<string, int>> entries;
                    for (auto kit = it.value().begin(); kit != it.value().end(); ++kit) {
                        const string& signStr = kit.value().get<string>();
                        int sign = 0;
                        if (signStr == "+") sign = 1;
                        else if (signStr == "-") sign = -1;
                        // "mixed" or anything else stays 0
                        entries.emplace_back(kit.key(), sign);
                    }
                    relatedMap[primary] = move(entries);
                }
                cout << "Loaded related-stocks map for " << relatedMap.size() << " primary tickers." << endl;
            } catch (const exception& e) {
                cout << "Warning: failed to parse ../output/related_stocks.json: " << e.what() << endl;
                relatedMap.clear();
            }
        } else {
            cout << "Note: ../output/related_stocks.json not found; macroContext will be valid:false for all positions." << endl;
        }
    }

    // vector<int> lookbackPeriodArray = {5, 10, 15, 20, 25, 30, 35, 40, 45, 50, 55, 60, 65, 70, 75, 80, 85, 90, 95, 100};
    // vector<int> ATRPeriodArray = {5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 25, 30, 35, 40, 45, 50, 55, 60};
    // vector<double> ATRMultiplierArray = {0.25, 0.5, 0.75, 1, 1.5, 2.0, 2.5, 3.0, 3.5, 4.0};
    // vector<double> RiskAmountArray = {0.01, 0.015, 0.02, 0.025, 0.03, 0.035, 0.04, 0.045, 0.05};
    // vector<double> percentageArray = {0.1, 0.15, 0.2, 0.25, 0.3, 0.35, 0.4, 0.45, 0.5, 0.55, 0.6, 0.65, 0.7, 0.75, 0.8, 0.85, 0.9, 0.95, 0.99};

    // int lookbackPeriod = lookbackPeriodArray[3];

    // double ATRMultiplier = ATRMultiplierArray[3];
    // int ATRPeriod = ATRPeriodArray[17];
    // double RiskAmount = RiskAmountArray[5];

    // double priceHighPercentageThreshold = percentageArray[16];
    // double priceLowPercentageThreshold = percentageArray[1];
    // double volumeHighPercentageThreshold = percentageArray[3];
    // double volumeLowPercentageThreshold = percentageArray[17];
    // double priceMediumPercentageTreshold = percentageArray[6];

    // double balance = 1000;

    // unique_ptr<BasePositionSize> sizer = make_unique<ATRPositionSize>(RiskAmount, ATRPeriod, ATRMultiplier);

    // unique_ptr<BaseContext> context = make_unique<BreakoutContext>(lookbackPeriod, priceHighPercentageThreshold, volumeHighPercentageThreshold, priceLowPercentageThreshold, volumeLowPercentageThreshold, priceMediumPercentageTreshold);

    // CustomStrategy strategy = CustomStrategy(balance, move(sizer), move(context));

    // Load category definitions written by DownloadData.py and prompt the user
    // to scope the run down to a subset of tickers. Filtering happens here so
    // executors and MacroFeatures see a single consistent trading universe.
    vector<pair<string, vector<string>>> categoriesYfinance;
    {
        ifstream catFile("../output/categoriesYfinance.json");
        if (catFile.is_open()) {
            try {
                nlohmann::json catJson;
                catFile >> catJson;
                for (auto it = catJson.begin(); it != catJson.end(); ++it) {
                    vector<string> tickers;
                    for (const auto& t : it.value()) tickers.push_back(t.get<string>());
                    categoriesYfinance.emplace_back(it.key(), move(tickers));
                }
            } catch (const exception& e) {
                cout << "Warning: failed to parse ../output/categoriesYfinance.json: " << e.what()
                     << ". Skipping category filter." << endl;
                categoriesYfinance.clear();
            }
        } else {
            cout << "Note: ../output/categoriesYfinance.json not found; skipping category filter." << endl;
        }
    }

    vector<pair<string, vector<string>>> categoriesDukas;
    {
        ifstream catFile("../output/categories.json");
        if (catFile.is_open()) {
            try {
                nlohmann::json catJson;
                catFile >> catJson;
                for (auto it = catJson.begin(); it != catJson.end(); ++it) {
                    vector<string> tickers;
                    for (const auto& t : it.value()) tickers.push_back(t.get<string>());
                    categoriesDukas.emplace_back(it.key(), move(tickers));
                }
            } catch (const exception& e) {
                cout << "Warning: failed to parse ../output/categories.json: " << e.what()
                     << ". Skipping category filter." << endl;
                categoriesDukas.clear();
            }
        } else {
            cout << "Note: ../output/categories.json not found; skipping category filter." << endl;
        }
    }

    cout << "Select execution mode:\n";
    cout << "  1 - ExecuteBaseCase (single run with default parameters with minute data)\n";
    cout << "  2 - ExecuteAllSweeps (full parameter sweep with minute data)\n";
    cout << "  3 - ExecuteBaseCase (single run with default parameters with daily data)\n";
    cout << "  4 - ExecuteAllSweeps (full parameter sweep with daily data)\n";
    cout << "  5 - ExecuteBaseCase (single run with default parameters with 15 minute interval data)\n";
    cout << "  6 - ExecuteAllSweeps (full parameter sweep with 15 minute interval data)\n";
    cout << "Enter choice: ";
    int choice;
    cin >> choice;

    bool useMinuteData = (choice == 3 || choice == 4);

    if (choice < 1 || choice > 6) {
        cout << "Invalid choice. Exiting.\n";
        return 1;
    }

    unordered_set<string> selectedTickers;
    bool useFilter = false;

    if (useMinuteData && !categoriesDukas.empty()) {
        cout << "Using Dukas minute data. ";
        const int n = static_cast<int>(categoriesDukas.size());
        cout << "\nSelect tickers to run the strategy on:\n";
        for (int i = 0; i < n; ++i) {
            cout << "  " << (i + 1) << ". " << categoriesDukas[i].first
                 << " (" << categoriesDukas[i].second.size() << " tickers)\n";
        }
        cout << "  " << (n + 1) << ". [All] - every loaded ticker\n";
        cout << "  " << (n + 2) << ". [Specific ticker] - enter a single ticker symbol\n";
        cout << "Enter selection (comma-separated numbers for multiple categories, e.g. 1,3): ";

        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        string line;
        getline(cin, line);

        auto trim = [](string s) {
            size_t a = s.find_first_not_of(" \t\r\n");
            size_t b = s.find_last_not_of(" \t\r\n");
            return (a == string::npos) ? string() : s.substr(a, b - a + 1);
        };
        line = trim(line);

        bool fallbackToAll = false;

        if (line.empty()) {
            fallbackToAll = true;
        } else if (line == to_string(n + 1)) {
            // [All] -> no filter
        } else if (line == to_string(n + 2)) {
            cout << "Enter ticker symbol: ";
            string ticker;
            getline(cin, ticker);
            ticker = trim(ticker);
            if (dailyData.find(ticker) == dailyData.end()) {
                cout << "Error: ticker '" << ticker << "' not found in DailyData.txt. Exiting." << endl;
                return 1;
            }
            selectedTickers.insert(ticker);
            useFilter = true;
        } else {
            stringstream ss(line);
            string token;
            bool valid = true;
            while (getline(ss, token, ',')) {
                token = trim(token);
                if (token.empty()) continue;
                bool isNum = !token.empty() && all_of(token.begin(), token.end(), ::isdigit);
                if (!isNum) { valid = false; break; }
                int idx = stoi(token);
                if (idx < 1 || idx > n) { valid = false; break; }
                for (const string& t : categoriesDukas[idx - 1].second) selectedTickers.insert(t);
            }
            if (!valid || selectedTickers.empty()) {
                cout << "Invalid selection. Falling back to [All]." << endl;
                selectedTickers.clear();
                fallbackToAll = true;
            } else {
                useFilter = true;
            }
        }
        if (fallbackToAll) {
            // intentionally leave useFilter=false
        }
    }
    else {
        if (!categoriesYfinance.empty()) {
            cout << "Using daily data. ";
            const int n = static_cast<int>(categoriesYfinance.size());
            cout << "\nSelect tickers to run the strategy on:\n";
            for (int i = 0; i < n; ++i) {
                cout << "  " << (i + 1) << ". " << categoriesYfinance[i].first
                    << " (" << categoriesYfinance[i].second.size() << " tickers)\n";
            }
            cout << "  " << (n + 1) << ". [All] - every loaded ticker\n";
            cout << "  " << (n + 2) << ". [Specific ticker] - enter a single ticker symbol\n";
            cout << "Enter selection (comma-separated numbers for multiple categories, e.g. 1,3): ";

            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            string line;
            getline(cin, line);

            auto trim = [](string s) {
                size_t a = s.find_first_not_of(" \t\r\n");
                size_t b = s.find_last_not_of(" \t\r\n");
                return (a == string::npos) ? string() : s.substr(a, b - a + 1);
            };
            line = trim(line);

            bool fallbackToAll = false;

            if (line.empty()) {
                fallbackToAll = true;
            } else if (line == to_string(n + 1)) {
                // [All] -> no filter
            } else if (line == to_string(n + 2)) {
                cout << "Enter ticker symbol: ";
                string ticker;
                getline(cin, ticker);
                ticker = trim(ticker);
                if (data.find(ticker) == data.end()) {
                    cout << "Error: ticker '" << ticker << "' not found in data.txt. Exiting." << endl;
                    return 1;
                }
                selectedTickers.insert(ticker);
                useFilter = true;
            } else {
                stringstream ss(line);
                string token;
                bool valid = true;
                while (getline(ss, token, ',')) {
                    token = trim(token);
                    if (token.empty()) continue;
                    bool isNum = !token.empty() && all_of(token.begin(), token.end(), ::isdigit);
                    if (!isNum) { valid = false; break; }
                    int idx = stoi(token);
                    if (idx < 1 || idx > n) { valid = false; break; }
                    for (const string& t : categoriesYfinance[idx - 1].second) selectedTickers.insert(t);
                }
                if (!valid || selectedTickers.empty()) {
                    cout << "Invalid selection. Falling back to [All]." << endl;
                    selectedTickers.clear();
                    fallbackToAll = true;
                } else {
                    useFilter = true;
                }
            }
            if (fallbackToAll) {
                // intentionally leave useFilter=false
            }
        } else {
            cout << "No category filter available.\n";
        }
    }

    unordered_map<string, StockData> filteredDailyData;
    unordered_map<string, StockData> filteredMinuteData;

    if (useMinuteData){
        if (useFilter) {
            int missing = 0;
            for (const string& t : selectedTickers) {
                auto it = dailyData.find(t);
                auto it2 = minuteData.find(t);
                if (it2 != minuteData.end()) filteredMinuteData.emplace(it2->first, it2->second);
                else {
                    ++missing;
                    continue;
                }
                if (it != dailyData.end()) filteredDailyData.emplace(it->first, it->second);
                else ++missing;
            }
            cout << "Filtered " << selectedTickers.size() << " -> " << filteredDailyData.size()
                << " tickers (" << missing << " not in dailyData.txt)." << endl;
        } else {
            filteredDailyData = dailyData;
            filteredMinuteData = minuteData;
        }

        int dataSize = 0;
        for (const auto& [ticker, stockData] : filteredDailyData) {
            size_t n = stockData.close.size();
            if (n == 0 || stockData.open.size() != n || stockData.high.size() != n ||
                stockData.low.size() != n || stockData.volume.size() != n || stockData.date.size() != n) continue;
            dataSize = dataSize + 1;
        }
        cout << "Number of Stocks: " << dataSize << endl;

        DowBaseCase dowBase;
        double initial_balance = static_cast<double>(dataSize) * dowBase.balance;
        {
            filesystem::create_directories("../output");
            ofstream configFile("../output/configuration.json");
            configFile << "{\n  \"initial_balance\": " << initial_balance << "\n}\n";
        }
    }
    else {
        if (useFilter) {
            int missing = 0;
            for (const string& t : selectedTickers) {
                auto it = data.find(t);
                if (it != data.end()) filteredDailyData.emplace(it->first, it->second);
                else ++missing;
            }
            cout << "Filtered " << selectedTickers.size() << " -> " << filteredDailyData.size()
                << " tickers (" << missing << " not in data.txt)." << endl;
        } else {
            filteredDailyData = data;
        }

        int dataSize = 0;
        for (const auto& [ticker, stockData] : filteredDailyData) {
            size_t n = stockData.close.size();
            if (n == 0 || stockData.open.size() != n || stockData.high.size() != n ||
                stockData.low.size() != n || stockData.volume.size() != n || stockData.date.size() != n) continue;
            dataSize = dataSize + 1;
        }
        cout << "Number of Stocks: " << dataSize << endl;

        DowBaseCase dowBase;
        double initial_balance = static_cast<double>(dataSize) * dowBase.balance;
        {
            filesystem::create_directories("../output");
            ofstream configFile("../output/configuration.json");
            configFile << "{\n  \"initial_balance\": " << initial_balance << "\n}\n";
        }
    }

    clock_t start = clock();

    if (choice == 1) {
        ExecuteBaseCase(filteredDailyData, relatedMap);
    } else if (choice == 2) {
        ExecuteAllSweeps(filteredDailyData, relatedMap);
    } else if (choice == 3) {
        ExecuteBaseCase(filteredDailyData, filteredMinuteData, relatedMap);
    } else if (choice == 4) {
        ExecuteAllSweeps(filteredDailyData, filteredMinuteData, relatedMap);
    } else {
        cout << "Invalid choice. Exiting.\n";
        return 1;
    }

    clock_t end = clock();
    double timeSpent = (double)(end - start)/CLOCKS_PER_SEC;

    // ofstream file1("Analysis.txt");

    // vector<Position> r = strategy.getClosedPositions();
    // double longReversal = 0;
    // double longBreakthrough = 0;
    // double shortReversal = 0;
    // double shortBreakthrough = 0;
    // double sum = 0;
    // int counter = 0;

    // for (int i = 0; i<r.size(); i++){
    //     cout << "Position " << i << ":" << endl;
    //     cout << "Position Type: " << r[i].getPositionType().getPositiontype() << endl;
    //     file1 << "Position Type: " << r[i].getPositionType().getPositiontype() << "\n";
    //     file1 << "Trade Type: " << r[i].getTradeType() << "\n";
    //     file1 << "Purchase Date: " << r[i].getPurchaseDate() << "\n";
    //     file1 << "Sell Date: " << r[i].getSellDate() << "\n";
    //     file1 << "Profit/Loss: ";
    //     cout << "Trade Type: " << r[i].getTradeType() << " | ";
    //     cout << "Purchase Date: " << r[i].getPurchaseDate() << " | " << "Sell Date: " << r[i].getSellDate() << endl;
    //     cout << "Profit/Loss: ";
    //     double profit = 0;
    //     if (r[i].getPositionType().getPositiontype() == "LONG"){
    //         profit = (r[i].getSellPrice() - r[i].getPurchasePrice()) *
    //                     r[i].getNumShares();
    //         if (r[i].getTradeType() == "LONG REVERSAL") { longReversal += profit; }
    //         if (r[i].getTradeType() == "LONG BREAKTHROUGH") { longBreakthrough += profit; }
    //     }
    //     else if (r[i].getPositionType().getPositiontype() == "SHORT"){
    //         profit = (r[i].getPurchasePrice() - r[i].getSellPrice()) *
    //                     r[i].getNumShares();
    //         if (r[i].getTradeType() == "SHORT REVERSAL") { shortReversal += profit; }
    //         if (r[i].getTradeType() == "SHORT BREAKTHROUGH") { shortBreakthrough += profit; }
    //     }
    //     if (profit > 0){ counter++; }
    //     cout << "$" << profit << endl;
    //     file1 << "$" << profit << "\n";
    //     file1 << "Stats:\n" << r[i].getStats() << "\n\n";
    //     cout << "Stats: " << endl;
    //     cout << r[i].getStats() << endl << endl;
    //     sum = sum + profit;
    // }

    // file1.close();

    // cout << "Total Profit/Loss: $" << sum << endl;
    // cout << "Number of positive trades: " << counter << endl;
    // cout << "LONG REVERSAL Profit/Loss: $" << longReversal << endl;
    // cout << "LONG BREAKTHROUGH Profit/Loss: $" << longBreakthrough << endl;
    // cout << "SHORT REVERSAL Profit/Loss: $" << shortReversal << endl;
    // cout << "SHORT BREAKTHROUGH Profit/Loss: $" << shortBreakthrough << endl;
    // cout << "Balance: $" << strategy.getBalance() << endl << endl;
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