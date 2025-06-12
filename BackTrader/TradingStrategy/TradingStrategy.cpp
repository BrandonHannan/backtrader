#include "../Position/Position.h"
#include <vector>
#include <algorithm>
#include <numeric>
#include <limits>

vector<double> DetermineProfits(vector<Position> &positions){
    int n = positions.size();
    vector<double> profits(n, 0);
    for (int i = 0; i < n; i++){
        profits[i] = positions[i].getSellPrice() - positions[i].getPurchasePrice();
    }
    return profits;
}

class TradingStrategy {
    private:
        double balance;
        bool containsOpenPosition;
        Position openPosition;
        vector<Position> closedPositions;
    public:
        TradingStrategy(): balance(-1), containsOpenPosition(false), openPosition(), closedPositions() {}
        virtual bool shouldExecuteTrade(const vector<double> &data) = 0;

        double getBalance(){
            return this->balance;
        }

        void updateBalance(double update){
            this->balance = this->balance + update;
        }

        void setBalance(double bal){
            this->balance = bal;
        }

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

        virtual ~TradingStrategy() = default;
};