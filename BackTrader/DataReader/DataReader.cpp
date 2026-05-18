#include "DataReader.h"

unordered_map<string, StockData> ReadData(const string &fileName){
    unordered_map<string, StockData> result;
    ifstream file;
    file.open(fileName);
    if (!file.is_open()){
        cerr << "Error Opening File: " << fileName << endl;
        return {};
    }

    string line;
    string currentStockTicker;
    string currentDataType;

    // Temporary storage for the current stock's data
    vector<double> temp_open, temp_close, temp_high, temp_low, temp_volume;
    vector<string> temp_date;
    double temp_contractSize = 0.0;
    double temp_frictionPerRoundTrip = 0.0;

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
            temp_open, temp_close, temp_high, temp_low, temp_volume,
            temp_date, temp_contractSize, temp_frictionPerRoundTrip);
    };

    while (getline(file, line)) {
        // Check for a new stock entry
        if (line.rfind("Stock: ", 0) == 0) {
            // If we were already processing a stock, save its data before starting the new one
            flush_current();

            // Clear temporary vectors for the new stock
            temp_open.clear();
            temp_close.clear();
            temp_high.clear();
            temp_low.clear();
            temp_volume.clear();
            temp_date.clear();
            temp_contractSize = 0.0;
            temp_frictionPerRoundTrip = 0.0;
            currentDataType.clear();

            // Extract the new stock ticker
            currentStockTicker = line.substr(7);
        }
        // Identify the data type header (ContractSize, Open, Close, etc.)
        else if (line == "ContractSize:") { currentDataType = "ContractSize"; }
        else if (line == "FrictionPerRoundTrip:") { currentDataType = "FrictionPerRoundTrip"; }
        else if (line == "Open:") { currentDataType = "Open"; }
        else if (line == "Close:") { currentDataType = "Close"; }
        else if (line == "High:") { currentDataType = "High"; }
        else if (line == "Low:") { currentDataType = "Low"; }
        else if (line == "Volume:") { currentDataType = "Volume"; }
        else if (line == "Date:") { currentDataType = "Date"; }
        // Otherwise, it's a data line
        else if (!line.empty() && !currentDataType.empty()) {
            istringstream iss(line);
            if (currentDataType == "Date") {
                string dateValue;
                while (iss >> dateValue) {
                    temp_date.push_back(dateValue);
                }
            }
            else if (currentDataType == "ContractSize") {
                double cs;
                if (iss >> cs) { temp_contractSize = cs; }
            }
            else if (currentDataType == "FrictionPerRoundTrip") {
                double fr;
                if (iss >> fr) { temp_frictionPerRoundTrip = fr; }
            }
            else {
                double numericValue;
                while (iss >> numericValue) {
                    if (currentDataType == "Open") temp_open.push_back(numericValue);
                    else if (currentDataType == "Close") temp_close.push_back(numericValue);
                    else if (currentDataType == "High") temp_high.push_back(numericValue);
                    else if (currentDataType == "Low") temp_low.push_back(numericValue);
                    else if (currentDataType == "Volume") temp_volume.push_back(numericValue);
                }
            }
        }
    }

    // After the loop, save the very last stock's data
    flush_current();
    file.close();
    return result;
}

unordered_map<string, StockData> ReadDukascopyData(const string &bidFile, const string &askFile) {
    unordered_map<string, StockData> base = ReadData(bidFile);
    unordered_map<string, StockData> ask  = ReadData(askFile);

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

        const StockData &askData = askIt->second;
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

        bid.askOpen  = askData.open;
        bid.askClose = askData.close;
        bid.askHigh  = askData.high;
        bid.askLow   = askData.low;
        ++it;
    }

    return base;
}
