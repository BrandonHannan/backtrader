#include "./DowContext.h"

DowContext::DowContext(int lookBackPeriod, int doubleLookBackPeriod, TrendMode trendMode, TrendLineMode trendLineMode): 
    BaseContext(lookBackPeriod), 
    doubleLookBackPeriod(doubleLookBackPeriod), 
    trendMode(trendMode), 
    trendLineMode(trendLineMode),
    trend(TrendIdentifier(lookBackPeriod, trendMode)), 
    doubleTrend(TrendIdentifier(doubleLookBackPeriod, trendMode)),
    trendLine(TrendLineTracker(trendLineMode)), 
    doubleTrendLine(TrendLineTracker(trendLineMode)), 
    priceStatistics(lookBackPeriod), 
    volumeStatistics(lookBackPeriod) {}

void DowContext::updateContext(const StockDataInstance &currentData, const StockDataInstance &previousData){
    double currentClose = currentData.close;
    double previousClose = previousData.close;
    double currentVolume = currentData.volume;
    double previousVolume = previousData.volume;


    if (this->firstUpdate){
        priceStatistics.addDataPoint(previousClose);
        priceStatistics.addDataPoint(currentClose);
        volumeStatistics.addDataPoint(previousVolume);
        volumeStatistics.addDataPoint(currentVolume);

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
        priceStatistics.addDataPoint(currentClose);
        volumeStatistics.addDataPoint(currentVolume);

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
        return Trade();
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

    if (this->trend.isReady() && this->trendLine.isReady()){
        return true;
    }
    return false;
}

string DowContext::getStats() const {
    string stats = "";
    int maxLength = 35;

    double maxPrice = this->priceStatistics.getMax();
    double meanPrice = this->priceStatistics.getMean();
    double stdPrice = this->priceStatistics.getStd();
    double minPrice = this->priceStatistics.getMin();
    double slopePrice = this->priceStatistics.getSlope();
    double slopeSEPrice = this->priceStatistics.getSlopeSE();
    double slopeRSQPrice = this->priceStatistics.getSlopeRSQ();
    bool slopeSigPrice = this->priceStatistics.getSlopeSignificance();

    double maxVol = this->volumeStatistics.getMax();
    double meanVol = this->volumeStatistics.getMean();
    double stdVol = this->volumeStatistics.getStd();
    double minVol = this->volumeStatistics.getMin();
    double slopeVolume = this->volumeStatistics.getSlope();
    double slopeSEVolume = this->volumeStatistics.getSlopeSE();
    double slopeRSQVolume = this->volumeStatistics.getSlopeRSQ();
    bool slopeSigVolume = this->volumeStatistics.getSlopeSignificance();

    stats.append(formatDoubleStat("Mean Price", meanPrice, maxLength));
    stats.append(formatDoubleStat("STD Price", stdPrice, maxLength));
    stats.append(formatDoubleStat("Min Price", minPrice, maxLength));
    stats.append(formatDoubleStat("Max Price", maxPrice, maxLength));
    stats.append(formatDoubleStat("Slope Price", slopePrice, maxLength));
    stats.append(formatDoubleStat("Slope Standard Error Price", slopeSEPrice, maxLength));
    stats.append(formatDoubleStat("Slope R^2 Price", slopeRSQPrice, maxLength));
    stats.append(formatBoolStat("Is Price Slope Significant", slopeSigPrice, maxLength));

    stats.append(formatDoubleStat("Mean Volume", meanVol, maxLength));
    stats.append(formatDoubleStat("STD Volume", stdVol, maxLength));
    stats.append(formatDoubleStat("Min Volume", minVol, maxLength));
    stats.append(formatDoubleStat("Max Volume", maxVol, maxLength));
    stats.append(formatDoubleStat("Slope Volume", slopeVolume, maxLength));
    stats.append(formatDoubleStat("Slope Standard Error Volume", slopeSEVolume, maxLength));
    stats.append(formatDoubleStat("Slope R^2 Volume", slopeRSQVolume, maxLength));
    stats.append(formatBoolStat("Is Volume Slope Significant", slopeSigVolume, maxLength));

    return stats;
}

void DowContext::clear(){
    this->clearBase();
    priceStatistics = WindowStatistics(this->lookBackPeriod);
    volumeStatistics = WindowStatistics(this->lookBackPeriod);

    this->trend = TrendIdentifier(this->lookBackPeriod, this->trendMode);
    this->doubleTrend = TrendIdentifier(this->doubleLookBackPeriod, this->trendMode);
    this->trendLine = TrendLineTracker(this->trendLineMode);
    this->doubleTrendLine = TrendLineTracker(this->trendLineMode);
}

