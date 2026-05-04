#ifndef DOWATRSTRATEGY_H
#define DOWATRSTRATEGY_H

#include "../../InputParameters/DowContextParameters/DowContextInputParameters.h"
#include "../../PositionSizing/ATRPositionSize/ATRPositionSize.h"
#include "../../TradingContext/DowContext/DowContext.h"
#include "../../Functions/MacroFeatures/MacroFeatures.h"
#include "../StrategyRunner.h"
#include "DowATRBaseCase.h"
#include <fstream>
#include <filesystem>
#include <iomanip>

using namespace std;

void ExecuteAllSweeps(unordered_map<string, StockData> &data,
                      const MacroFeatures::RelatedMap &related){
    filesystem::create_directories("../output");
    ofstream file("../output/Returns.txt");
    DowContextInputParameters dowParams;
    DowBaseCase dowBase;
    MacroFeatures macro(data, related);
    vector<unique_ptr<ISweepJob>> mySweeps;

    mySweeps.push_back(make_unique<StrategyRunner<int>>(
        "Lookback", dowParams.lookbackPeriodArray, dowBase.balance, [&](int testVal) {
            auto sizer = make_unique<ATRPositionSize>(dowBase.riskAmount, dowBase.atrPeriod, dowBase.atrMultiplier);
            auto context = make_unique<DowContext>(testVal, dowBase.doubleLookback, dowBase.signalLookback, dowBase.trendMode, dowBase.trendLineMode);
            auto strategy = make_unique<CustomStrategy>(dowBase.balance, move(sizer), move(context));
            strategy->setMacroFeatures(&macro);
            return strategy;
        }
    ));

    mySweeps.push_back(make_unique<StrategyRunner<int>>(
        "Double Lookback", dowParams.doubleLookbackPeriodArray, dowBase.balance, [&](int testVal) {
            auto sizer = make_unique<ATRPositionSize>(dowBase.riskAmount, dowBase.atrPeriod, dowBase.atrMultiplier);
            auto context = make_unique<DowContext>(dowBase.lookback, testVal, dowBase.signalLookback, dowBase.trendMode, dowBase.trendLineMode);
            auto strategy = make_unique<CustomStrategy>(dowBase.balance, move(sizer), move(context));
            strategy->setMacroFeatures(&macro);
            return strategy;
        }
    ));

    mySweeps.push_back(make_unique<StrategyRunner<int>>(
        "Signal Lookback", dowParams.signalLookBackPeriodArray, dowBase.balance, [&](int testVal) {
            auto sizer = make_unique<ATRPositionSize>(dowBase.riskAmount, dowBase.atrPeriod, dowBase.atrMultiplier);
            auto context = make_unique<DowContext>(dowBase.lookback, dowBase.doubleLookback, testVal, dowBase.trendMode, dowBase.trendLineMode);
            auto strategy = make_unique<CustomStrategy>(dowBase.balance, move(sizer), move(context));
            strategy->setMacroFeatures(&macro);
            return strategy;
        }
    ));

    mySweeps.push_back(make_unique<StrategyRunner<double>>(
        "Risk Amount", dowParams.RiskAmountArray, dowBase.balance, [&](double testVal) {
            auto sizer = make_unique<ATRPositionSize>(testVal, dowBase.atrPeriod, dowBase.atrMultiplier);
            auto context = make_unique<DowContext>(dowBase.lookback, dowBase.doubleLookback, dowBase.signalLookback, dowBase.trendMode, dowBase.trendLineMode);
            auto strategy = make_unique<CustomStrategy>(dowBase.balance, move(sizer), move(context));
            strategy->setMacroFeatures(&macro);
            return strategy;
        }
    ));

    mySweeps.push_back(make_unique<StrategyRunner<int>>(
        "ATR Period", dowParams.ATRPeriodArray, dowBase.balance, [&](int testVal) {
            auto sizer = make_unique<ATRPositionSize>(dowBase.riskAmount, testVal, dowBase.atrMultiplier);
            auto context = make_unique<DowContext>(dowBase.lookback, dowBase.doubleLookback, dowBase.signalLookback, dowBase.trendMode, dowBase.trendLineMode);
            auto strategy = make_unique<CustomStrategy>(dowBase.balance, move(sizer), move(context));
            strategy->setMacroFeatures(&macro);
            return strategy;
        }
    ));

    mySweeps.push_back(make_unique<StrategyRunner<double>>(
        "ATR Multiplier", dowParams.ATRMultiplierArray, dowBase.balance, [&](double testVal) {
            auto sizer = make_unique<ATRPositionSize>(dowBase.riskAmount, dowBase.atrPeriod, testVal);
            auto context = make_unique<DowContext>(dowBase.lookback, dowBase.doubleLookback, dowBase.signalLookback, dowBase.trendMode, dowBase.trendLineMode);
            auto strategy = make_unique<CustomStrategy>(dowBase.balance, move(sizer), move(context));
            strategy->setMacroFeatures(&macro);
            return strategy;
        }
    ));

    mySweeps.push_back(make_unique<StrategyRunner<TrendMode>>(
        "Trend Mode", dowParams.trendModes, dowBase.balance, [&](TrendMode testVal) {
            auto sizer = make_unique<ATRPositionSize>(dowBase.riskAmount, dowBase.atrPeriod, dowBase.atrMultiplier);
            auto context = make_unique<DowContext>(dowBase.lookback, dowBase.doubleLookback, dowBase.signalLookback, testVal, dowBase.trendLineMode);
            auto strategy = make_unique<CustomStrategy>(dowBase.balance, move(sizer), move(context));
            strategy->setMacroFeatures(&macro);
            return strategy;
        }
    ));

    mySweeps.push_back(make_unique<StrategyRunner<TrendLineMode>>(
        "Trend Line Mode", dowParams.trendLineModes, dowBase.balance, [&](TrendLineMode testVal) {
            auto sizer = make_unique<ATRPositionSize>(dowBase.riskAmount, dowBase.atrPeriod, dowBase.atrMultiplier);
            auto context = make_unique<DowContext>(dowBase.lookback, dowBase.doubleLookback, dowBase.signalLookback, dowBase.trendMode, testVal);
            auto strategy = make_unique<CustomStrategy>(dowBase.balance, move(sizer), move(context));
            strategy->setMacroFeatures(&macro);
            return strategy;
        }
    ));

    RunAllSweeps(mySweeps, file, data);
    file.close();

    // Execute base case and write all positions to data.json
    {
        auto sizer = make_unique<ATRPositionSize>(dowBase.riskAmount, dowBase.atrPeriod, dowBase.atrMultiplier);
        auto context = make_unique<DowContext>(dowBase.lookback, dowBase.doubleLookback, dowBase.signalLookback, dowBase.trendMode, dowBase.trendLineMode);
        auto baseStrategy = make_unique<CustomStrategy>(dowBase.balance, move(sizer), move(context));
        baseStrategy->setMacroFeatures(&macro);

        for (const auto& [ticker, stockData] : data) {
            size_t n = stockData.close.size();
            if (n == 0 || stockData.open.size() != n || stockData.high.size() != n ||
                stockData.low.size() != n || stockData.volume.size() != n || stockData.date.size() != n) continue;
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

void ExecuteBaseCase(unordered_map<string, StockData> &data,
                     const MacroFeatures::RelatedMap &related){
    filesystem::create_directories("../output");
    DowBaseCase dowBase;
    MacroFeatures macro(data, related);

    auto sizer = make_unique<ATRPositionSize>(dowBase.riskAmount, dowBase.atrPeriod, dowBase.atrMultiplier);
    auto context = make_unique<DowContext>(dowBase.lookback, dowBase.doubleLookback, dowBase.signalLookback, dowBase.trendMode, dowBase.trendLineMode);
    auto strategy = make_unique<CustomStrategy>(dowBase.balance, move(sizer), move(context));
    strategy->setMacroFeatures(&macro);

    for (const auto& [ticker, stockData] : data) {
        size_t n = stockData.close.size();
        if (n == 0 || stockData.open.size() != n || stockData.high.size() != n ||
            stockData.low.size() != n || stockData.volume.size() != n || stockData.date.size() != n) continue;
        strategy->ExecuteStrategy(ticker, stockData);
        strategy->setBalance(dowBase.balance);
    }

    // --- Print statistics ---
    const vector<Position>& positions = strategy->getClosedPositions();
    int totalTrades = positions.size();
    int winningTrades = 0, losingTrades = 0, breakEvenTrades = 0;
    int longTrades = 0, shortTrades = 0;
    double totalPnL = 0.0, totalWin = 0.0, totalLoss = 0.0;
    double bestTrade = numeric_limits<double>::lowest();
    double worstTrade = numeric_limits<double>::max();
    int longestTrade = 0, shortestTrade = numeric_limits<int>::max();
    map<string, int> tradeTypeCounts;
    vector<pair<double, const Position*>> pnlList;
    pnlList.reserve(positions.size());

    for (const Position& pos : positions) {
        bool isLong = pos.getPositionType().getPositiontype() == "LONG";
        double pnl = isLong
            ? (pos.getSellPrice() - pos.getPurchasePrice()) * pos.getNumShares()
            : (pos.getPurchasePrice() - pos.getSellPrice()) * pos.getNumShares();

        totalPnL += pnl;
        if (pnl > 0) { winningTrades++; totalWin += pnl; }
        else if (pnl < 0) { losingTrades++; totalLoss += pnl; }
        else { breakEvenTrades++; }

        if (isLong) longTrades++; else shortTrades++;
        if (pnl > bestTrade) bestTrade = pnl;
        if (pnl < worstTrade) worstTrade = pnl;

        int len = pos.LengthOfTrade();
        if (len >= 0) {
            if (len > longestTrade) longestTrade = len;
            if (len < shortestTrade) shortestTrade = len;
        }

        tradeTypeCounts[pos.getTradeType()]++;
        pnlList.emplace_back(pnl, &pos);
    }

    double winRate = totalTrades > 0 ? (100.0 * winningTrades / totalTrades) : 0.0;
    double avgPnL = totalTrades > 0 ? totalPnL / totalTrades : 0.0;
    double avgWin = winningTrades > 0 ? totalWin / winningTrades : 0.0;
    double avgLoss = losingTrades > 0 ? totalLoss / losingTrades : 0.0;
    double profitFactor = (totalLoss != 0.0) ? (totalWin / -totalLoss) : 0.0;

    map<int, vector<double>> yearlyReturns = strategy->getYearlyReturns();

    cout << fixed << setprecision(2);
    cout << "\n========== Base Case Results ==========\n";
    cout << "Total Trades:       " << totalTrades << "\n";
    cout << "Winning Trades:     " << winningTrades << "\n";
    cout << "Losing Trades:      " << losingTrades << "\n";
    cout << "Break-Even Trades:  " << breakEvenTrades << "\n";
    cout << "Win Rate:           " << winRate << "%\n";
    cout << "Long Trades:        " << longTrades << "\n";
    cout << "Short Trades:       " << shortTrades << "\n";
    cout << "\n--- P&L ---\n";
    cout << "Total P&L:          $" << totalPnL << "\n";
    cout << "Avg P&L per Trade:  $" << avgPnL << "\n";
    cout << "Avg Winning Trade:  $" << avgWin << "\n";
    cout << "Avg Losing Trade:   $" << avgLoss << "\n";
    cout << "Best Trade:         $" << (totalTrades > 0 ? bestTrade : 0.0) << "\n";
    cout << "Worst Trade:        $" << (totalTrades > 0 ? worstTrade : 0.0) << "\n";
    cout << "Profit Factor:      " << profitFactor << "\n";
    cout << "\n--- Trade Duration ---\n";
    cout << "Longest Trade:      " << (totalTrades > 0 ? longestTrade : 0) << " days\n";
    cout << "Shortest Trade:     " << (totalTrades > 0 && shortestTrade != numeric_limits<int>::max() ? shortestTrade : 0) << " days\n";
    cout << "\n--- Trade Types ---\n";
    for (const auto& [type, count] : tradeTypeCounts) {
        cout << "  " << type << ": " << count << "\n";
    }
    cout << "\n--- Yearly P&L ---\n";
    for (const auto& [year, pnls] : yearlyReturns) {
        double yearTotal = 0.0;
        for (double p : pnls) yearTotal += p;
        cout << "  " << year << ": $" << yearTotal << " (" << pnls.size() << " trades)\n";
    }

    sort(pnlList.begin(), pnlList.end(), [](const auto& a, const auto& b){ return a.first > b.first; });

    auto printTrade = [](int rank, double pnl, const Position* p) {
        cout << "  #" << rank
             << " | " << p->getStockName()
             << " | " << p->getPositionType().getPositiontype()
             << " | " << p->getTradeType() << "\n";
        cout << "       Entry: " << p->getPurchaseDate() << " @ $" << p->getPurchasePrice()
             << "   Exit: " << p->getSellDate() << " @ $" << p->getSellPrice() << "\n";
        cout << "       Shares: " << p->getNumShares()
             << "   Stop: $" << p->getStopLossPrice()
             << "   Days: " << p->LengthOfTrade()
             << "   P&L: $" << pnl << "\n";
    };

    int topLimit = min(10, (int)pnlList.size());
    cout << "\n--- Top " << topLimit << " Most Profitable Trades ---\n";
    for (int i = 0; i < topLimit; i++) {
        printTrade(i + 1, pnlList[i].first, pnlList[i].second);
    }

    int botLimit = min(10, (int)pnlList.size());
    cout << "\n--- Top " << botLimit << " Least Profitable Trades ---\n";
    for (int i = 0; i < botLimit; i++) {
        int idx = (int)pnlList.size() - 1 - i;
        printTrade(i + 1, pnlList[idx].first, pnlList[idx].second);
    }

    cout << "=======================================\n\n";

    json positionsJson = json::array();
    for (const Position& pos : positions) {
        positionsJson.push_back(pos.toJson());
    }
    ofstream dataFile("../output/data.json");
    dataFile << positionsJson.dump(2);
    dataFile.close();

    cout << "Positions written to ../output/data.json\n";
}

void ExecuteAllSweeps2D(unordered_map<string, StockData> &data,
                        const MacroFeatures::RelatedMap &related){
    filesystem::create_directories("../output");
    ofstream file("../output/Returns.txt");
    DowContextInputParameters dowParams;
    DowBaseCase dowBase;
    MacroFeatures macro(data, related);
    vector<unique_ptr<ISweepJob>> mySweeps;

    mySweeps.push_back(make_unique<StrategyRunner2D<int, double>>(
        "ATR Period", "ATR Multiplier",
        dowParams.ATRPeriodArray, dowParams.ATRMultiplierArray,
        dowBase.balance,
        [&](int atrPer, double atrMult) { // Lambda captures both parameters
            auto sizer = make_unique<ATRPositionSize>(dowBase.riskAmount, atrPer, atrMult);
            auto context = make_unique<DowContext>(dowBase.lookback, dowBase.doubleLookback, dowBase.signalLookback, dowBase.trendMode, dowBase.trendLineMode);
            auto strategy = make_unique<CustomStrategy>(dowBase.balance, move(sizer), move(context));
            strategy->setMacroFeatures(&macro);
            return strategy;
        }
    ));
}

#endif