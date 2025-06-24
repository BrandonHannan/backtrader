#include "../StockData/StockData.h"
#include "../Position/Position.h"
#include <unordered_map>
#include <cmath>
#include <random>
#include <deque>
#include <map>
#include <vector>
#include <algorithm>
#include <numeric>
#include <limits>

using namespace std;

class MaxMinPos {
    public:
        double maxOrMin;
        int index;
        string date;
        MaxMinPos() {}
        MaxMinPos(double val, int idx, string d): maxOrMin(val), index(idx), date(d) {}
};

class LookBack {
    public:
        int lookbackPeriod;
        int ATRLookbackPeriod;
        MaxMinPos maxPrice;
        MaxMinPos minPrice;
        MaxMinPos maxVol;
        MaxMinPos minVol;

        double sumATR = 0;
        deque<double> trueRangeWindow;

        double sumPrice = 0;
        double sumSQPrice = 0;
        double sumVol= 0;
        double sumSQVol = 0;

        double sumDiffPrice = 0;
        double sumDiffPricePrev = 0;

        double sumPricePrev = 0;
        double sumSQPricePrev = 0;
        double sumVolPrev = 0;
        double sumSQVolPrev = 0;

        LookBack() {}
        LookBack(int lbP, int ATRlbP): lookbackPeriod(lbP), ATRLookbackPeriod(ATRlbP), maxPrice(MaxMinPos(-99999999, -1, "")), 
        minPrice(MaxMinPos(99999999, -1, "")), maxVol(MaxMinPos(-99999999, -1, "")), minVol(MaxMinPos(99999999, -1, "")) {}

        void updateLookBackSumPrice(double currentPrice, double prevPrice, double prevPrevPrice){
            this->sumPricePrev = this->sumPricePrev - prevPrevPrice + prevPrice;
            this->sumSQPricePrev = this->sumSQPricePrev - prevPrevPrice * prevPrevPrice + prevPrice * prevPrice;
            this->sumPrice = this->sumPrice - prevPrice + currentPrice;
            this->sumSQPrice = this->sumSQPrice - prevPrice * prevPrice + currentPrice * currentPrice;
        }

        void updateLookBackSumVolume(double currentVol, double prevVol, double prevPrevVol){
            this->sumVolPrev = this->sumVolPrev - prevPrevVol + prevVol;
            this->sumSQVolPrev = this->sumSQVolPrev - prevPrevVol * prevPrevVol + prevVol * prevVol;
            this->sumVol = this->sumVol - prevVol + currentVol;
            this->sumSQVol = this->sumSQVol - prevVol * prevVol + currentVol * currentVol;
        }

        void updateLookBackSumDiffPrice(double currentDiffPrice, double prevDiffPrice, double prevPrevDiffPrice){
            this->sumDiffPrice = this->sumDiffPrice - prevDiffPrice + currentDiffPrice;
            this->sumDiffPricePrev = this->sumDiffPricePrev - prevPrevDiffPrice + prevDiffPrice;
        }

        void updateATR(double maximum){
            this->sumATR += maximum;
            this->trueRangeWindow.push_back(maximum);
            if (this->trueRangeWindow.size() > ATRLookbackPeriod){
                this->sumATR -= this->trueRangeWindow.front();
                this->trueRangeWindow.pop_front();
            }
        }

        void updateMaxPrice(double p){
            this->maxPrice.maxOrMin = max(this->maxPrice.maxOrMin, p);
        }

        void updateMinPrice(double p){
            this->minPrice.maxOrMin = min(this->minPrice.maxOrMin, p);
        }

        void updateMaxVolume(double v){
            this->maxVol.maxOrMin = max(this->maxVol.maxOrMin, v);
        }

        void updateMinVolume(double v){
            this->minVol.maxOrMin = min(this->minVol.maxOrMin, v);
        }

        double DetermineATR(){
            return (this->sumATR)/ATRLookbackPeriod;
        }

        double DetermineMeanPrice(){
            return (this->sumPrice)/lookbackPeriod;
        }

        double DetermineMeanPricePrev(){
            return (this->sumPricePrev)/lookbackPeriod;
        }

        double DetermineMeanDiffPrice(){
            return (this->sumDiffPrice)/lookbackPeriod;
        }

        double DetermineMeanDiffPricePrev(){
            return (this->sumDiffPricePrev)/lookbackPeriod;
        }

        double DetermineMeanVolume(){
            return (this->sumVol)/lookbackPeriod;
        }

        double DetermineMeanVolumePrev(){
            return (this->sumVolPrev)/lookbackPeriod;
        }

        double DetermineSTDPrice(){
            double variance = (this->sumSQPrice - (this->sumPrice * this->sumPrice) / lookbackPeriod)/(lookbackPeriod - 1);
            double std = sqrt(variance);
            return std;
        }

        double DetermineProbGreaterDiffPrice(double val){
            double zScore = (val - this->DetermineMeanPrice())/this->DetermineSTDPrice();
            return (1.0 - 0.5 * (1.0 + erf(zScore / sqrt(2.0))));
        }

        double DetermineSTDPricePrev(){
            double variance = (this->sumSQPricePrev - (this->sumPricePrev * this->sumPricePrev) / 
            lookbackPeriod)/(lookbackPeriod - 1);
            double std = sqrt(variance);
            return std;
        }

        double DetermineSTDVolume(){
            double variance = (this->sumSQVol - (this->sumVol * this->sumVol) / lookbackPeriod)/(lookbackPeriod - 1);
            double std = sqrt(variance);
            return std;
        }

        double DetermineSTDVolumePrev(){
            double variance = (this->sumSQVolPrev - (this->sumVolPrev * this->sumVolPrev) / 
            lookbackPeriod)/(lookbackPeriod - 1);
            double std = sqrt(variance);
            return std;
        }
};

class TradingStrategy {
    private:
        bool containsOpenPosition;
        Position openPosition;
        vector<Position> closedPositions;
    public:
        double balance;
        TradingStrategy(): balance(-1), containsOpenPosition(false), openPosition(), closedPositions() {}
        TradingStrategy(double bal, bool cOP, Position pos, vector<Position> cPoses): balance(bal), containsOpenPosition(cOP),
        openPosition(pos), closedPositions(cPoses) {}
        virtual void ExecuteStrategy(const StockData &data) = 0;

        bool getContainsOpenPosition(){
            return this->containsOpenPosition;
        }

        void setContainsOpenPosition(bool cOP){
            this->containsOpenPosition = cOP;
        }

        Position getOpenPosition(){
            return this->openPosition;
        }

        void setOpenPosition(Position &p){
            this->openPosition = p;
        }

        vector<Position> getClosedPositions(){
            return this->closedPositions;
        }

        Position getClosedPosition(int index){
            if (index < 0 || index >= this->closedPositions.size()){
                return Position();
            }
            return this->closedPositions[index];
        }

        void appendClosedPosition(Position p){
            this->closedPositions.push_back(p);
        }

        void setClosedPositions(vector<Position> cP){
            this->closedPositions = cP;
        }

        vector<double> getAllReturns(){
            int n = this->closedPositions.size();
            vector<double> profits(n, 0);
            for (int i = 0; i < n; i++){
                if (this->closedPositions[i].getPositionType() == LONG){
                    profits[i] = (this->closedPositions[i].getSellPrice() - this->closedPositions[i].getPurchasePrice()) *
                                this->closedPositions[i].getNumShares();
                }
                else if (this->closedPositions[i].getPositionType() == SHORT){
                    profits[i] = (this->closedPositions[i].getPurchasePrice() - this->closedPositions[i].getSellPrice()) *
                                this->closedPositions[i].getNumShares();
                }
            }
            return profits;
        }

        map<int, vector<double>> getYearlyReturns(){
            int n = this->closedPositions.size();
            map<int, vector<double>> returns;
            for (int i = 0; i < n; i++){
                int year = stoi(this->closedPositions[i].getSellDate().substr(0, 4));
                double profit = 0;
                if (this->closedPositions[i].getPositionType() == LONG){
                    profit = (this->closedPositions[i].getSellPrice() - this->closedPositions[i].getPurchasePrice()) *
                                this->closedPositions[i].getNumShares();
                }
                else if (this->closedPositions[i].getPositionType() == SHORT){
                    profit = (this->closedPositions[i].getPurchasePrice() - this->closedPositions[i].getSellPrice()) *
                                this->closedPositions[i].getNumShares();
                }
                if (returns.find(year) == returns.end()){
                    returns[year] = {profit};
                }
                else{
                    returns[year].push_back(profit);
                }
            }
            return returns;
        }

        virtual ~TradingStrategy() = default;
};