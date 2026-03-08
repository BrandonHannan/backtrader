#ifndef POSITION_H
#define POSITION_H

#include <iostream>
#include "PositionType.h"
#include "../../Functions/DateHelper.h"
#include "../StockData/StockData.h"
#include <string>
#include <format>

using namespace std;

class Position{
    private:
        string stockName;
        PositionType positionType; // Signals either a LONG or a SHORT trade
        string tradeType; // Signals the type of trade
        string purchaseDate;
        string sellDate;
        double purchasePrice;
        double sellPrice;
        double numShares;
        double stopLossPrice;
        string stats;

        double originalStopLossPrice;
        
        bool isClosed;

        int LengthOfTradeBetweenDates(const string &purchaseDate, const string &sellDate) const;
    
    public:
        Position();
        Position(string stockName, string pType, string tType, string pDate, string sDate, double pPrice, double sPrice, double nShares, double sLPrice);

        string getStockName() const;

        void setStockName(string stockName);

        PositionType getPositionType() const;

        void setPositionType(PositionType pType);

        void setPositionType(string pType);

        string getTradeType() const;

        void setTradeType(string tradeType);

        string getPurchaseDate() const;

        void setPurchaseDate(string pDate);

        string getSellDate() const;

        void setSellDate(string sellDate);

        double getPurchasePrice() const;

        void setPurchasePrice(double purchasePrice);

        double getOriginalStopLossPrice() const;

        double getStopLossPrice() const;

        void setStopLossPrice(double stopLossPrice);

        double getSellPrice() const;

        void setSellPrice(double sellPrice);

        double getNumShares() const;

        void setNumShares(double numShares);

        string getStats() const;

        void setStats(string stats);

        bool getIsClosed() const;

        void setIsClosed(bool isClosed);

        int LengthOfTrade() const;

        int currentLengthOfTrade(string currentDate) const;

        string getBasePositionInfo() const;

        double getExitPrice(const StockDataInstance &currentData, const StockDataInstance &futureData) const;

        Position& operator=(const Position &obj);
};
#endif