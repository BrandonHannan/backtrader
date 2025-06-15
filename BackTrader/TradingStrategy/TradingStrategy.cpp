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
        virtual void ExecuteTrade(const StockData &data) = 0;

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