#ifndef POSITION_H
#define POSITION_H

#include <iostream>
#include <string>

using namespace std;

enum PositionType{
    SHORT,
    LONG,
    NONE
};

class Position{
    private:
        PositionType positionType;
        string purchaseDate;
        string sellDate;
        double purchasePrice;
        double sellPrice;
        double numShares;
        bool isClosed;

        int toJulian(int y, int m, int d);

        int LengthOfTradeBetweenDates();

    public:
        Position();
        Position(PositionType pType, string pDate, string sDate, double pPrice, double sPrice, double nShares, bool isC);

        PositionType getPositionType();

        void setPositionType(PositionType pType);

        string getPurchaseDate();

        void setPurchaseDate(string pDate);

        string getSellDate();

        void setSellDate(string sDate);

        double getPurchasePrice();

        void setPurchasePrice(double pPrice);

        double getSellPrice();

        void setSellPrice(double sPrice);

        double getNumShares();

        void setNumShares(double nShares);

        bool getIsClosed();

        void setIsClosed(bool isC);

        int LengthOfTrade();
};

#endif