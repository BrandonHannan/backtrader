#include "./DowContext.h"

DowContext::DowContext(int lookBackPeriod, int doubleLookBackPeriod, int signalLookBackPeriod, TrendMode trendMode, TrendLineMode trendLineMode): 
    BaseContext(lookBackPeriod), 
    doubleLookBackPeriod(doubleLookBackPeriod), 
    signalLookBackPeriod(signalLookBackPeriod),
    trendMode(trendMode), 
    trendLineMode(trendLineMode),
    trend(TrendIdentifier(lookBackPeriod, trendMode)), 
    doubleTrend(TrendIdentifier(doubleLookBackPeriod, trendMode)),
    trendLine(TrendLineTracker(trendLineMode)), 
    doubleTrendLine(TrendLineTracker(trendLineMode)), 
    sMACD(SMAMACD(lookBackPeriod, doubleLookBackPeriod, signalLookBackPeriod)) {}

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
    double macd = this->sMACD.getMACD();
    double signal = this->sMACD.getSignal();

    Trendline currentTrendLine = this->trendLine.getActiveTrend();
    Trendline currentDoubleTrendLine = this->doubleTrendLine.getActiveTrend();
    if (currentTrend.type == TrendType::NONE){
        return Trade();
    }
    
    if (currentTrend.type == TrendType::UPTREND){
        return Trade("LONG", "LONG BREAKTHROUGH");
        // if ((macd - signal) > 0){
        //     return Trade("LONG", "LONG BREAKTHROUGH");
        // }
    }

    return Trade();
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

    if (this->trend.isReady() && this->trendLine.isReady() /*&& this->doubleTrend.isReady() && this->doubleTrendLine.isReady()*/){
        // if (this->sMACD.isReady()){
        //     return true;
        // }
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

static json extremumToJson(const Extremum &e) {
    return {
        {"index",   e.index},
        {"date",    e.data.date},
        {"open",    e.data.open},
        {"close",   e.data.close},
        {"high",    e.data.high},
        {"low",     e.data.low},
        {"isTrough", e.isTrough}
    };
}

static json trendToJson(const Trend &t) {
    string typeStr;
    switch (t.type) {
        case TrendType::UPTREND:   typeStr = "UPTREND";   break;
        case TrendType::DOWNTREND: typeStr = "DOWNTREND"; break;
        default:                   typeStr = "NONE";      break;
    }
    return {
        {"type", typeStr},
        {"e1", extremumToJson(t.e1)},
        {"e2", extremumToJson(t.e2)},
        {"e3", extremumToJson(t.e3)},
        {"e4", extremumToJson(t.e4)},
        {"e5", extremumToJson(t.e5)}
    };
}

static json trendlineToJson(const Trendline &tl) {
    string typeStr;
    switch (tl.initialTrendType) {
        case TrendType::UPTREND:   typeStr = "UPTREND";   break;
        case TrendType::DOWNTREND: typeStr = "DOWNTREND"; break;
        default:                   typeStr = "NONE";      break;
    }
    return {
        {"isActive",         tl.isActive},
        {"slope",            tl.m},
        {"intercept",        tl.c},
        {"dateDifference",   tl.dateDifference},
        {"initialTrendType", typeStr},
        {"anchor",           extremumToJson(tl.anchor)},
        {"currentPoint",     extremumToJson(tl.currentPoint)}
    };
}

json DowContext::getContextData() const {
    json data;
    data["contextType"] = "DowContext";

    data["priceStatistics"] = {
        {"mean",            priceStatistics.getMean()},
        {"std",             priceStatistics.getStd()},
        {"min",             priceStatistics.getMin()},
        {"max",             priceStatistics.getMax()},
        {"slope",           priceStatistics.getSlope()},
        {"slopeSE",         priceStatistics.getSlopeSE()},
        {"slopeRSQ",        priceStatistics.getSlopeRSQ()},
        {"slopeSignificant", priceStatistics.getSlopeSignificance()}
    };
    data["volumeStatistics"] = {
        {"mean",            volumeStatistics.getMean()},
        {"std",             volumeStatistics.getStd()},
        {"min",             volumeStatistics.getMin()},
        {"max",             volumeStatistics.getMax()},
        {"slope",           volumeStatistics.getSlope()},
        {"slopeSE",         volumeStatistics.getSlopeSE()},
        {"slopeRSQ",        volumeStatistics.getSlopeRSQ()},
        {"slopeSignificant", volumeStatistics.getSlopeSignificance()}
    };

    data["macdReady"]  = sMACD.isReady();
    data["macd"]       = sMACD.isReady() ? sMACD.getMACD()   : 0.0;
    data["signal"]     = sMACD.isReady() ? sMACD.getSignal() : 0.0;

    data["trendReady"] = trend.isReady();
    data["trend"]      = trend.isReady() ? trendToJson(trend.getCurrentTrend()) : json::object();

    data["doubleTrendReady"] = doubleTrend.isReady();
    data["doubleTrend"]      = doubleTrend.isReady() ? trendToJson(doubleTrend.getCurrentTrend()) : json::object();

    data["trendLineReady"] = trendLine.isReady();
    data["trendLine"]      = trendLine.isReady() ? trendlineToJson(trendLine.getActiveTrend()) : json::object();

    data["doubleTrendLineReady"] = doubleTrendLine.isReady();
    data["doubleTrendLine"]      = doubleTrendLine.isReady() ? trendlineToJson(doubleTrendLine.getActiveTrend()) : json::object();

    return data;
}

void DowContext::clear(){
    this->clearBase();
    this->trend = TrendIdentifier(this->lookBackPeriod, this->trendMode);
    this->doubleTrend = TrendIdentifier(this->doubleLookBackPeriod, this->trendMode);
    this->trendLine = TrendLineTracker(this->trendLineMode);
    this->doubleTrendLine = TrendLineTracker(this->trendLineMode);
    this->sMACD = SMAMACD(this->lookBackPeriod, this->doubleLookBackPeriod, this->signalLookBackPeriod);
}

