#ifndef STRATEGYRUNNER_H
#define STRATEGYRUNNER_H

#include "../TradingStrategy/Custom/CustomStrategy.h"
#include "../TradingContext/BaseContext.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <memory>
#include <functional>

class ISweepJob {
public:
    virtual ~ISweepJob() = default;
    virtual void run(ofstream& file, const unordered_map<string, StockData>& data) = 0;
    virtual void run(ofstream& file, const unordered_map<string, StockData>& data, const unordered_map<string, StockData>& minuteData) = 0;
};

// 2. The Implementation: Handles ANY data type (int, double, enum)
template <typename T>
class StrategyRunner : public ISweepJob {
private:
    string paramName;
    vector<T> testValues;
    double initialBalance;
    function<unique_ptr<CustomStrategy>(T)> strategyBuilder; // The lambda that builds the strategy

public:
    StrategyRunner(string name, vector<T> values, double initBal, function<unique_ptr<CustomStrategy>(T)> builder)
        : paramName(name), testValues(values), initialBalance(initBal), strategyBuilder(builder) {}

    // The universal execution logic you only have to write once
    void run(ofstream& file, const unordered_map<string, StockData>& data) override {
        cout << "Running sweep for: " << paramName << "...\n";
        file << paramName << "\n";
        
        for (const T& val : testValues) {
            if constexpr (std::is_enum_v<T>) {
                file << static_cast<int>(val) << "\n^\n";
            } else {
                file << val << "\n^\n";
            }
            
            // Build the strategy for this specific value
            unique_ptr<CustomStrategy> specificStrategy = strategyBuilder(val);

            // Execute across all valid stocks
            for (const auto& [ticker, stockData] : data) {
                size_t n = stockData.close.size();
                if (n == 0 || stockData.open.size() != n || stockData.high.size() != n ||
                    stockData.low.size() != n || stockData.volume.size() != n || stockData.date.size() != n) continue;
                specificStrategy->ExecuteStrategy(ticker, stockData);
                specificStrategy->setBalance(initialBalance);
            }

            // Write results
            map<int, vector<double>> returns = specificStrategy->getYearlyReturns();
            for (auto const& x : returns){
                file << x.first << "\n$\n";
                for (double pnl : x.second) file << pnl << "\n";
                file << "&\n";
            }
            file << "%\n";
        }
    }

    void run(ofstream& file, const unordered_map<string, StockData>& data, const unordered_map<string, StockData>& minuteData) override {
        cout << "Running sweep for: " << paramName << " with minute data...\n";
        file << paramName << "\n";

        for (const T& val : testValues) {
            if constexpr (std::is_enum_v<T>) {
                file << static_cast<int>(val) << "\n^\n";
            } else {
                file << val << "\n^\n";
            }

            // Build the strategy for this specific value
            unique_ptr<CustomStrategy> specificStrategy = strategyBuilder(val);

            // Execute across all valid stocks
            for (const auto& [ticker, stockData] : data) {
                size_t n = stockData.close.size();
                if (n == 0 || stockData.open.size() != n || stockData.high.size() != n ||
                    stockData.low.size() != n || stockData.volume.size() != n || stockData.date.size() != n) continue;
                auto it = minuteData.find(ticker);
                if (it == minuteData.end()) continue;
                specificStrategy->ExecuteStrategy(ticker, stockData, it->second);
                specificStrategy->setBalance(initialBalance);
            }

            // Write results
            map<int, vector<double>> returns = specificStrategy->getYearlyReturns();
            for (auto const& x : returns){
                file << x.first << "\n$\n";
                for (double pnl : x.second) file << pnl << "\n";
                file << "&\n";
            }
            file << "%\n";
        }
    }
};

// 2.5 The 2D Implementation: Handles combinations of any two data types
template <typename T1, typename T2>
class StrategyRunner2D : public ISweepJob {
private:
    string paramName1;
    string paramName2;
    vector<T1> testValues1;
    vector<T2> testValues2;
    double initialBalance;
    function<unique_ptr<CustomStrategy>(T1, T2)> strategyBuilder; // Lambda now takes two parameters

public:
    StrategyRunner2D(string name1, string name2, vector<T1> values1, vector<T2> values2, double initBal, function<unique_ptr<CustomStrategy>(T1, T2)> builder)
        : paramName1(name1), paramName2(name2), testValues1(values1), testValues2(values2), initialBalance(initBal), strategyBuilder(builder) {}

    void run(ofstream& file, const unordered_map<string, StockData>& data) override {
        cout << "Running 2D sweep for: " << paramName1 << " & " << paramName2 << "...\n";
        
        // Header now includes both names
        file << paramName1 << "_" << paramName2 << "\n"; 
        
        for (const T1& val1 : testValues1) {
            for (const T2& val2 : testValues2) {
                // Write the combination pair separated by a comma (e.g., "14,2.5")
                if constexpr (std::is_enum_v<T1>) {
                    file << static_cast<int>(val1) << ",";
                } else {
                    file << val1 << ",";
                }
                
                if constexpr (std::is_enum_v<T2>) {
                    file << static_cast<int>(val2) << "\n^\n";
                } else {
                    file << val2 << "\n^\n";
                }
                
                // Build the strategy for this specific combination
                unique_ptr<CustomStrategy> specificStrategy = strategyBuilder(val1, val2);

                // Execute across all valid stocks
                for (const auto& [ticker, stockData] : data) {
                    size_t n = stockData.close.size();
                    if (n == 0 || stockData.open.size() != n || stockData.high.size() != n ||
                        stockData.low.size() != n || stockData.volume.size() != n || stockData.date.size() != n) continue;
                    specificStrategy->ExecuteStrategy(ticker, stockData);
                    specificStrategy->setBalance(initialBalance);
                }

                // Write results
                map<int, vector<double>> returns = specificStrategy->getYearlyReturns();
                for (auto const& x : returns){
                    file << x.first << "\n$\n";
                    for (double pnl : x.second) file << pnl << "\n";
                    file << "&\n";
                }
                file << "%\n";
            }
        }
    }

    void run(ofstream& file, const unordered_map<string, StockData>& data, const unordered_map<string, StockData>& minuteData) override {
        cout << "Running 2D sweep for: " << paramName1 << " & " << paramName2 << " with minute data...\n";

        // Header now includes both names
        file << paramName1 << "_" << paramName2 << "\n";

        for (const T1& val1 : testValues1) {
            for (const T2& val2 : testValues2) {
                // Write the combination pair separated by a comma (e.g., "14,2.5")
                if constexpr (std::is_enum_v<T1>) {
                    file << static_cast<int>(val1) << ",";
                } else {
                    file << val1 << ",";
                }

                if constexpr (std::is_enum_v<T2>) {
                    file << static_cast<int>(val2) << "\n^\n";
                } else {
                    file << val2 << "\n^\n";
                }

                // Build the strategy for this specific combination
                unique_ptr<CustomStrategy> specificStrategy = strategyBuilder(val1, val2);

                // Execute across all valid stocks
                for (const auto& [ticker, stockData] : data) {
                    size_t n = stockData.close.size();
                    if (n == 0 || stockData.open.size() != n || stockData.high.size() != n ||
                        stockData.low.size() != n || stockData.volume.size() != n || stockData.date.size() != n) continue;
                    auto it = minuteData.find(ticker);
                    if (it == minuteData.end()) continue;
                    specificStrategy->ExecuteStrategy(ticker, stockData, it->second);
                    specificStrategy->setBalance(initialBalance);
                }

                // Write results
                map<int, vector<double>> returns = specificStrategy->getYearlyReturns();
                for (auto const& x : returns){
                    file << x.first << "\n$\n";
                    for (double pnl : x.second) file << pnl << "\n";
                    file << "&\n";
                }
                file << "%\n";
            }
        }
    }
};

// 3. The Universal Function: It just takes a list of jobs and runs them.
void RunAllSweeps(const vector<unique_ptr<ISweepJob>>& jobs, ofstream& file, const unordered_map<string, StockData>& data) {
    cout << "Starting Sweep Engine...\n";
    for (const auto& job : jobs) {
        job->run(file, data);
    }
    cout << "All sweeps completed successfully.\n";
}

void RunAllMinuteSweeps(const vector<unique_ptr<ISweepJob>>& jobs, ofstream& file, const unordered_map<string, StockData>& data, const unordered_map<string, StockData>& minuteData) {
    cout << "Starting Minute Sweep Engine...\n";
    for (const auto& job : jobs) {
        job->run(file, data, minuteData); // Pass both datasets
    }
    cout << "All minute sweeps completed successfully.\n";
}

#endif