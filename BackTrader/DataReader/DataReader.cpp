#include "../StockData/StockData.h"
#include <unordered_map>
#include <fstream>
#include <sstream>
#include <iostream>

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

    while (getline(file, line)) {
        // Check for a new stock entry
        if (line.rfind("Stock: ", 0) == 0) {
            // If we were already processing a stock, save its data before starting the new one
            if (!currentStockTicker.empty()) {
                result[currentStockTicker] = StockData(temp_open, temp_close, temp_high, temp_low, temp_volume, temp_date);
            }
            
            // Clear temporary vectors for the new stock
            temp_open.clear();
            temp_close.clear();
            temp_high.clear();
            temp_low.clear();
            temp_volume.clear();
            temp_date.clear();

            // Extract the new stock ticker
            currentStockTicker = line.substr(7);
        }
        // Identify the data type header (Open, Close, etc.)
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
    if (!currentStockTicker.empty()) {
        result[currentStockTicker] = StockData(temp_open, temp_close, temp_high, temp_low, temp_volume, temp_date);
    }
    file.close();
    return result;
}