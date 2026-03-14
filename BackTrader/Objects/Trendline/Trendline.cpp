#include "Trendline.h"

Trendline::Trendline(): isActive(false), m(0), c(0), dateDifference(-1), initialTrendType(TrendType::NONE) {}

bool Trendline::isPointValidWithinTrendLine(const StockDataInstance &data){
    if (!this->isActive){
        return false;
    }

    int dateDifference = LengthOfTradeBetweenDates(this->anchor.data.date, data.date);
    double y = m * dateDifference + this->c;

    if (this->initialTrendType == TrendType::UPTREND){
        return data.close >= y;
    }
    else if (this->initialTrendType == TrendType::DOWNTREND){
        return data.close <= y;
    }

    return false;
}