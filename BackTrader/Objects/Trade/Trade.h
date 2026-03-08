#ifndef TRADE_H
#define TRADE_H

#include "../PositionType/PositionType.h"
#include <string>

struct Trade {
    PositionType positionType;
    string tradeType;
    bool isValid;

    Trade() : isValid(false) {}
    Trade(string positionType, string tradeType): positionType(positionType), tradeType(tradeType), isValid(true) {}
};

#endif