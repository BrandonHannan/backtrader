#include "WindowStatistics.h"

WindowStatistics::WindowStatistics(int windowSize): windowSize(windowSize), sum(0), sumSQ(0), indexSum(0), sumXY(0), indexSumSQ(0) {
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
    this->mean = make_unique<double>(meanValue);
    this->std = make_unique<double>(sqrtValue);
    double denominator = this->windowSize * this->indexSumSQ - this->indexSum * this->indexSum;
    if (abs(denominator) < this->EPS){
        return;
    }
    double slope = (this->windowSize * this->sumXY - this->indexSum * this->sum)/denominator;
    double intercept = (this->sum - slope * this->indexSum)/this->windowSize;
    this->slope = make_unique<double>(slope);
    double rss = this->sumSQ - (2.0 * slope * this->sumXY) 
                                - (2 * intercept * this->sum) 
                                + (slope * slope * this->indexSumSQ) 
                                + (2.0 * slope * intercept * this->indexSum) 
                                + (intercept * intercept * this->windowSize);
    if (rss < 0 && rss > -1e-12){
        rss = 0.0;
    }
    double tss = this->sumSQ - (this->sumSQ * this->sumSQ)/double(this->windowSize);
    if (tss < 0.0 && tss > -1e-12){
        tss = 0.0;
    }

    if (this->windowSize > 2){
        double sigma2 = rss / (this->windowSize - 2);

        double Sxx = this->indexSumSQ - (this->indexSum * this->indexSum)/double(this->windowSize);
        if (Sxx > 0) {
            double se_slope = sqrt(sigma2/Sxx);
            this->slopeSE = make_unique<double>(se_slope);
            double tStat = slope/se_slope;
            this->slopeTStatistic = make_unique<double>(tStat);
        }
    }

    if (tss <= 0.0){
        if (rss <= this->EPS){
            double r2 = 1.0;
            this->slopeRSQ = make_unique<double>(r2);
        }
    }
    else{
        double r2 = 1.0 - (rss/tss);
        if (r2 < -1.0) r2 = -1.0;
        if (r2 >  1.0) r2 =  1.0;
        this->slopeRSQ = make_unique<double>(r2);
    }
}

void WindowStatistics::addDataPoint(double value){
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

        this->dataPoints.pop();
        this->dataPoints.push(value);
    }

    if (this->dataPoints.size() == this->windowSize){
        this->updateVariables();
    }
}