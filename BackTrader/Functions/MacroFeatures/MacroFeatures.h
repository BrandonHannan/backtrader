#ifndef MACROFEATURES_H
#define MACROFEATURES_H

#include "../../Objects/StockData/StockData.h"
#include "../../include/nlohmann/json.hpp"
#include <string>
#include <vector>
#include <unordered_map>
#include <utility>

using namespace std;

// Cross-asset macro context calculator. For a given primary ticker and entry
// date, computes four aggregate features describing the related tickers'
// behaviour around that date.
class MacroFeatures {
public:
    // sign: +1 for "+", -1 for "-", 0 for "mixed"
    using RelatedMap = unordered_map<string, vector<pair<string, int>>>;

    MacroFeatures(const unordered_map<string, StockData>& data,
                  const RelatedMap& related,
                  int returnLookback = 20,
                  int smaLookback = 20,
                  int atrPeriod = 14,
                  int atrSmaLookback = 50);

    // Returns macroContext JSON for (primaryTicker, primaryDate). Returns
    // a zeroed block with valid:false when no usable related-ticker data
    // exists at the requested date.
    nlohmann::json compute(const string& primaryTicker,
                           const string& primaryDate) const;

private:
    const unordered_map<string, StockData>& data_;
    const RelatedMap& related_;
    int returnLookback_;
    int smaLookback_;
    int atrPeriod_;
    int atrSmaLookback_;

    // Latest index in `dates` whose value is <= target. -1 if target precedes
    // the first available date. ISO YYYY-MM-DD sorts lexicographically.
    static int findFallbackIndex(const vector<string>& dates, const string& target);

    static double pearson(const vector<double>& a, const vector<double>& b);
    static double tStat(const vector<double>& closes, int endIdx, int n);
    static double sma(const vector<double>& v, int endIdx, int n);
    static vector<double> dailyReturns(const vector<double>& closes, int endIdx, int n);
    // Returns ATR(period) at endIdx divided by SMA of the last `smaLookback`
    // ATR values. Returns -1.0 when insufficient history.
    static double currentATR_overSMA(const StockData& s, int endIdx,
                                     int atrPeriod, int atrSmaLookback);
};

#endif
