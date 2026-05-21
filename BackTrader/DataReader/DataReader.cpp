#include "DataReader.h"
#include "DataCache.h"
#include "../StrategyRunner/ThreadPool.h"
#include <cstdlib>
#include <cstring>
#include <future>
#include <memory>
#include <utility>

namespace {

// Parse all whitespace-separated doubles in [p, end) into `out` using strtod.
// strtod is locale-aware; the engine sets LC_NUMERIC="C" once in main so the
// decimal separator is always '.'.
inline void parseDoublesInto(const char* p, const char* end, vector<double>& out) {
    char* tail;
    while (p < end) {
        while (p < end && (*p == ' ' || *p == '\t')) ++p;
        if (p >= end) break;
        double v = strtod(p, &tail);
        if (tail == p) break;
        out.push_back(v);
        p = tail;
    }
}

// Parse all whitespace-separated tokens in [p, end) into `out` as strings.
inline void parseTokensInto(const char* p, const char* end, vector<string>& out) {
    while (p < end) {
        while (p < end && (*p == ' ' || *p == '\t')) ++p;
        if (p >= end) break;
        const char* start = p;
        while (p < end && *p != ' ' && *p != '\t') ++p;
        out.emplace_back(start, p - start);
    }
}

} // namespace

unordered_map<string, StockData> ReadData(const string &fileName){
    unordered_map<string, StockData> result;
    ifstream file;

    // Large I/O buffer reduces syscalls on multi-GB files. Must be set before open().
    auto ioBuffer = std::make_unique<char[]>(8 * 1024 * 1024);
    file.rdbuf()->pubsetbuf(ioBuffer.get(), 8 * 1024 * 1024);

    file.open(fileName);
    if (!file.is_open()){
        cerr << "Error Opening File: " << fileName << endl;
        return {};
    }

    string line;
    line.reserve(16 * 1024 * 1024); // OHLCV lines in minute files can span several MB
    string currentStockTicker;

    // Temporary storage for the current stock's data
    vector<double> temp_open, temp_close, temp_high, temp_low, temp_volume;
    vector<string> temp_date;
    double temp_contractSize = 0.0;
    double temp_frictionPerRoundTrip = 0.0;

    // Hoisted dispatch: which vector does the current data section append into?
    // Exactly one of these is non-null while inside an OHLCV/Date section.
    vector<double>* active_double_vec = nullptr;
    vector<string>* active_string_vec = nullptr;
    enum class ScalarTarget { None, ContractSize, FrictionPerRoundTrip };
    ScalarTarget active_scalar = ScalarTarget::None;

    auto flush_current = [&]() {
        if (currentStockTicker.empty()) return;
        if (temp_contractSize <= 0.0) {
            cerr << "[Error] " << currentStockTicker
                 << ": missing/invalid ContractSize - dropping ticker" << endl;
            return;
        }
        size_t n = temp_open.size();
        if (temp_close.size() != n || temp_high.size() != n || temp_low.size() != n ||
            temp_volume.size() != n || temp_date.size() != n) {
            cerr << "[Error] " << currentStockTicker
                 << ": OHLCV+date vector lengths do not match (open=" << n
                 << " close=" << temp_close.size() << " high=" << temp_high.size()
                 << " low=" << temp_low.size() << " vol=" << temp_volume.size()
                 << " date=" << temp_date.size() << ") - dropping ticker" << endl;
            return;
        }
        result[currentStockTicker] = StockData(
            std::move(temp_open), std::move(temp_close), std::move(temp_high),
            std::move(temp_low), std::move(temp_volume), std::move(temp_date),
            temp_contractSize, temp_frictionPerRoundTrip);
    };

    while (getline(file, line)) {
        // Trim a trailing \r left behind by CRLF line endings on Windows-authored files.
        if (!line.empty() && line.back() == '\r') line.pop_back();

        if (line.rfind("Stock: ", 0) == 0) {
            flush_current();

            temp_open.clear();
            temp_close.clear();
            temp_high.clear();
            temp_low.clear();
            temp_volume.clear();
            temp_date.clear();
            temp_contractSize = 0.0;
            temp_frictionPerRoundTrip = 0.0;
            active_double_vec = nullptr;
            active_string_vec = nullptr;
            active_scalar = ScalarTarget::None;

            currentStockTicker = line.substr(7);
            continue;
        }

        // Section headers — resolve target once per header, not per token.
        if (line == "ContractSize:") {
            active_double_vec = nullptr; active_string_vec = nullptr;
            active_scalar = ScalarTarget::ContractSize;
            continue;
        }
        if (line == "FrictionPerRoundTrip:") {
            active_double_vec = nullptr; active_string_vec = nullptr;
            active_scalar = ScalarTarget::FrictionPerRoundTrip;
            continue;
        }
        if (line == "Open:")   { active_double_vec = &temp_open;   active_string_vec = nullptr; active_scalar = ScalarTarget::None; continue; }
        if (line == "Close:")  { active_double_vec = &temp_close;  active_string_vec = nullptr; active_scalar = ScalarTarget::None; continue; }
        if (line == "High:")   { active_double_vec = &temp_high;   active_string_vec = nullptr; active_scalar = ScalarTarget::None; continue; }
        if (line == "Low:")    { active_double_vec = &temp_low;    active_string_vec = nullptr; active_scalar = ScalarTarget::None; continue; }
        if (line == "Volume:") { active_double_vec = &temp_volume; active_string_vec = nullptr; active_scalar = ScalarTarget::None; continue; }
        if (line == "Date:")   { active_double_vec = nullptr; active_string_vec = &temp_date; active_scalar = ScalarTarget::None; continue; }

        if (line.empty()) continue;

        const char* p   = line.data();
        const char* end = p + line.size();

        if (active_double_vec) {
            // ~8 chars per double on average (incl. separator). Slight over-reserve is far cheaper than a realloc mid-loop.
            active_double_vec->reserve(active_double_vec->size() + line.size() / 8 + 1);
            parseDoublesInto(p, end, *active_double_vec);
        } else if (active_string_vec) {
            active_string_vec->reserve(active_string_vec->size() + line.size() / 11 + 1); // YYYY-MM-DD + space ≈ 11 chars
            parseTokensInto(p, end, *active_string_vec);
        } else if (active_scalar == ScalarTarget::ContractSize) {
            char* tail;
            double v = strtod(p, &tail);
            if (tail != p) temp_contractSize = v;
        } else if (active_scalar == ScalarTarget::FrictionPerRoundTrip) {
            char* tail;
            double v = strtod(p, &tail);
            if (tail != p) temp_frictionPerRoundTrip = v;
        }
    }

    flush_current();
    file.close();
    return result;
}

unordered_map<string, StockData> ReadDukascopyData(const string &bidFile, const string &askFile) {
    // Parse bid and ask in parallel. Each side is CPU-bound during parsing once data is in the
    // page cache, so two threads is the right amount: more threads would just thrash the disk.
    ThreadPool pool(2);
    auto bidFut = pool.submit([&]{ return ReadDataCached(bidFile); });
    auto askFut = pool.submit([&]{ return ReadDataCached(askFile); });
    unordered_map<string, StockData> base = bidFut.get();
    unordered_map<string, StockData> ask  = askFut.get();

    if (base.empty()) {
        cerr << "[Error] ReadDukascopyData: bid file produced no tickers (" << bidFile << ")" << endl;
        return {};
    }
    if (ask.empty()) {
        cerr << "[Error] ReadDukascopyData: ask file produced no tickers (" << askFile << ")" << endl;
        return {};
    }

    for (auto it = base.begin(); it != base.end(); ) {
        const string &ticker = it->first;
        StockData &bid = it->second;
        auto askIt = ask.find(ticker);

        if (askIt == ask.end()) {
            cerr << "[Warning] " << ticker << ": present in bid file but missing from ask file - dropping" << endl;
            it = base.erase(it);
            continue;
        }

        StockData &askData = askIt->second;
        if (askData.date.size() != bid.date.size() || askData.open.size() != bid.open.size()) {
            cerr << "[Warning] " << ticker << ": bid/ask array sizes differ (bid="
                 << bid.date.size() << ", ask=" << askData.date.size() << ") - dropping" << endl;
            it = base.erase(it);
            continue;
        }

        bool datesAlign = true;
        for (size_t k = 0; k < bid.date.size(); ++k) {
            if (bid.date[k] != askData.date[k]) { datesAlign = false; break; }
        }
        if (!datesAlign) {
            cerr << "[Warning] " << ticker << ": bid/ask date arrays do not align - dropping" << endl;
            it = base.erase(it);
            continue;
        }

        // `ask` is discarded at the end of this function, so move instead of copy.
        bid.askOpen  = std::move(askData.open);
        bid.askClose = std::move(askData.close);
        bid.askHigh  = std::move(askData.high);
        bid.askLow   = std::move(askData.low);
        ++it;
    }

    return base;
}
