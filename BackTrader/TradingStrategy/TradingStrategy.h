#ifndef TRADINGSTRATEGY_H
#define TRADINGSTRATEGY_H

#include "../StockData/StockData.h"
#include "../Position/Position.h"
#include <vector>
#include <unordered_map>
#include <cmath>
#include <random>
#include <deque>
#include <map>

class MaxMinPos {
    public:
        double maxOrMin;
        int index;
        string date;
        MaxMinPos();
        MaxMinPos(double val, int idx, string d);
};

class LookBack {
    public:
        int lookbackPeriod;
        int ATRLookbackPeriod;
        MaxMinPos maxPrice;
        MaxMinPos minPrice;
        MaxMinPos maxVol;
        MaxMinPos minVol;

        double sumATR;
        deque<double> trueRangeWindow;

        double sumPrice;
        double sumSQPrice;
        double sumVol;
        double sumSQVol;

        double sumDiffPrice;
        double sumDiffPricePrev;

        double sumPricePrev;
        double sumSQPricePrev;
        double sumVolPrev;
        double sumSQVolPrev;

        LookBack() {}
        LookBack(int lbP, int ATRlbP);

        void updateLookBackSumPrice(double currentPrice, double prevPrice, double prevPrevPrice);

        void updateLookBackSumVolume(double currentVol, double prevVol, double prevPrevVol);

        void updateLookBackSumDiffPrice(double currentDiffPrice, double prevDiffPrice, double prevPrevDiffPrice);

        void updateATR(double maximum);

        void updateMaxPrice(double p);

        void updateMinPrice(double p);

        void updateMaxVolume(double v);

        void updateMinVolume(double v);

        double DetermineATR();

        double DetermineMeanPrice();

        double DetermineMeanPricePrev();

        double DetermineMeanDiffPrice();

        double DetermineMeanDiffPricePrev();

        double DetermineMeanVolume();

        double DetermineMeanVolumePrev();

        double DetermineSTDPrice();

        double DetermineProbGreaterDiffPrice(double val);

        double DetermineSTDPricePrev();

        double DetermineSTDVolume();

        double DetermineSTDVolumePrev();
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

        virtual void ExecuteStrategy(const StockData &data) = 0;

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