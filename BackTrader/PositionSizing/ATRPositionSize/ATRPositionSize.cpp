#include "ATRPositionSize.h"

ATRPositionSize::ATRPositionSize(double riskAmount, int ATRPeriod, double ATRMultiplier): BasePositionSize(riskAmount),
ATRPeriod(ATRPeriod), ATRMultiplier(ATRMultiplier), trueRangeSum(0.0), currentATR(0.0) {}

double ATRPositionSize::calculatePositionSize(double balance, PositionType position) const {
    if (this->currentATR <= 0 || position.isNull()){
        return 0.0;
    }

    double dollarRisk = balance * this->riskAmount;
    double stopLossPrice = 0.0;
    double riskPerShare = 0.0;
    if (position.getPositiontype() == "LONG"){
        stopLossPrice = 
    }
}