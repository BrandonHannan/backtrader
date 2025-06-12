#ifndef TRADINGSTRATEGY_H
#define TRADINGSTRATEGY_H

#include "../Position/Position.h"
vector<double> DetermineProfits(vector<Position> &positions);

class TradingStrategy {
    private:
        double balance;
        bool containsOpenPosition;
        Position openPosition;
        vector<Position> closedPositions;
    public:
        TradingStrategy();
        virtual bool shouldExecuteTrade(const vector<double> &data) = 0;

        double getBalance();

        void updateBalance(double update);

        void setBalance(double bal);

        bool getContainsOpenPosition();

        void setContainsOpenPosition(bool cOP);

        Position getOpenPosition();

        void setOpenPosition(Position p);

        vector<Position> getClosedPositions();

        Position getClosedPosition(int index);

        void appendClosedPosition(Position p);

        void setClosedPositions(vector<Position> cP);

        double StandardDeviation();

        double Mean();

        double CalculatePValue();

        virtual ~TradingStrategy() = default;
};

#endif