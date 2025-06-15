#include <iostream>
#include "Position.h"

class Position{
    private:
        PositionType positionType;
        string purchaseDate;
        string sellDate;
        double purchasePrice;
        double sellPrice;
        double numShares;
        bool isClosed;

        int toJulian(int y, int m, int d) {
            if (m <= 2) {
                y -= 1;
                m += 12;
            }
            int A = y / 100;
            int B = 2 - A + A / 4;
            return int(365.25 * (y + 4716)) + int(30.6001 * (m + 1)) + d + B - 1524;
        }

        int LengthOfTradeBetweenDates(){
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
    public:
        Position(): positionType(NONE), purchaseDate(""), sellDate(""), purchasePrice(-1), sellPrice(-1), isClosed(false) {}
        Position(PositionType pType, string pDate, string sDate, double pPrice, double sPrice, double nShares, bool isC): 
        positionType(pType), purchaseDate(pDate), sellDate(sDate), purchasePrice(pPrice), sellPrice(sPrice), numShares(nShares), 
        isClosed(isC) {}

        PositionType getPositionType(){
            return this->positionType;
        }

        void setPositionType(PositionType pType){
            this->positionType = pType;
        }

        string getPurchaseDate(){
            return this->purchaseDate;
        }

        void setPurchaseDate(string pDate){
            this->purchaseDate = pDate;
        }

        string getSellDate(){
            return this->sellDate;
        }

        void setSellDate(string sDate){
            this->sellDate = sDate;
        }

        double getPurchasePrice(){
            return this->purchasePrice;
        }

        void setPurchasePrice(double pPrice){
            this->purchasePrice = pPrice;
        }

        double getSellPrice(){
            return this->sellPrice;
        }

        void setSellPrice(double sPrice){
            this->sellPrice = sPrice;
        }

        double getNumShares(){
            return this->numShares;
        }

        void setNumShares(double nShares){
            this->numShares = nShares;
        }

        bool getIsClosed(){
            return this->isClosed;
        }

        void setIsClosed(bool isC){
            this->isClosed = isC;
        }

        int LengthOfTrade(){
            // Returns -1 if an invalid position
            return LengthOfTradeBetweenDates();
        }
};