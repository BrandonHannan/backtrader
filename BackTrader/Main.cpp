#include "DataReader/DataReader.h"
#include "TradingStrategy/TradingStrategy.h"
#include "TradingStrategy/CustomChannelBreakout/CustomChannelBreakout.h"
#include "Position/Position.h"
#include <iostream>
#include <algorithm>
#include <time.h>

int main(){
    // Use this For MacOS
    //unordered_map<string, StockData> data = ReadData("../data.txt");
    // Use this For Windows
    unordered_map<string, StockData> data = ReadData("C:\\Users\\BrandonHannan\\source\\repos\\backtrader\\data.txt");
    cout << "Number of Stocks: " << data.size() << endl;
    for (auto stockData : data){
        cout << "Stock: " << stockData.first << endl;
        StockData x = stockData.second;
        cout << "Open: " << x.open[0] << endl;
        cout << "Close: " << x.close[0] << endl;
        cout << "Low: " << x.low[0] << endl;
        cout << "High: " << x.high[0] << endl;
        cout << "Volume: " << x.volume[0] << endl;
        cout << "Date: " << x.date[0] << endl << endl;
    }
    return 0;
}