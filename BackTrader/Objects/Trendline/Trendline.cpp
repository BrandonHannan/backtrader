#include "Trendline.h"

Trendline::Trendline(): isActive(false), m(0), c(0), dateDifference(-1) {}

bool Trendline::isPointValidWithinTrendLine(const StockDataInstance &data, const TrendType &trendType){
    if (!this->isActive){
        return false;
    }

    int dateDifference = LengthOfTradeBetweenDates(this->anchor.data.date, data.date);
    double y = m * dateDifference + this->c;

    if (trendType == TrendType::UPTREND){
        return data.close >= y;
    }
    else if (trendType == TrendType::DOWNTREND){
        return data.close <= y;
    }

    return false;
}