#include "Position.h"

Position::Position(): isClosed(true) {}

Position::Position(string stockName, string pType, string tType, string pDate, string sDate, double pPrice, double sPrice, double nShares, double sLPrice):
                stockName(stockName), positionType(pType), tradeType(tType), purchaseDate(pDate), sellDate(sDate), purchasePrice(pPrice), 
                sellPrice(sPrice), numShares(nShares), stopLossPrice(sLPrice), isClosed(false) {}


// Helper function to convert dates to a Julian Date number 
int Position::toJulian(int y, int m, int d) const {
    if (m <= 2) {
        y -= 1;
        m += 12;
    }
    int A = y / 100;
    int B = 2 - A + A / 4;
    return int(365.25 * (y + 4716)) + int(30.6001 * (m + 1)) + d + B - 1524;
}

// Helper function to determine the number of days between the purhcase date and the sell date
int Position::LengthOfTradeBetweenDates() const {
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

string Position::getStockName() const {
    return this->stockName;
}

void Position::setStockName(string stockName){
    this->stockName = stockName;
}

// Returns whether the trade is a SHORT or a LONG
PositionType Position::getPositionType() const {
    return this->positionType;
}

void Position::setPositionType(PositionType pType){
    this->positionType = pType;
}

void Position::setPositionType(string pType){
    this->positionType = PositionType(pType);
}

// Returns the trade type
string Position::getTradeType() const {
    return this->tradeType;
}

void Position::setTradeType(string tType){
    this->tradeType = tType;
}

string Position::getPurchaseDate() const {
    return this->purchaseDate;
}

void Position::setPurchaseDate(string pDate){
    this->purchaseDate = pDate;
}

string Position::getSellDate() const {
    return this->sellDate;
}

void Position::setSellDate(string sDate){
    this->sellDate = sDate;
}

double Position::getPurchasePrice() const {
    return this->purchasePrice;
}

void Position::setPurchasePrice(double pPrice){
    this->purchasePrice = pPrice;
}

double Position::getStopLossPrice() const {
    return this->stopLossPrice;
}

void Position::setStopLossPrice(double sLP){
    this->stopLossPrice = sLP;
}

double Position::getSellPrice() const {
    return this->sellPrice;
}

void Position::setSellPrice(double sPrice){
    this->sellPrice = sPrice;
}

double Position::getNumShares() const {
    return this->numShares;
}

void Position::setNumShares(double nShares){
    this->numShares = nShares;
}

string Position::getStats() const {
    return this->stats;
}

void Position::setStats(string stats){
    this->stats = stats;
}

bool Position::getIsClosed() const {
    return this->isClosed;
}

void Position::setIsClosed(bool isC){
    this->isClosed = isC;
}

int Position::LengthOfTrade() const {
    // Returns -1 if an invalid position
    return LengthOfTradeBetweenDates();
}

string Position::getBasePositionInfo() const {
    string result;
    result.append(format("     Trade Type: {}     Position Type: {}           Stock Name: {}\n", this->tradeType, this->positionType.getPositiontype(), this->stockName));
    result.append(format(" Purchase Price: {}     Purchase Date: {}     Number of Shares: {}\n", format("{:.2f}", this->purchasePrice), this->purchaseDate, format("{:.2f}", this->numShares)));
    result.append(format("Stop Loss Price: {}\n", format("{:.2f}", this->stopLossPrice)));
    result.append(format("     Sell Price: {}         Sell Date: {}      Length of Trade: {}\n", format("{:.2f}", this->sellPrice), this->sellDate, format("{:.2f}", LengthOfTrade())));
    return result;
}

Position& Position::operator=(const Position &obj){
    if (this != &obj){
        this->setStockName(obj.stockName);
        this->setTradeType(obj.tradeType);
        this->setPositionType(obj.positionType);
        this->setNumShares(obj.numShares);
        this->setPurchaseDate(obj.purchaseDate);
        this->setSellDate(obj.sellDate);
        this->setPurchasePrice(obj.purchasePrice);
        this->setSellPrice(obj.sellPrice);
        this->setStopLossPrice(obj.stopLossPrice);
        this->setStats(obj.stats);
        this->setIsClosed(obj.isClosed);
    }
    return *this;
}