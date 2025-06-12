#include "../Position/Position.h"
#include <vector>
#include <algorithm>
#include <numeric>
#include <limits>
#include <boost/math/distributions/students_t.hpp>

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

        double StandardDeviation(){
            int n = this->closedPositions.size();
            if (n < 2.0){
                return 0;
            }

            vector<double> returns = DetermineProfits(this->closedPositions);
            double mean = accumulate(returns.begin(), returns.end(), 0.0) / n;

            double sumSq = 0.0;
            for (double val : returns) {
                sumSq += (val - mean) * (val - mean);
            }

            double variance = sumSq / (n - 1);
            return sqrt(variance);
        }

        double Mean(){
            if (this->closedPositions.empty()){
                return 0;
            }
            vector<double> returns = DetermineProfits(this->closedPositions);
            return accumulate(returns.begin(), returns.end(), 0.0) / this->closedPositions.size();
        }

        double CalculatePValue(){
            int n = this->closedPositions.size();
            if (n < 2){
                return 1;
            }
            vector<double> returns = DetermineProfits(this->closedPositions);
            double mean = accumulate(returns.begin(), returns.end(), 0.0) / n;
            double sumSq = 0.0;
            for (double val : returns) {
                sumSq += (val - mean) * (val - mean);
            }
            double std = sqrt(sumSq / (n - 1));
            if (std == 0) { return (mean > 0) ? 0.0 : 1.0; }

            double standard_error = std_dev / sqrt(n);
            double t_statistic = (mean - 0) / standard_error;
            int df = n - 1;

            boost::math::students_t dist(df);
            double p_value = boost::math::cdf(boost::math::complement(dist, t_statistic));
            return p_value;
        }

        virtual ~TradingStrategy() = default;
};