#ifndef GENERICINPUTPARAMETERS_H
#define GENERICINPUTPARAMETERS_H

#include <vector>

using namespace std;

class GenericInputParameters{
    public:
        vector<int> lookbackPeriodArray = {5, 10, 15, 20, 25, 30, 35, 40, 45, 50, 55, 60, 65, 70, 75, 80, 85, 90, 95, 100};
        vector<int> ATRPeriodArray = {5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 25, 30, 35, 40, 45, 50, 55, 60};
        vector<double> ATRMultiplierArray = {0.25, 0.5, 0.75, 1, 1.5, 2.0, 2.5, 3.0, 3.5, 4.0};
        vector<double> RiskAmountArray = {0.01, 0.015, 0.02, 0.025, 0.03, 0.035, 0.04, 0.045, 0.05};
        vector<double> percentageArray = {0.1, 0.15, 0.2, 0.25, 0.3, 0.35, 0.4, 0.45, 0.5, 0.55, 0.6, 0.65, 0.7, 0.75, 0.8, 0.85, 0.9, 0.95, 0.99};

        virtual ~GenericInputParameters() = default;
};

#endif