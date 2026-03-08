#include "TrendLineTracker.h"

TrendLineTracker::TrendLineTracker(TrendLineMode mode): mode(mode), activeTrendline() {}

double TrendLineTracker::calculateGradient(const Extremum& p1, const Extremum& p2) const {
    if (p1.data.date == p2.data.date) return 0.0;
    return (p2.data.close - p1.data.close) / static_cast<double>(LengthOfTradeBetweenDates(p1.data.date, p2.data.date));
}

void TrendLineTracker::updateActiveTrendline(const double &newGradient, const Extremum &newPoint){
    if (this->mode == TrendLineMode::MINIMUM){
        if (abs(newGradient) < this->activeTrendline.m){
            this->activeTrendline.m = newGradient;
            this->activeTrendline.c = newPoint.data.close;
            this->activeTrendline.currentPoint = newPoint;
            this->activeTrendline.dateDifference = LengthOfTradeBetweenDates(this->activeTrendline.anchor.data.date, newPoint.data.date);
        }
    }
    else{
        if (abs(newGradient) > this->activeTrendline.m){
            this->activeTrendline.m = newGradient;
            this->activeTrendline.c = newPoint.data.close;
            this->activeTrendline.currentPoint = newPoint;
            this->activeTrendline.dateDifference = LengthOfTradeBetweenDates(this->activeTrendline.anchor.data.date, newPoint.data.date);
        }
    }
}

void TrendLineTracker::update(const Trend& currentTrend){
    if (!this->activeTrendline.isActive){
        if (currentTrend.type == TrendType::UPTREND) {
            this->activeTrendline.anchor = currentTrend.e1; // T1
            this->activeTrendline.currentPoint = currentTrend.e3; // T2
        }
        else if (currentTrend.type == TrendType::DOWNTREND){
            this->activeTrendline.anchor = currentTrend.e1; // P1
            this->activeTrendline.currentPoint = currentTrend.e3; // P2
        }
        else{
            return;
        }

        this->activeTrendline.c = currentTrend.e1.data.close;
        this->activeTrendline.m = calculateGradient(this->activeTrendline.anchor, this->activeTrendline.currentPoint);
        this->activeTrendline.dateDifference = LengthOfTradeBetweenDates(currentTrend.e1.data.date, currentTrend.e3.data.date);

        if (currentTrend.e5.index != -1){
            double newGradient = calculateGradient(this->activeTrendline.anchor, currentTrend.e5);

            this->updateActiveTrendline(newGradient, currentTrend.e5);
        }
    }
    else{
        // Identify the newest extremum in the pattern
        Extremum latestExtremum = currentTrend.e1;
        double newGradient = 0;

        // Only evaluate troughs for uptrends, and peaks for downtrends
        bool isValidExtremum = (this->activeTrendline.m > 0 && latestExtremum.isTrough) || (this->activeTrendline.m < 0 && !latestExtremum.isTrough);
        isValidExtremum = isValidExtremum && latestExtremum.data.date != this->activeTrendline.anchor.data.date;
        isValidExtremum = isValidExtremum && latestExtremum.data.date != this->activeTrendline.currentPoint.data.date;
        isValidExtremum = LengthOfTradeBetweenDates(this->activeTrendline.anchor.data.date, latestExtremum.data.date) > this->activeTrendline.dateDifference;

        if (isValidExtremum){
            newGradient = calculateGradient(this->activeTrendline.anchor, latestExtremum);

            this->updateActiveTrendline(newGradient, latestExtremum);
        }

        latestExtremum = currentTrend.e2;

        isValidExtremum = (this->activeTrendline.m > 0 && latestExtremum.isTrough) || (this->activeTrendline.m < 0 && !latestExtremum.isTrough);
        isValidExtremum = isValidExtremum && latestExtremum.data.date != this->activeTrendline.anchor.data.date;
        isValidExtremum = isValidExtremum && latestExtremum.data.date != this->activeTrendline.currentPoint.data.date;
        isValidExtremum = LengthOfTradeBetweenDates(this->activeTrendline.anchor.data.date, latestExtremum.data.date) > this->activeTrendline.dateDifference;

        if (isValidExtremum){
            newGradient = calculateGradient(this->activeTrendline.anchor, latestExtremum);

            this->updateActiveTrendline(newGradient, latestExtremum);
        }

        latestExtremum = currentTrend.e3;

        isValidExtremum = (this->activeTrendline.m > 0 && latestExtremum.isTrough) || (this->activeTrendline.m < 0 && !latestExtremum.isTrough);
        isValidExtremum = isValidExtremum && latestExtremum.data.date != this->activeTrendline.anchor.data.date;
        isValidExtremum = isValidExtremum && latestExtremum.data.date != this->activeTrendline.currentPoint.data.date;
        isValidExtremum = LengthOfTradeBetweenDates(this->activeTrendline.anchor.data.date, latestExtremum.data.date) > this->activeTrendline.dateDifference;

        if (isValidExtremum){
            newGradient = calculateGradient(this->activeTrendline.anchor, latestExtremum);

            this->updateActiveTrendline(newGradient, latestExtremum);
        }

        latestExtremum = currentTrend.e4;

        isValidExtremum = (this->activeTrendline.m > 0 && latestExtremum.isTrough) || (this->activeTrendline.m < 0 && !latestExtremum.isTrough);
        isValidExtremum = isValidExtremum && latestExtremum.data.date != this->activeTrendline.anchor.data.date;
        isValidExtremum = isValidExtremum && latestExtremum.data.date != this->activeTrendline.currentPoint.data.date;
        isValidExtremum = LengthOfTradeBetweenDates(this->activeTrendline.anchor.data.date, latestExtremum.data.date) > this->activeTrendline.dateDifference;

        if (isValidExtremum){
            newGradient = calculateGradient(this->activeTrendline.anchor, latestExtremum);

            this->updateActiveTrendline(newGradient, latestExtremum);
        }

        latestExtremum = currentTrend.e5;

        isValidExtremum = (this->activeTrendline.m > 0 && latestExtremum.isTrough) || (this->activeTrendline.m < 0 && !latestExtremum.isTrough);
        isValidExtremum = isValidExtremum && latestExtremum.data.date != this->activeTrendline.anchor.data.date;
        isValidExtremum = isValidExtremum && latestExtremum.data.date != this->activeTrendline.currentPoint.data.date;
        isValidExtremum = LengthOfTradeBetweenDates(this->activeTrendline.anchor.data.date, latestExtremum.data.date) > this->activeTrendline.dateDifference;

        if (isValidExtremum){
            newGradient = calculateGradient(this->activeTrendline.anchor, latestExtremum);

            this->updateActiveTrendline(newGradient, latestExtremum);
        }
    }
}

Trendline TrendLineTracker::getActiveTrend() const {
    return this->activeTrendline;
}

void TrendLineTracker::clear(){
    this->activeTrendline = Trendline();
}