#ifndef TREND_H
#define TREND_H

#include "../Extremum/Extremum.h"

enum class TrendMode { FIVE_POINT, THREE_POINT };

enum class TrendType { NONE, UPTREND, DOWNTREND };

// Structure to hold our confirmed trend pattern
struct Trend {
    TrendType type = TrendType::NONE;
    Extremum e1, e2, e3, e4, e5; // e5 is the most recent (T3 for uptrend, P3 for downtrend)
};

#endif