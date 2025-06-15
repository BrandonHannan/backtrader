#ifndef TRADINGSTRATEGY_H
#define TRADINGSTRATEGY_H

#include "../Position/Position.h"
#include <vector>
#include <unordered_map>
#include <map>

class MaxMinPos {
    public:
        double maxOrMin;
        int index;
        string date;
        MaxMinPos();
        MaxMinPos(double val, int idx, string d);
};

class TradingStrategy {
    private:
        bool containsOpenPosition;
        Position openPosition;
        vector<Position> closedPositions;
    public:
        double balance;

        TradingStrategy();
        TradingStrategy(double bal, bool cOP, Position pos, vector<Position> cPoses);

        virtual void ExecuteTrade(const StockData &data) = 0;

        bool getContainsOpenPosition();

        void setContainsOpenPosition(bool cOP);

        Position getOpenPosition();

        void setOpenPosition(Position p);

        vector<Position> getClosedPositions();

        Position getClosedPosition(int index);

        void appendClosedPosition(Position p);

        void setClosedPositions(vector<Position> cP);

        vector<double> getAllReturns();

        map<int, vector<double>> getYearlyReturns();

        virtual ~TradingStrategy() = default;
};

#endif