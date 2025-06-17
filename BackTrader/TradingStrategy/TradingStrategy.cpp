#include "../StockData/StockData.h"
#include "../Position/Position.h"
#include <unordered_map>
#include <map>
#include <vector>
#include <algorithm>
#include <numeric>
#include <limits>

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
        MaxMinPos maxPrice;
        MaxMinPos minPrice;
        MaxMinPos maxVol;
        MaxMinPos minVol;

        double sumPrice;
        double sumSQPrice;
        double sumVol;
        double sumSQVol;

        double sumPricePrev;
        double sumSQPricePrev;
        double sumVolPrev;
        double sumSQVolPrev;

        LookBack() {}
        LookBack(int lbP): lookbackPeriod(lbP), maxPrice(MaxMinPos(-99999999, -1, "")), minPrice(MaxMinPos(99999999, -1, "")),
        maxVol(MaxMinPos(-99999999, -1, "")), minVol(MaxMinPos(99999999, -1, "")) {}

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

        double DetermineMeanPrice(){
            return (this->sumPrice)/lookbackPeriod;
        }

        double DetermineMeanPricePrev(){
            return (this->sumPricePrev)/lookbackPeriod;
        }

        double DetermineMeanVolume(){
            return (this->sumVol)/lookbackPeriod;
        }

        double DetermineMeanVolumePrev(){
            return (this->sumVolPrev)/lookbackPeriod;
        }

        double DetermineSTDPrice(){
            double mean = this->DetermineMeanPrice();
            double variance = (this->sumSQPrice - (this->sumPrice * this->sumPrice) / lookbackPeriod)/(lookbackPeriod - 1);
            double std = sqrt(variance);
            return std;
        }

        double DetermineSTDPricePrev(){
            double mean = this->DetermineMeanPricePrev();
            double variance = (this->sumSQPricePrev - (this->sumPricePrev * this->sumPricePrev) / 
            lookbackPeriod)/(lookbackPeriod - 1);
            double std = sqrt(variance);
            return std;
        }

        double DetermineSTDVolume(){
            double mean = this->DetermineMeanVolume();
            double variance = (this->sumSQVol - (this->sumVol * this->sumVol) / lookbackPeriod)/(lookbackPeriod - 1);
            double std = sqrt(variance);
            return std;
        }

        double DetermineSTDVolumePrev(){
            double mean = this->DetermineMeanVolumePrev();
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

        void setOpenPosition(Position p){
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
                profits[i] = this->closedPositions[i].getSellPrice() - this->closedPositions[i].getPurchasePrice();
            }
            return profits;
        }

        map<int, vector<double>> getYearlyReturns(){
            int n = this->closedPositions.size();
            map<int, vector<double>> returns;
            for (int i = 0; i < n; i++){
                int year = stoi(this->closedPositions[i].getSellDate().substr(0, 4));
                double profit = this->closedPositions[i].getSellPrice() - this->closedPositions[i].getPurchasePrice();
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