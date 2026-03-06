#include "./TrendIdentifier.h"

TrendIdentifier::TrendIdentifier(int lookBackPeriod, TrendMode mode)
    : lookBackPeriod(lookBackPeriod), currentTrend(Trend()), mode(mode) {}

void TrendIdentifier::processNextDay(StockDataInstance data){
    Trend result;

    // 1. STATE MACHINE: Identify extrema on the fly
    if (currentDay == 0) {
        lastData = data;
    } else if (dir == Direction::UNKNOWN) {
        if (data.close > lastData.close) {
            dir = Direction::UP;
            extrema.push_back({currentDay - 1, lastData, true}); // The start was a trough
        } else if (data.close < lastData.close) {
            dir = Direction::DOWN;
            extrema.push_back({currentDay - 1, lastData, false}); // The start was a peak
        }
    } else if (dir == Direction::UP) {
        if (data.close < lastData.close) {
            extrema.push_back({currentDay - 1, lastData, false}); // Peak confirmed
            dir = Direction::DOWN;
        }
    } else if (dir == Direction::DOWN) {
        if (data.close > lastData.close) {
            extrema.push_back({currentDay - 1, lastData, true}); // Trough confirmed
            dir = Direction::UP;
        }
    }

    lastData = data;
    currentDay++;

    // 2. WINDOW MANAGEMENT: Remove extrema that are older than our lookback period
    while (!extrema.empty() && extrema.front().index <= (currentDay - 1) - static_cast<int>(this->lookBackPeriod)) {
        extrema.pop_front();
    }

    // 3. CHECK FOR TRENDS BASED ON SELECTED MODE
    
    // --- MODE 1: STRICT 5-POINT TREND (3 Troughs, 2 Peaks or vice versa) ---
    if (this->mode == TrendMode::FIVE_POINT && extrema.size() >= 5) {
        int i = extrema.size() - 5; 
        
        Extremum e1 = extrema[i], e2 = extrema[i+1], e3 = extrema[i+2], e4 = extrema[i+3], e5 = extrema[i+4];

        // UPTREND: T1 -> P1 -> T2 -> P2 -> T3
        if (e1.isTrough && !e2.isTrough && e3.isTrough && !e4.isTrough && e5.isTrough) {
            double t1 = e1.data.close, p1 = e2.data.close;
            double t2 = e3.data.close, p2 = e4.data.close;
            double t3 = e5.data.close;

            if (t3 > t2 && t2 > t1 && p2 > p1) {
                this->currentTrend = {TrendType::UPTREND, e1, e2, e3, e4, e5};
                return;
            }
        }

        // DOWNTREND: P1 -> T1 -> P2 -> T2 -> P3
        if (!e1.isTrough && e2.isTrough && !e3.isTrough && e4.isTrough && !e5.isTrough) {
            double p1 = e1.data.close, t1 = e2.data.close;
            double p2 = e3.data.close, t2 = e4.data.close;
            double p3 = e5.data.close;

            if (p3 < p2 && p2 < p1 && t2 < t1) {
                this->currentTrend = {TrendType::DOWNTREND, e1, e2, e3, e4, e5};
                return;
            }
        }
    }
    
    // --- MODE 2: LENIENT 3-POINT TREND (2 Troughs, 1 Peak or vice versa) ---
    else if (this->mode == TrendMode::THREE_POINT && extrema.size() >= 3) {
        int i = extrema.size() - 3; 
        
        Extremum e1 = extrema[i], e2 = extrema[i+1], e3 = extrema[i+2];

        // UPTREND: T1 -> P1 -> T2
        if (e1.isTrough && !e2.isTrough && e3.isTrough) {
            double t1 = e1.data.close;
            double p1 = e2.data.close; // P1 is captured but we don't have P2 to compare it to yet
            double t2 = e3.data.close;

            // CONFIRMATION: T2 must be > T1, AND current price must break above P1
            if (t2 > t1 && data.close > p1) { 
                this->currentTrend = {TrendType::UPTREND, e1, e2, e3};
                return;
            }
        }

        // DOWNTREND: P1 -> T1 -> P2
        if (!e1.isTrough && e2.isTrough && !e3.isTrough) {
            double p1 = e1.data.close;
            double t1 = e2.data.close; 
            double p2 = e3.data.close;

            // CONFIRMATION: P2 must be < P1, AND current price must break below T1
            if (p2 < p1 && data.close < t1) { 
                this->currentTrend = {TrendType::DOWNTREND, e1, e2, e3};
                return;
            }
        }
    }

    this->currentTrend.type = TrendType::NONE;
}

Trend TrendIdentifier::getCurrentTrend() {
    return this->currentTrend;
}
