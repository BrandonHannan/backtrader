#ifndef BASESTRATEGY_H
#define BASESTRATEGY_H
#include "../PositionType/Position.h"
#include "../StockData/StockData.h"
#include "../PositionSizing/BasePositionSize.h"
#include <unordered_map>
#include <map>
#include <cmath>
#include <vector>
#include <algorithm>
#include <numeric>
#include <limits>
#include <memory>

using namespace std;

class BaseStrategy {
    private:
        double balance;
        unique_ptr<BasePositionSize> positionSizer;
        Position position;
        vector<Position> closedPositions;
    public:
        BaseStrategy(double bal, unique_ptr<BasePositionSize> sizer);

        virtual void ExecuteStrategy(const string &stockName, const StockData &data) = 0;

        double getBalance();

        void addToBalance(double val);

        BasePositionSize* getPositionSizer();

        void setPositionSizer(unique_ptr<BasePositionSize> sizer);

        Position getPosition();

        void setPosition(Position &p);

        vector<Position> getClosedPositions();

        Position getClosedPosition(int index);

        void appendClosedPosition(Position p);

        void setClosedPositions(vector<Position> cP);

        vector<double> getAllReturns();

        map<int, vector<double>> getYearlyReturns();

        virtual ~BaseStrategy() = default;
};
#endif