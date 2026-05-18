#include "BaseStrategy.h"

BaseStrategy::BaseStrategy(double bal, unique_ptr<BasePositionSize> sizer, unique_ptr<BaseContext> context): balance(bal), positionSizer(move(sizer)), strategyContext(move(context)), closedPositions({}), position(Position()) {}

double BaseStrategy::getBalance(){
    return this->balance;
}

void BaseStrategy::addToBalance(double val){
    this->balance = this->balance + val;
}

void BaseStrategy::setBalance(double val){
    this->balance = val;
}

BasePositionSize* BaseStrategy::getPositionSizer() {
    return this->positionSizer.get();
}

void BaseStrategy::setPositionSizer(unique_ptr<BasePositionSize> sizer){
    this->positionSizer = move(sizer);
}

BaseContext* BaseStrategy::getContext() {
    return this->strategyContext.get();
}

void BaseStrategy::setContext(unique_ptr<BaseContext> context){
    this->strategyContext = move(context);
}

Position& BaseStrategy::getPosition(){
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
        if (this->closedPositions[i].getPositionType().getPositiontype() == "LONG") {
            results.push_back((sPrice - pPrice) * this->closedPositions[i].getNumShares() * this->closedPositions[i].getContractSize());
        } else {
            // For SHORT trades, selling lower than the purchase price is a profit
            results.push_back((pPrice - sPrice) * this->closedPositions[i].getNumShares() * this->closedPositions[i].getContractSize());
        }
    }
    return results;
}

map<int, vector<double>> BaseStrategy::getYearlyReturns(){
    int n = this->closedPositions.size();
    map<int, vector<double>> returns;

    auto addToYear = [&](int year, double amount) {
        returns[year].push_back(amount);
    };

    for (int i = 0; i < n; i++){
        const Position &pos = this->closedPositions[i];
        const string &pDate = pos.getPurchaseDate();
        const string &sDate = pos.getSellDate();
        if (pDate.size() < 10 || sDate.size() < 10) continue;

        int exitYear  = stoi(sDate.substr(0, 4));

        double totalProfit = 0;
        if (pos.getPositionType().getPositiontype() == "LONG"){
            totalProfit = (pos.getSellPrice() - pos.getPurchasePrice()) * pos.getNumShares() * pos.getContractSize();
        }
        else if (pos.getPositionType().getPositiontype() == "SHORT"){
            totalProfit = (pos.getPurchasePrice() - pos.getSellPrice()) * pos.getNumShares() * pos.getContractSize();
        }

        addToYear(exitYear, totalProfit);
    }
    return returns;
}

int binarySearchDate(const vector<string> &dates, const string &targetDate) {
    int left = 0;
    int right = dates.size() - 1;

    while (left <= right) {
        int mid = left + (right - left) / 2;

        if (dates[mid] == targetDate) {
            return mid;
        } else if (dates[mid] < targetDate) {
            left = mid + 1;
        } else {
            right = mid - 1;
        }
    }

    return -1; // Not found
}