#ifndef POSITION_H
#define POSITION_H

#include <iostream>
#include "PositionType.h"
#include <string>

using namespace std;

class Position{
    private:
        PositionType positionType;
        string tradeType;
        string purchaseDate;
        string sellDate;
        double purchasePrice;
        double sellPrice;
        double numShares;
        double stopLossPrice;
        
        bool isClosed;

        int toJulian(int y, int m, int d);

        int LengthOfTradeBetweenDates();
    
    public:
        Position();
        Position(string pType, string tType, string pDate, string sDate, double pPrice, double sPrice, double nShares, double sLPrice);

        PositionType getPositionType();

        void setPositionType(PositionType pType);

        string getTradeType();

        void setTradeType(string tradeType);

        string getPurchaseDate();

        void setPurchaseDate(string pDate);

        string getSellDate();

        void setSellDate(string sellDate);

        double getPurchasePrice();

        void setPurchasePrice(double purchasePrice);

        double getStopLossPrice();

        void setStopLossPrice(double stopLossPrice);

        double getSellPrice();

        void setSellPrice(double sellPrice);

        double getNumShares();

        void setNumShares(double numShares);

        bool getIsClosed();

        void setIsClosed(bool isClosed);

        int LengthOfTrade();

        Position& operator=(const Position &obj);
};
#endif