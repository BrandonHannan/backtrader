#include "WindowStatistics.h"

WindowStatistics::WindowStatistics(int windowSize): windowSize(windowSize), sum(0), sumSQ(0), indexSum(0), sumY(0), sumXY(0), indexSumSQ(0) {
    // LookbackPeriod must be >= 2
    for (int i = 0; i<windowSize; i++){
        double x = double(i);
        this->indexSum += x;
        this->indexSumSQ += x * x;
    }
}

void WindowStatistics::addDataPoint(double value){
    this->dataPoints.push(value);
    this->sum += value;
    this->sumSQ += value * value;
    this->sumY += value;
    this->sumXY += value * (this->dataPoints.size() - 1);

    if (this->dataPoints.size() > this->windowSize){
        double frontValue = this->dataPoints.front();
        this->sum -= frontValue;
        this->sumSQ -= frontValue * frontValue;
        this->sumY -= frontValue;
        this->sumXY
        this->dataPoints.pop();
    }

    if (this->dataPoints.size() == this->windowSize){
        double meanValue = this->sum / this->windowSize;
        double varValue = (this->sumSQ / this->windowSize) - (meanValue * meanValue);
        double sqrtValue = sqrt(varValue);
        this->mean = make_unique<double>(meanValue);
        this->std = make_unique<double>(sqrtValue);


    }
}