#include "WindowStatistics.h"

WindowStatistics::WindowStatistics(int windowSize): windowSize(windowSize), sum(0), sumSQ(0), indexSum(0), sumXY(0), indexSumSQ(0), mean(NAN), std(NAN), slope(NAN), slopeSE(NAN), slopeRSQ(NAN), slopeTStatistic(NAN) {
    // LookbackPeriod must be >= 2
    for (int i = 0; i<windowSize; i++){
        double x = double(i);
        this->indexSum += x;
        this->indexSumSQ += double(x) * double(x);
    }
}

void WindowStatistics::updateVariables(){
    double meanValue = this->sum / this->windowSize;
    double varValue = (this->sumSQ / this->windowSize) - (meanValue * meanValue);
    double sqrtValue = sqrt(varValue);
    this->mean = meanValue;
    this->std = sqrtValue;
    double denominator = this->windowSize * this->indexSumSQ - this->indexSum * this->indexSum;
    if (abs(denominator) < this->EPS){
        return;
    }
    double slope = (this->windowSize * this->sumXY - this->indexSum * this->sum)/denominator;
    double intercept = (this->sum - slope * this->indexSum)/this->windowSize;
    this->slope = slope;
    double rss = this->sumSQ - (2.0 * slope * this->sumXY) 
                                - (2 * intercept * this->sum) 
                                + (slope * slope * this->indexSumSQ) 
                                + (2.0 * slope * intercept * this->indexSum) 
                                + (intercept * intercept * this->windowSize);
    if (rss < 0 && rss > -1e-12){
        rss = 0.0;
    }
    double tss = this->sumSQ - (this->sum * this->sum)/double(this->windowSize);
    if (tss < 0.0 && tss > -1e-12){
        tss = 0.0;
    }

    if (this->windowSize > 2){
        double sigma2 = rss / (this->windowSize - 2);

        double Sxx = this->indexSumSQ - (this->indexSum * this->indexSum)/double(this->windowSize);
        if (Sxx > 0) {
            double se_slope = sqrt(sigma2/Sxx);
            this->slopeSE = se_slope;
            double tStat = slope/se_slope;
            this->slopeTStatistic = tStat;
        }
    }

    if (tss <= 0.0){
        if (rss <= this->EPS){
            double r2 = 1.0;
            this->slopeRSQ = r2;
        }
    }
    else{
        double r2 = 1.0 - (rss/tss);
        if (r2 < -1.0) r2 = -1.0;
        if (r2 >  1.0) r2 =  1.0;
        this->slopeRSQ = r2;
    }
}

void WindowStatistics::addDataPoint(double value){
    this->sortedWindow.insert(value);

    if (this->dataPoints.size() < this->windowSize){
        this->dataPoints.push(value);
        this->sum += value;
        this->sumSQ += value * value;
        this->sumXY += value * (this->dataPoints.size() - 1);
    }
    else{
        double frontValue = this->dataPoints.front();

        this->sumXY = this->sumXY - (this->sum - frontValue) + value * (this->windowSize - 1);
        this->sum += value - frontValue;
        this->sumSQ += (value * value) - (frontValue * frontValue);

        auto it = this->sortedWindow.find(frontValue);
        if (it != this->sortedWindow.end()){
            this->sortedWindow.erase(it);
        }

        this->dataPoints.pop();
        this->dataPoints.push(value);
    }

    if (this->dataPoints.size() == this->windowSize){
        this->updateVariables();
    }
}

int WindowStatistics::getSize() const {
    return this->dataPoints.size();
}

double WindowStatistics::getMean() const {
    return this->mean;
}

double WindowStatistics::getStd() const {
    return this->std;
}

double WindowStatistics::getSlope() const {
    return this->slope;
}

double WindowStatistics::getSlopeSE() const {
    return this->slopeSE;
}

double WindowStatistics::getSlopeRSQ() const {
    return this->slopeRSQ;
}

bool WindowStatistics::getSlopeSignificance() const {
    if (this->slopeTStatistic < this->PValue){
        return true;
    }
    return false;
}

double WindowStatistics::getMax() const {
    if (this->sortedWindow.empty()){
        return 0.0;
    }
    return *this->sortedWindow.begin();
}

double WindowStatistics::getMin() const {
    if (this->sortedWindow.empty()){
        return 0.0;
    }
    return *this->sortedWindow.rbegin();
}

bool WindowStatistics::isReady() const {
    if (this->dataPoints.size() < this->windowSize){
        return false;
    }
    return true;
}