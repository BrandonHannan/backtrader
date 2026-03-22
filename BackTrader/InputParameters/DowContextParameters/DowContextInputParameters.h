#ifndef DOWCONTEXTINPUTPARAMETERS_H
#define DOWCONTEXTINPUTPARAMETERS_H

#include "../GenericInputParameters.h"
#include "../../Objects/Trendline/Trendline.h"
#include "../../Objects/Trend/Trend.h"
#include <vector>

using namespace std;

class DowContextInputParameters: public GenericInputParameters{
    public:
        vector<int> doubleLookbackPeriodArray = {5, 10, 15, 20, 25, 30, 35, 40, 45, 50, 55, 60, 65, 70, 75, 80, 85, 90, 95, 100};
        vector<int> signalLookBackPeriodArray = {5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 25, 30, 35, 40, 45, 50};
        vector<TrendMode> trendModes = {TrendMode::THREE_POINT, TrendMode::FIVE_POINT};
        vector<TrendLineMode> trendLineModes = {TrendLineMode::MINIMUM, TrendLineMode::MAXIMUM};
        vector<int> ATRPeriodArray = {5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 25, 30, 35, 40, 45, 50, 55, 60};
};

#endif