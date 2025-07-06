#include "TradingStrategy.h"

using namespace std;

MaxMinPos::MaxMinPos() : maxOrMin(0), index(-1), date("") {}
MaxMinPos::MaxMinPos(double val, int idx, string d) : maxOrMin(val), index(idx), date(d) {}

LookBack::LookBack() : lookbackPeriod(0), ATRLookbackPeriod(0), sumATR(0), sumPrice(0), sumSQPrice(0), sumVol(0), sumSQVol(0),
                       sumDiffPrice(0), sumDiffPricePrev(0), sumPricePrev(0), sumSQPricePrev(0), sumVolPrev(0), sumSQVolPrev(0),
                       sumX(0), sumSQX(0), priceSlope(0), priceSlopePrev(0) {}

LookBack::LookBack(int lbP, int ATRlbP): lookbackPeriod(lbP), ATRLookbackPeriod(ATRlbP), maxPrice(-99999999),
    minPrice(99999999), maxVol(-99999999), minVol(99999999), sumATR(0), sumPrice(0), sumSQPrice(0), sumVol(0), 
    sumSQVol(0), sumDiffPrice(0), sumDiffPricePrev(0), sumPricePrev(0), sumSQPricePrev(0), sumVolPrev(0), 
    sumSQVolPrev(0), sumX(0), sumSQX(0), priceSlope(0), priceSlopePrev(0) {}

void LookBack::updateLookBackSumPrice(double currentPrice, double prevPrice, double prevPrevPrice){
    this->sumPricePrev = this->sumPricePrev - prevPrevPrice + prevPrice;
    this->sumSQPricePrev = this->sumSQPricePrev - prevPrevPrice * prevPrevPrice + prevPrice * prevPrice;
    this->sumPrice = this->sumPrice - prevPrice + currentPrice;
    this->sumSQPrice = this->sumSQPrice - prevPrice * prevPrice + currentPrice * currentPrice;
}

void LookBack::updateLookBackSumVolume(double currentVol, double prevVol, double prevPrevVol){
    this->sumVolPrev = this->sumVolPrev - prevPrevVol + prevVol;
    this->sumSQVolPrev = this->sumSQVolPrev - prevPrevVol * prevPrevVol + prevVol * prevVol;
    this->sumVol = this->sumVol - prevVol + currentVol;
    this->sumSQVol = this->sumSQVol - prevVol * prevVol + currentVol * currentVol;
}

void LookBack::updateLookBackSumDiffPrice(double currentDiffPrice, double prevDiffPrice, double prevPrevDiffPrice){
    this->sumDiffPrice = this->sumDiffPrice - prevDiffPrice + currentDiffPrice;
    this->sumDiffPricePrev = this->sumDiffPricePrev - prevPrevDiffPrice + prevDiffPrice;
}

void LookBack::updateATR(double maximum){
    this->sumATR += maximum;
    this->trueRangeWindow.push_back(maximum);
    if (this->trueRangeWindow.size() > (size_t)ATRLookbackPeriod){
        this->sumATR -= this->trueRangeWindow.front();
        this->trueRangeWindow.pop_front();
    }
}

void LookBack::updateMaxPrice(const vector<double> &data, int index){ 
    while (!this->priceMaxWindow.empty() && data[this->priceMaxWindow.back()] < data[index]){
        this->priceMaxWindow.pop_back();
    }
    this->priceMaxWindow.push_back(index);
    if (this->priceMaxWindow.front() <= index - this->lookbackPeriod){
        this->priceMaxWindow.pop_front();
    }
    this->maxPrice = data[this->priceMaxWindow.front()];
}
void LookBack::updateMinPrice(const vector<double> &data, int index){ 
    while (!this->priceMinWindow.empty() && data[this->priceMinWindow.back()] > data[index]){
        this->priceMinWindow.pop_back();
    }
    this->priceMinWindow.push_back(index);
    if (this->priceMinWindow.front() <= index - this->lookbackPeriod){
        this->priceMinWindow.pop_front();
    }
    this->minPrice = data[this->priceMinWindow.front()];
}
void LookBack::updateMaxVolume(const vector<double> &data, int index){ 
    while (!this->volumeMaxWindow.empty() && data[this->volumeMaxWindow.back()] < data[index]){
        this->volumeMaxWindow.pop_back();
    }
    this->volumeMaxWindow.push_back(index);
    if (this->volumeMaxWindow.front() <= index - this->lookbackPeriod){
        this->volumeMaxWindow.pop_front();
    }
    this->maxVol = data[this->volumeMaxWindow.front()];
}
void LookBack::updateMinVolume(const vector<double> &data, int index){ 
    while (!this->volumeMinWindow.empty() && data[this->volumeMinWindow.back()] > data[index]){
        this->volumeMinWindow.pop_back();
    }
    this->volumeMinWindow.push_back(index);
    if (this->volumeMinWindow.front() <= index - this->lookbackPeriod){
        this->volumeMinWindow.pop_front();
    }
    this->minVol = data[this->volumeMinWindow.front()]; 
}
double LookBack::DetermineATR(){ return (this->sumATR)/ATRLookbackPeriod; }
double LookBack::DetermineMeanPrice(){ return (this->sumPrice)/lookbackPeriod; }
double LookBack::DetermineMeanPricePrev(){ return (this->sumPricePrev)/lookbackPeriod; }
double LookBack::DetermineMeanDiffPrice(){ return (this->sumDiffPrice)/lookbackPeriod; }
double LookBack::DetermineMeanDiffPricePrev(){ return (this->sumDiffPricePrev)/lookbackPeriod; }
double LookBack::DetermineMeanVolume(){ return (this->sumVol)/lookbackPeriod; }
double LookBack::DetermineMeanVolumePrev(){ return (this->sumVolPrev)/lookbackPeriod; }

double LookBack::DetermineSTDPrice(){
    if (lookbackPeriod <= 1) return 0;
    double variance = (this->sumSQPrice - (this->sumPrice * this->sumPrice) / lookbackPeriod)/(lookbackPeriod - 1);
    return sqrt(variance);
}

double LookBack::DetermineProbGreaterDiffPrice(double val){
    double stdDev = DetermineSTDPrice();
    if (stdDev == 0) return 0.5; // Avoid division by zero
    double zScore = (val - this->DetermineMeanPrice())/stdDev;
    return 2 * (1.0 - 0.5 * (1.0 + erf(zScore / sqrt(2.0))));
}

double LookBack::DetermineSTDPricePrev(){
    if (lookbackPeriod <= 1) return 0;
    double variance = (this->sumSQPricePrev - (this->sumPricePrev * this->sumPricePrev) / lookbackPeriod)/(lookbackPeriod - 1);
    return sqrt(variance);
}

double LookBack::DetermineSTDVolume(){
    if (lookbackPeriod <= 1) return 0;
    double variance = (this->sumSQVol - (this->sumVol * this->sumVol) / lookbackPeriod)/(lookbackPeriod - 1);
    return sqrt(variance);
}

double LookBack::DetermineSTDVolumePrev(){
    if (lookbackPeriod <= 1) return 0;
    double variance = (this->sumSQVolPrev - (this->sumVolPrev * this->sumVolPrev) / lookbackPeriod)/(lookbackPeriod - 1);
    return sqrt(variance);
}

LookBack& LookBack::operator=(const LookBack &obj){
    if (this != &obj){
        this->lookbackPeriod = obj.lookbackPeriod;
        this->ATRLookbackPeriod = obj.ATRLookbackPeriod;
        this->maxPrice = obj.maxPrice;
        this->minPrice = obj.minPrice;
        this->maxVol = obj.maxVol;
        this->minVol = obj.minVol;
        this->priceMaxWindow = obj.priceMaxWindow;
        this->priceMinWindow = obj.priceMinWindow;
        this->volumeMaxWindow = obj.volumeMaxWindow;
        this->volumeMinWindow = obj.volumeMinWindow;
        this->sumATR = obj.sumATR;
        this->trueRangeWindow = obj.trueRangeWindow;
        this->sumPrice = obj.sumPrice;
        this->sumSQPrice = obj.sumSQPrice;
        this->sumVol = obj.sumVol;
        this->sumSQVol = obj.sumSQVol;
        this->sumDiffPrice = obj.sumDiffPrice;
        this->sumDiffPricePrev = obj.sumDiffPricePrev;
        this->sumPricePrev = obj.sumPricePrev;
        this->sumSQPricePrev = obj.sumSQPricePrev;
        this->sumVolPrev = obj.sumVolPrev;
        this->sumSQVolPrev = obj.sumSQVolPrev;
        this->sumX = obj.sumX;
        this->sumSQX = obj.sumSQX;
        this->sumXY = obj.sumXY;
        this->sumXYPrev = obj.sumXYPrev;
        this->priceSlope = obj.priceSlope;
        this->priceSlopePrev = obj.priceSlopePrev;
    }
    return *this;
}

TradingStrategy::TradingStrategy(): containsOpenPosition(false), openPosition(), closedPositions(), balance(-1) {}

// Corrected initialization order to match header declaration
TradingStrategy::TradingStrategy(double bal, bool cOP, Position pos, vector<Position> cPoses):
    containsOpenPosition(cOP),
    openPosition(pos),
    closedPositions(cPoses),
    balance(bal) {}

bool TradingStrategy::getContainsOpenPosition(){ return this->containsOpenPosition; }
void TradingStrategy::setContainsOpenPosition(bool cOP){ this->containsOpenPosition = cOP; }
Position TradingStrategy::getOpenPosition(){ return this->openPosition; }
void TradingStrategy::setOpenPosition(Position &p){ this->openPosition = p; }
vector<Position> TradingStrategy::getClosedPositions(){ return this->closedPositions; }

Position TradingStrategy::getClosedPosition(int index){
    if (index < 0 || index >= (int)this->closedPositions.size()){
        return Position();
    }
    return this->closedPositions[index];
}

void TradingStrategy::appendClosedPosition(Position p){ this->closedPositions.push_back(p); }
void TradingStrategy::setClosedPositions(vector<Position> cP){ this->closedPositions = cP; }

vector<double> TradingStrategy::getAllReturns(){
    int n = this->closedPositions.size();
    vector<double> profits(n, 0);
    for (int i = 0; i < n; i++){
        if (this->closedPositions[i].getPositionType() == LONG){
            profits[i] = (this->closedPositions[i].getSellPrice() - this->closedPositions[i].getPurchasePrice()) * this->closedPositions[i].getNumShares();
        }
        else if (this->closedPositions[i].getPositionType() == SHORT){
            profits[i] = (this->closedPositions[i].getPurchasePrice() - this->closedPositions[i].getSellPrice()) * this->closedPositions[i].getNumShares();
        }
    }
    return profits;
}

map<int, vector<double>> TradingStrategy::getYearlyReturns(){
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