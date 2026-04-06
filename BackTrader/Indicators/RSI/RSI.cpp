#include "./RSI.h"

RSI::RSI(int period)
    : period(period), dayCount(0), prevClose(0.0),
      avgGain(0.0), avgLoss(0.0), ready(false) {}

void RSI::processNextDay(double close) {
    seedCloses.push(close);
    dayCount++;

    if (dayCount < period) {
        // Still accumulating seed closes — not enough data for even one gain/loss pair
        return;
    }

    if (dayCount == period) {
        // Seed phase complete: compute initial avgGain and avgLoss over period-1 pairs
        double gainSum = 0.0;
        double lossSum = 0.0;
        double prev = seedCloses.front();
        seedCloses.pop();
        while (!seedCloses.empty()) {
            double cur = seedCloses.front();
            seedCloses.pop();
            double change = cur - prev;
            if (change > 0.0) gainSum += change;
            else              lossSum -= change;
            prev = cur;
        }
        avgGain = gainSum / (period - 1);
        avgLoss = lossSum / (period - 1);
        prevClose = close;
        ready = true;
        return;
    }

    // Wilder's smoothing
    double gain = close > prevClose ? close - prevClose : 0.0;
    double loss = close < prevClose ? prevClose - close : 0.0;
    avgGain = (avgGain * (period - 1) + gain) / period;
    avgLoss = (avgLoss * (period - 1) + loss) / period;
    prevClose = close;
}

double RSI::getRSI() const {
    if (avgLoss == 0.0) return 100.0;
    double rs = avgGain / avgLoss;
    return 100.0 - (100.0 / (1.0 + rs));
}

bool RSI::isReady() const {
    return ready;
}

void RSI::clear() {
    dayCount = 0;
    prevClose = 0.0;
    avgGain = 0.0;
    avgLoss = 0.0;
    ready = false;
    seedCloses = queue<double>();
}
