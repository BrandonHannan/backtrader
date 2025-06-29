#include <iostream>
#include "Position.h"

int Position::toJulian(int y, int m, int d) {
    if (m <= 2) {
        y -= 1;
        m += 12;
    }
    int A = y / 100;
    int B = 2 - A + A / 4;
    return int(365.25 * (y + 4716)) + int(30.6001 * (m + 1)) + d + B - 1524;
}

int Position::LengthOfTradeBetweenDates(){
    if (purchaseDate == "" || sellDate == "" || purchaseDate.length() != 10 || sellDate.length() != 10){
        return -1;
    }
    int length = 0;
    int beginningYear = stoi(purchaseDate.substr(0, 4));
    int beginningMonth = stoi(purchaseDate.substr(5, 7));
    int beginningDay = stoi(purchaseDate.substr(8, 10));

    int endYear = stoi(sellDate.substr(0, 4));
    int endMonth = stoi(sellDate.substr(5, 7));
    int endDay = stoi(sellDate.substr(8, 10));

    length = toJulian(endYear, endMonth, endDay) - toJulian(beginningYear, beginningMonth, beginningDay);
    return length;
}

Position::Position(): positionType(NONE), tradeType(NOTHING), purchaseDate(""), sellDate(""), purchasePrice(-1), 
stopLossPrice(-1), sellPrice(-1), numShares(0), isClosed(false) {} // Initialized numShares to 0 as a good practice

Position::Position(PositionType pType, TradeType tType, string pDate, string sDate, double pPrice, double sLP,
double sPrice, double nShares, bool isC): positionType(pType), tradeType(tType), purchaseDate(pDate), 
sellDate(sDate), purchasePrice(pPrice), stopLossPrice(sLP), sellPrice(sPrice), numShares(nShares), 
isClosed(isC) {}

// Scope all other member functions with Position::
PositionType Position::getPositionType(){
    return this->positionType;
}

void Position::setPositionType(PositionType pType){
    this->positionType = pType;
}

TradeType Position::getTradeType(){
    return this->tradeType;
}

void Position::setTradeType(TradeType tType){
    this->tradeType = tType;
}

string Position::getPurchaseDate(){
    return this->purchaseDate;
}

void Position::setPurchaseDate(string pDate){
    this->purchaseDate = pDate;
}

string Position::getSellDate(){
    return this->sellDate;
}

void Position::setSellDate(string sDate){
    this->sellDate = sDate;
}

double Position::getPurchasePrice(){
    return this->purchasePrice;
}

void Position::setPurchasePrice(double pPrice){
    this->purchasePrice = pPrice;
}

double Position::getStopLossPrice(){
    return this->stopLossPrice;
}

void Position::setStopLossPrice(double sLP){
    this->stopLossPrice = sLP;
}

double Position::getSellPrice(){
    return this->sellPrice;
}

void Position::setSellPrice(double sPrice){
    this->sellPrice = sPrice;
}

double Position::getNumShares(){
    return this->numShares;
}

void Position::setNumShares(double nShares){
    this->numShares = nShares;
}

bool Position::getIsClosed(){
    return this->isClosed;
}

void Position::setIsClosed(bool isC){
    this->isClosed = isC;
}

int Position::LengthOfTrade(){
    // Returns -1 if an invalid position
    return LengthOfTradeBetweenDates();
}

Position& Position::operator=(const Position &obj){
    if (this != &obj){
        this->setTradeType(obj.tradeType);
        this->setPositionType(obj.positionType);
        this->setNumShares(obj.numShares);
        this->setPurchaseDate(obj.purchaseDate);
        this->setSellDate(obj.sellDate);
        this->setPurchasePrice(obj.purchasePrice);
        this->setSellPrice(obj.sellPrice);
        this->setStopLossPrice(obj.stopLossPrice);
        this->setIsClosed(obj.isClosed);
    }
    return *this;
}
