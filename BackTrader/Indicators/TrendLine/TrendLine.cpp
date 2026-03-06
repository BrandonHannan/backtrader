#include "TrendLine.h"

TrendLine::TrendLine(TrendLineMode mode): mode(mode), activeTrendline() {}

int TrendLine::LengthOfTradeBetweenDates(string date1, string date2){
    if (date1 == "" || date2 == "" || date1.length() != 10 || date2.length() != 10){
        return -1;
    }
    int length = 0;
    int beginningYear = stoi(date1.substr(0, 4));
    int beginningMonth = stoi(date1.substr(5, 7));
    int beginningDay = stoi(date1.substr(8, 10));

    int endYear = stoi(date2.substr(0, 4));
    int endMonth = stoi(date2.substr(5, 7));
    int endDay = stoi(date2.substr(8, 10));

    length = ToJulian(endYear, endMonth, endDay) - ToJulian(beginningYear, beginningMonth, beginningDay);
    return length;
}

double TrendLine::calculateGradient(const Extremum& p1, const Extremum& p2){
    if (p1.data.date == p2.data.date) return 0.0;
    return (p2.data.close - p1.data.close) / static_cast<double>(this->LengthOfTradeBetweenDates(p1.data.date, p2.data.date));
}

void TrendLine::update(const Trend& currentTrend){
    this->activeTrendline.type = currentTrend.type;

    if (!this->activeTrendline.isActive || this->activeTrendline.type != currentTrend.type){
        if (currentTrend.type == TrendType::UPTREND) {
            this->activeTrendline.anchor = currentTrend.e1; // T1
            this->activeTrendline.currentPoint = currentTrend.e3; // T2
        }
        else if (currentTrend.type == TrendType::DOWNTREND){
            this->activeTrendline.anchor = currentTrend.e1; // P1
            this->activeTrendline.currentPoint = currentTrend.e3; // P2
        }

        this->activeTrendline.currentGradient = calculateGradient(this->activeTrendline.anchor, this->activeTrendline.currentPoint);
    }
    else{
        // Identify the newest extremum in the pattern
        // (e3 for 3-point mode, e5 for 5-point mode)
        Extremum latestExtremum = (currentTrend.e5.index != -1) ? currentTrend.e5 : currentTrend.e3;

        // Ensure we are looking at a new point we haven't processed yet
        int dateLength = this->LengthOfTradeBetweenDates(this->activeTrendline.currentPoint.data.date, latestExtremum.data.date);
        if (dateLength > 0) {

            // Only evaluate troughs for uptrends, and peaks for downtrends
            bool isValidExtremum = (this->activeTrendline.type == TrendType::UPTREND && latestExtremum.isTrough) ||
                                    (this->activeTrendline.type == TrendType::DOWNTREND && !latestExtremum.isTrough);

            if (isValidExtremum) {
                double newGradient = calculateGradient(this->activeTrendline.anchor, latestExtremum);

                // Update if the absolute gradient is strictly smaller (flatter slope)
                if (abs(newGradient) < abs(this->activeTrendline.currentGradient)) {
                    this->activeTrendline.currentPoint = latestExtremum;
                    this->activeTrendline.currentGradient = newGradient;
                }
            }
        }
    }
}

Trendline TrendLine::getActiveTrend() const {
    return this->activeTrendline;
}