#include "BaseStrategy.h"

BaseStrategy::BaseStrategy(double bal, unique_ptr<BasePositionSize> sizer): balance(bal), positionSizer(move(sizer)), closedPositions({}) {}

double BaseStrategy::getBalance(){
    return this->balance;
}

void BaseStrategy::addToBalance(double val){
    this->balance = this->balance + val;
}

BasePositionSize* BaseStrategy::getPositionSizer() {
    return this->positionSizer.get();
}

void BaseStrategy::setPositionSizer(unique_ptr<BasePositionSize> sizer){
    this->positionSizer = move(sizer);
}

Position BaseStrategy::getPosition(){
    return this->position;
}

void BaseStrategy::setPosition(Position &p){
    this->position = p;
}

vector<Position> BaseStrategy::getClosedPositions(){
    return this->closedPositions;
}

Position BaseStrategy::getClosedPosition(int index){
    if (index < 0 || index >= this->closedPositions.size()){
        return Position();
    }
    else{
        return this->closedPositions[index];
    }
}

void BaseStrategy::appendClosedPosition(Position p){
    this->closedPositions.push_back(p);
}

void BaseStrategy::setClosedPositions(vector<Position> cP){
    this->closedPositions = cP;
}

vector<double> BaseStrategy::getAllReturns(){
    vector<double> results;
    for (int i = 0; i<this->closedPositions.size(); i++){
        double pPrice = this->closedPositions[i].getPurchasePrice();
        double sPrice = this->closedPositions[i].getSellPrice();
        results.push_back(sPrice - pPrice);
    }
    return results;
}

map<int, vector<double>> BaseStrategy::getYearlyReturns(){
    int n = this->closedPositions.size();
    map<int, vector<double>> returns;
    for (int i = 0; i < n; i++){
        int year = stoi(this->closedPositions[i].getSellDate().substr(0, 4));
        double profit = 0;
        if (this->closedPositions[i].getPositionType().getPositiontype() == "LONG"){
            profit = (this->closedPositions[i].getSellPrice() - this->closedPositions[i].getPurchasePrice()) *
                        this->closedPositions[i].getNumShares();
        }
        else if (this->closedPositions[i].getPositionType().getPositiontype() == "SHORT"){
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