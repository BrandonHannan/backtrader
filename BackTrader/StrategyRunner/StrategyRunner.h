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
                if (stockData.close.empty() || stockData.close.size() != stockData.volume.size()) continue;
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
};

// 3. The Universal Function: It just takes a list of jobs and runs them.
void RunAllSweeps(const vector<unique_ptr<ISweepJob>>& jobs, ofstream& file, const unordered_map<string, StockData>& data) {
    cout << "Starting Sweep Engine...\n";
    for (const auto& job : jobs) {
        job->run(file, data);
    }
    cout << "All sweeps completed successfully.\n";
}

#endif