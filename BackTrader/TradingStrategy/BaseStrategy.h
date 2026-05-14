#ifndef BASESTRATEGY_H
#define BASESTRATEGY_H
#include "../Objects/PositionType/Position.h"
#include "../Objects/StockData/StockData.h"
#include "../PositionSizing/BasePositionSize.h"
#include "../TradingContext/BaseContext.h"
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
        // Pass any class that inherits the class BasePositionSize using the make_unique<inheritedClass> function
        // E.g. std::make_unique<ATRPositionSize>(risk, atrPeriod, atrMultiplier)
        // And you can use it like this E.g. getPositionSizer()->processNewData(currentBar, previousBar);
        // Note: You can only use the functions defined by the BasePositionSize class
        unique_ptr<BasePositionSize> positionSizer;
        unique_ptr<BaseContext> strategyContext;
        Position position;
        vector<Position> closedPositions;
    public:
        BaseStrategy(double bal, unique_ptr<BasePositionSize> sizer, unique_ptr<BaseContext> context);

        virtual void ExecuteStrategy(const string &stockName, const StockData &data) = 0;

        virtual void ExecuteStrategy(const string &stockName, const StockData &data, const StockData &minuteData) = 0;

        double getBalance();

        void addToBalance(double val);

        void setBalance(double val);

        BasePositionSize* getPositionSizer();

        void setPositionSizer(unique_ptr<BasePositionSize> sizer);

        BaseContext* getContext();

        void setContext(unique_ptr<BaseContext> context);

        Position &getPosition();

        void setPosition(Position &p);

        vector<Position> getClosedPositions();

        Position getClosedPosition(int index);

        void appendClosedPosition(Position p);

        void setClosedPositions(vector<Position> cP);

        vector<double> getAllReturns();

        map<int, vector<double>> getYearlyReturns();

        virtual ~BaseStrategy() = default;
};

int binarySearchDate(const vector<string> &dates, const string &targetDate);
int binarySearchInitialMinuteDate(const vector<string> &minuteDates, const string &targetDailyDate);
int binarySearchLastMinuteDate(const vector<string> &minuteDates, const string &targetDailyDate);
#endif