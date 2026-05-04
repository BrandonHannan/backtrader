#include "MacroFeatures.h"
#include "../WindowStatistics.h"
#include "../../Indicators/ATR/ATR.h"
#include <algorithm>
#include <cmath>

using json = nlohmann::json;

namespace {
constexpr double EPS = 1e-14;
}

MacroFeatures::MacroFeatures(const unordered_map<string, StockData>& data,
                             const RelatedMap& related,
                             int returnLookback,
                             int smaLookback,
                             int atrPeriod,
                             int atrSmaLookback)
    : data_(data),
      related_(related),
      returnLookback_(returnLookback),
      smaLookback_(smaLookback),
      atrPeriod_(atrPeriod),
      atrSmaLookback_(atrSmaLookback) {}

int MacroFeatures::findFallbackIndex(const vector<string>& dates, const string& target) {
    if (dates.empty()) return -1;
    auto it = std::upper_bound(dates.begin(), dates.end(), target);
    if (it == dates.begin()) return -1;
    return static_cast<int>(std::distance(dates.begin(), it) - 1);
}

double MacroFeatures::pearson(const vector<double>& a, const vector<double>& b) {
    int n = static_cast<int>(a.size());
    if (n < 2 || n != static_cast<int>(b.size())) return 0.0;
    double meanA = 0.0, meanB = 0.0;
    for (int i = 0; i < n; ++i) { meanA += a[i]; meanB += b[i]; }
    meanA /= n;
    meanB /= n;
    double num = 0.0, denA = 0.0, denB = 0.0;
    for (int i = 0; i < n; ++i) {
        double da = a[i] - meanA;
        double db = b[i] - meanB;
        num  += da * db;
        denA += da * da;
        denB += db * db;
    }
    double den = std::sqrt(denA * denB);
    if (den < EPS) return 0.0;
    double r = num / den;
    if (r > 1.0)  r = 1.0;
    if (r < -1.0) r = -1.0;
    return r;
}

double MacroFeatures::tStat(const vector<double>& closes, int endIdx, int n) {
    int startIdx = endIdx - n + 1;
    if (startIdx < 0 || endIdx >= static_cast<int>(closes.size())) return 0.0;
    WindowStatistics ws(n);
    for (int i = startIdx; i <= endIdx; ++i) {
        ws.addDataPoint(closes[i]);
    }
    double slope = ws.getSlope();
    double slopeSE = ws.getSlopeSE();
    if (std::isnan(slope) || std::isnan(slopeSE) || std::abs(slopeSE) < EPS) return 0.0;
    return slope / slopeSE;
}

double MacroFeatures::sma(const vector<double>& v, int endIdx, int n) {
    int startIdx = endIdx - n + 1;
    if (startIdx < 0 || endIdx >= static_cast<int>(v.size())) return 0.0;
    double sum = 0.0;
    for (int i = startIdx; i <= endIdx; ++i) sum += v[i];
    return sum / n;
}

vector<double> MacroFeatures::dailyReturns(const vector<double>& closes, int endIdx, int n) {
    int startIdx = endIdx - n + 1;
    if (startIdx - 1 < 0 || endIdx >= static_cast<int>(closes.size())) return {};
    vector<double> ret;
    ret.reserve(n);
    for (int i = startIdx; i <= endIdx; ++i) {
        double prev = closes[i - 1];
        if (std::abs(prev) < EPS) {
            ret.push_back(0.0);
        } else {
            ret.push_back((closes[i] - prev) / prev);
        }
    }
    return ret;
}

double MacroFeatures::currentATR_overSMA(const StockData& s, int endIdx,
                                         int atrPeriod, int atrSmaLookback) {
    int barsNeeded = atrPeriod + atrSmaLookback - 1;
    int startIdx = endIdx - barsNeeded + 1;
    if (startIdx < 1) return -1.0;
    if (endIdx >= static_cast<int>(s.close.size())) return -1.0;

    ATR atr(atrPeriod);
    vector<double> atrValues;
    atrValues.reserve(atrSmaLookback);

    for (int i = startIdx; i <= endIdx; ++i) {
        StockDataInstance current(i, s.open[i], s.close[i], s.high[i], s.low[i], s.volume[i], s.date[i]);
        StockDataInstance previous(i - 1, s.open[i - 1], s.close[i - 1], s.high[i - 1], s.low[i - 1], s.volume[i - 1], s.date[i - 1]);
        atr.processNewData(current, previous);
        if (atr.isReady()) {
            atrValues.push_back(atr.getATR());
        }
    }

    if (static_cast<int>(atrValues.size()) < atrSmaLookback) return -1.0;

    double currentATR = atrValues.back();
    double sum = 0.0;
    int start = static_cast<int>(atrValues.size()) - atrSmaLookback;
    for (int i = start; i < static_cast<int>(atrValues.size()); ++i) sum += atrValues[i];
    double smaATR = sum / atrSmaLookback;
    if (smaATR < EPS) return 0.0;
    return currentATR / smaATR;
}

json MacroFeatures::compute(const string& primary, const string& primaryDate) const {
    json invalid = {
        {"agreementScore",   0.0},
        {"relativeMomentum", 0.0},
        {"confluenceRatio",  0.0},
        {"ecosystemVolRatio", 0.0},
        {"valid", false}
    };

    auto relIt = related_.find(primary);
    if (relIt == related_.end() || relIt->second.empty()) return invalid;

    auto dataIt = data_.find(primary);
    if (dataIt == data_.end()) return invalid;
    const StockData& primaryData = dataIt->second;
    {
        size_t n = primaryData.close.size();
        if (n == 0 || primaryData.date.size() != n) return invalid;
    }

    int endIdx_primary = findFallbackIndex(primaryData.date, primaryDate);
    if (endIdx_primary < returnLookback_) return invalid;

    vector<double> primaryReturns = dailyReturns(primaryData.close, endIdx_primary, returnLookback_);
    if (static_cast<int>(primaryReturns.size()) != returnLookback_) return invalid;

    double primaryT = tStat(primaryData.close, endIdx_primary, returnLookback_);

    double agreementSum = 0.0;
    int agreementCount = 0;
    double tStatSum = 0.0;
    int tStatCount = 0;
    int confluenceMatch = 0;
    int confluenceTotal = 0;
    double volSum = 0.0;
    int volCount = 0;

    for (const auto& [relTicker, sign] : relIt->second) {
        auto rdIt = data_.find(relTicker);
        if (rdIt == data_.end()) continue;
        const StockData& rd = rdIt->second;
        size_t rn = rd.close.size();
        if (rn == 0 || rd.open.size() != rn || rd.high.size() != rn ||
            rd.low.size() != rn || rd.volume.size() != rn || rd.date.size() != rn) continue;

        int endIdx_k = findFallbackIndex(rd.date, primaryDate);
        if (endIdx_k < returnLookback_) continue;

        vector<double> relReturns = dailyReturns(rd.close, endIdx_k, returnLookback_);
        if (static_cast<int>(relReturns.size()) != returnLookback_) continue;

        if (sign != 0) {
            double r = pearson(primaryReturns, relReturns);
            agreementSum += static_cast<double>(sign) * r;
            ++agreementCount;

            double rT = tStat(rd.close, endIdx_k, returnLookback_);
            tStatSum += static_cast<double>(sign) * rT;
            ++tStatCount;

            if (endIdx_k - smaLookback_ + 1 >= 0) {
                double smaVal = sma(rd.close, endIdx_k, smaLookback_);
                double curClose = rd.close[endIdx_k];
                bool match = (sign > 0) ? (curClose > smaVal) : (curClose < smaVal);
                if (match) ++confluenceMatch;
                ++confluenceTotal;
            }
        }

        double volRatio = currentATR_overSMA(rd, endIdx_k, atrPeriod_, atrSmaLookback_);
        if (volRatio >= 0.0) {
            volSum += volRatio;
            ++volCount;
        }
    }

    if (agreementCount == 0 && tStatCount == 0 && volCount == 0) return invalid;

    json result;
    result["agreementScore"]   = (agreementCount > 0) ? (agreementSum / agreementCount) : 0.0;
    double meanRelT = (tStatCount > 0) ? (tStatSum / tStatCount) : 0.0;
    result["relativeMomentum"] = primaryT - meanRelT;
    result["confluenceRatio"]  = (confluenceTotal > 0) ? (static_cast<double>(confluenceMatch) / confluenceTotal) : 0.0;
    result["ecosystemVolRatio"] = (volCount > 0) ? (volSum / volCount) : 0.0;
    result["valid"] = true;
    return result;
}
