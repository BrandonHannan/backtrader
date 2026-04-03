#ifndef DOWATRSTRATEGY_H
#define DOWATRSTRATEGY_H

#include "../../InputParameters/DowContextParameters/DowContextInputParameters.h"
#include "../../PositionSizing/ATRPositionSize/ATRPositionSize.h"
#include "../../TradingContext/DowContext/DowContext.h"
#include "../StrategyRunner.h"
#include "DowATRBaseCase.h"
#include <fstream>
#include <filesystem>

using namespace std;    

void ExecuteAllSweeps(unordered_map<string, StockData> &data){
    filesystem::create_directories("../output");
    ofstream file("../output/Returns.txt");
    DowContextInputParameters dowParams;
    DowBaseCase dowBase;
    vector<unique_ptr<ISweepJob>> mySweeps;

    mySweeps.push_back(make_unique<StrategyRunner<int>>(
        "Lookback", dowParams.lookbackPeriodArray, dowBase.balance, [&](int testVal) {
            auto sizer = make_unique<ATRPositionSize>(dowBase.riskAmount, dowBase.atrPeriod, dowBase.atrMultiplier);
            auto context = make_unique<DowContext>(testVal, dowBase.doubleLookback, dowBase.signalLookback, dowBase.trendMode, dowBase.trendLineMode);
            return make_unique<CustomStrategy>(dowBase.balance, move(sizer), move(context));
        }
    ));

    mySweeps.push_back(make_unique<StrategyRunner<int>>(
        "Double Lookback", dowParams.doubleLookbackPeriodArray, dowBase.balance, [&](int testVal) {
            auto sizer = make_unique<ATRPositionSize>(dowBase.riskAmount, dowBase.atrPeriod, dowBase.atrMultiplier);
            auto context = make_unique<DowContext>(dowBase.lookback, testVal, dowBase.signalLookback, dowBase.trendMode, dowBase.trendLineMode);
            return make_unique<CustomStrategy>(dowBase.balance, move(sizer), move(context));
        }
    ));

    mySweeps.push_back(make_unique<StrategyRunner<int>>(
        "Signal Lookback", dowParams.signalLookBackPeriodArray, dowBase.balance, [&](int testVal) {
            auto sizer = make_unique<ATRPositionSize>(dowBase.riskAmount, dowBase.atrPeriod, dowBase.atrMultiplier);
            auto context = make_unique<DowContext>(dowBase.lookback, dowBase.doubleLookback, testVal, dowBase.trendMode, dowBase.trendLineMode);
            return make_unique<CustomStrategy>(dowBase.balance, move(sizer), move(context));
        }
    ));

    mySweeps.push_back(make_unique<StrategyRunner<double>>(
        "Risk Amount", dowParams.RiskAmountArray, dowBase.balance, [&](double testVal) {
            auto sizer = make_unique<ATRPositionSize>(testVal, dowBase.atrPeriod, dowBase.atrMultiplier);
            auto context = make_unique<DowContext>(dowBase.lookback, dowBase.doubleLookback, dowBase.signalLookback, dowBase.trendMode, dowBase.trendLineMode);
            return make_unique<CustomStrategy>(dowBase.balance, move(sizer), move(context));
        }
    ));

    mySweeps.push_back(make_unique<StrategyRunner<int>>(
        "ATR Period", dowParams.ATRPeriodArray, dowBase.balance, [&](int testVal) {
            auto sizer = make_unique<ATRPositionSize>(dowBase.riskAmount, testVal, dowBase.atrMultiplier);
            auto context = make_unique<DowContext>(dowBase.lookback, dowBase.doubleLookback, dowBase.signalLookback, dowBase.trendMode, dowBase.trendLineMode);
            return make_unique<CustomStrategy>(dowBase.balance, move(sizer), move(context));
        }
    ));

    mySweeps.push_back(make_unique<StrategyRunner<double>>(
        "ATR Multiplier", dowParams.ATRMultiplierArray, dowBase.balance, [&](double testVal) {
            auto sizer = make_unique<ATRPositionSize>(dowBase.riskAmount, dowBase.atrPeriod, testVal);
            auto context = make_unique<DowContext>(dowBase.lookback, dowBase.doubleLookback, dowBase.signalLookback, dowBase.trendMode, dowBase.trendLineMode);
            return make_unique<CustomStrategy>(dowBase.balance, move(sizer), move(context));
        }
    ));

    mySweeps.push_back(make_unique<StrategyRunner<TrendMode>>(
        "Trend Mode", dowParams.trendModes, dowBase.balance, [&](TrendMode testVal) {
            auto sizer = make_unique<ATRPositionSize>(dowBase.riskAmount, dowBase.atrPeriod, dowBase.atrMultiplier);
            auto context = make_unique<DowContext>(dowBase.lookback, dowBase.doubleLookback, dowBase.signalLookback, testVal, dowBase.trendLineMode);
            return make_unique<CustomStrategy>(dowBase.balance, move(sizer), move(context));
        }
    ));

    mySweeps.push_back(make_unique<StrategyRunner<TrendLineMode>>(
        "Trend Line Mode", dowParams.trendLineModes, dowBase.balance, [&](TrendLineMode testVal) {
            auto sizer = make_unique<ATRPositionSize>(dowBase.riskAmount, dowBase.atrPeriod, dowBase.atrMultiplier);
            auto context = make_unique<DowContext>(dowBase.lookback, dowBase.doubleLookback, dowBase.signalLookback, dowBase.trendMode, testVal);
            return make_unique<CustomStrategy>(dowBase.balance, move(sizer), move(context));
        }
    ));

    RunAllSweeps(mySweeps, file, data);
    file.close();

    // Execute base case and write all positions to data.json
    {
        auto sizer = make_unique<ATRPositionSize>(dowBase.riskAmount, dowBase.atrPeriod, dowBase.atrMultiplier);
        auto context = make_unique<DowContext>(dowBase.lookback, dowBase.doubleLookback, dowBase.signalLookback, dowBase.trendMode, dowBase.trendLineMode);
        auto baseStrategy = make_unique<CustomStrategy>(dowBase.balance, move(sizer), move(context));

        for (const auto& [ticker, stockData] : data) {
            if (stockData.close.empty() || stockData.close.size() != stockData.volume.size()) continue;
            baseStrategy->ExecuteStrategy(ticker, stockData);
            baseStrategy->setBalance(dowBase.balance);
        }

        json positionsJson = json::array();
        for (const Position& pos : baseStrategy->getClosedPositions()) {
            positionsJson.push_back(pos.toJson());
        }

        ofstream dataFile("../output/data.json");
        dataFile << positionsJson.dump(2);
        dataFile.close();
        cout << "Base case positions written to ../output/data.json\n";
    }

    return;
}

void ExecuteAllSweeps2D(unordered_map<string, StockData> &data){
    filesystem::create_directories("../output");
    ofstream file("../output/Returns.txt");
    DowContextInputParameters dowParams;
    DowBaseCase dowBase;
    vector<unique_ptr<ISweepJob>> mySweeps;

    mySweeps.push_back(make_unique<StrategyRunner2D<int, double>>(
        "ATR Period", "ATR Multiplier", 
        dowParams.ATRPeriodArray, dowParams.ATRMultiplierArray, 
        dowBase.balance, 
        [&](int atrPer, double atrMult) { // Lambda captures both parameters
            auto sizer = make_unique<ATRPositionSize>(dowBase.riskAmount, atrPer, atrMult);
            auto context = make_unique<DowContext>(dowBase.lookback, dowBase.doubleLookback, dowBase.signalLookback, dowBase.trendMode, dowBase.trendLineMode);
            return make_unique<CustomStrategy>(dowBase.balance, move(sizer), move(context));
        }
    ));
}

#endif