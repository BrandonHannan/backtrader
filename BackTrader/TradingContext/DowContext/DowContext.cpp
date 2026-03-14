#include "./DowContext.h"

DowContext::DowContext(int lookBackPeriod, int doubleLookBackPeriod, TrendMode trendMode, TrendLineMode trendLineMode): 
BaseContext(lookBackPeriod), trend(TrendIdentifier(lookBackPeriod, trendMode)), doubleTrend(TrendIdentifier(doubleLookBackPeriod, trendMode)),
trendLine(trendLineMode), doubleTrendLine(trendLineMode) {}

void DowContext::updateContext(const StockDataInstance &currentData, const StockDataInstance &previousData){
    if (this->firstUpdate){
        this->trend.processNextDay(previousData);
        this->trend.processNextDay(currentData);
        this->doubleTrend.processNextDay(previousData);
        this->doubleTrend.processNextDay(currentData);

        Trend currentTrend = this->trend.getCurrentTrend();
        Trend currentDoubleTrend = this->doubleTrend.getCurrentTrend();

        Trendline currentTrendLine = this->trendLine.getActiveTrend();
        Trendline currentDoubleTrendLine = this->doubleTrendLine.getActiveTrend();

        if (currentTrend.type != TrendType::NONE || currentTrendLine.isActive){
            this->trendLine.update(currentTrend);
        }

        if (currentDoubleTrend.type != TrendType::NONE || currentDoubleTrendLine.isActive){
            this->doubleTrendLine.update(currentDoubleTrend);
        }
        this->firstUpdate = false;
    }
    else{
        this->trend.processNextDay(currentData);
        this->doubleTrend.processNextDay(currentData);

        Trend currentTrend = this->trend.getCurrentTrend();
        Trend currentDoubleTrend = this->doubleTrend.getCurrentTrend();

        Trendline currentTrendLine = this->trendLine.getActiveTrend();
        Trendline currentDoubleTrendLine = this->doubleTrendLine.getActiveTrend();

        if (currentTrend.type != TrendType::NONE || currentTrendLine.isActive){
            this->trendLine.update(currentTrend);
        }

        if (currentDoubleTrend.type != TrendType::NONE || currentDoubleTrendLine.isActive){
            this->doubleTrendLine.update(currentDoubleTrend);
        }
    }
    return;
}

Trade DowContext::shouldExecuteTrade(const StockDataInstance &currentData) const {
    Trend currentTrend = this->trend.getCurrentTrend();
    Trend currentDoubleTrend = this->doubleTrend.getCurrentTrend();

    Trendline currentTrendLine = this->trendLine.getActiveTrend();
    Trendline currentDoubleTrendLine = this->doubleTrendLine.getActiveTrend();
    if (currentTrend.type == TrendType::NONE){
        return;
    }
    else if (currentTrend.type == TrendType::UPTREND){
        return Trade("LONG", "LONG BREAKTHROUGH");
    }
    else{
        return Trade("SHORT", "SHORT BREAKTHROUGH");
    }
}

bool DowContext::shouldSellTrade(const Position &currentPosition, const StockDataInstance &currentData) const {
    bool shouldSell = this->checkStopLossPrice(currentPosition, currentData);

    // Sell Current Position if current price violates current trend line
    // Trendline currentTrendLine = this->trendLine.getActiveTrend();
    // Trendline currentDoubleTrendLine = this->doubleTrendLine.getActiveTrend();

    // if (currentTrendLine.isActive && !currentTrendLine.isPointValidWithinTrendLine(currentData)){
    //     shouldSell = true;
    // }

    return shouldSell;
}

bool DowContext::isValid() const {

    // Return true only once the double trend has been established
    // if (this->doubleTrend.isReady()){
    //     return true;
    // }

    if (this->trend.isReady()){
        return true;
    }
    return false;
}

