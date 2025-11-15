#ifndef POSITION_H
#define POSITION_H

#include <iostream>
#include <string>

using namespace std;

enum TradeType {
    HVS,
    HNVSL,
    HNVHL,
    HNVHHVL,
    LVL, 
    LNVSS,
    LNVLS,
    LNVLHVS,
    NOTHING
};

enum PositionType{
    SHORT,
    LONG,
    NONE
};

class PositionStats{
    public:
        double pValue;
        double pValuePrev;
        double priceSlope;
        double priceSlopePrev;
        double prev;
        double current;
        PositionStats();
        PositionStats(double pV, double pVP, double pS, double pSP, double p, double c);
};

class Position{
    private:
        PositionType positionType;
        TradeType tradeType;
        string purchaseDate;
        string sellDate;
        double purchasePrice;
        double stopLossPrice;
        double sellPrice;
        double numShares;
        bool isClosed;
        PositionStats stats;

        int toJulian(int y, int m, int d);

        int LengthOfTradeBetweenDates();

    public:
        Position();
        Position(PositionType pType, TradeType tType, string pDate, string sDate, double pPrice, double sLP,
        double sPrice, double nShares, bool isC, PositionStats s);

        PositionType getPositionType();

        void setPositionType(PositionType pType);

        TradeType getTradeType();

        void setTradeType(TradeType tType);

        string getPurchaseDate();

        void setPurchaseDate(string pDate);

        string getSellDate();

        void setSellDate(string sDate);

        double getPurchasePrice();

        void setPurchasePrice(double pPrice);

        double getStopLossPrice();

        void setStopLossPrice(double sLP);

        double getSellPrice();

        void setSellPrice(double sPrice);

        double getNumShares();

        void setNumShares(double nShares);

        bool getIsClosed();

        void setIsClosed(bool isC);

        int LengthOfTrade();

        PositionStats getStats();

        void setStats(PositionStats s);

        Position& operator=(const Position &obj);
};

#endif