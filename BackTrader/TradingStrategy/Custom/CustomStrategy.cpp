#include "CustomStrategy.h"

CustomStrategy::CustomStrategy(double balance, unique_ptr<BasePositionSize> sizer, unique_ptr<BaseContext> context): BaseStrategy(balance, move(sizer), move(context)) {}


void CustomStrategy::ExecuteStrategy(const string &stockName, const StockData &data){

    auto* sizer = this->getPositionSizer();
    auto* context = this->getContext();
    int size = data.close.size();

    for (int i = 1; i<(size - 1); i++){
        double previousOpen = data.open[i - 1];
        double previousClose = data.close[i - 1];
        double previousLow = data.low[i - 1];
        double previousHigh = data.high[i - 1];
        string previousDate = data.date[i - 1];
        double previousVolume = data.volume[i - 1];

        double currentOpen = data.open[i];
        double currentClose = data.close[i];
        double currentLow = data.low[i];
        double currentHigh = data.high[i];
        string currentDate = data.date[i];
        double currentVolume = data.volume[i];

        double futureOpen = data.open[i + 1];
        double futureClose = data.close[i + 1];
        double futureLow = data.low[i + 1];
        double futureHigh = data.high[i + 1];
        string futureDate = data.date[i + 1];
        double futureVolume = data.volume[i + 1];

        bool hasSizerUpdated = false;

        double currentBalance = this->getBalance();

        StockDataInstance previousInstance(i - 1, previousOpen, previousClose, previousHigh, previousLow, previousVolume, previousDate, data.contractSize);
        StockDataInstance currentInstance(i, currentOpen, currentClose, currentHigh, currentLow, currentVolume, currentDate, data.contractSize);
        StockDataInstance futureInstance(i + 1, futureOpen, futureClose, futureHigh, futureLow, futureVolume, futureDate, data.contractSize);

        Position &currentPosition = this->getPosition();

        if (!currentPosition.getIsClosed()){
            bool shouldSell = context->shouldSellTrade(currentPosition, currentInstance);

            if (shouldSell || i == size - 2){
                double exitPrice = currentPosition.getExitPrice(currentInstance, futureInstance);

                bool isStopLoss = false;
                if (currentPosition.getPositionType().getPositiontype() == "LONG") {
                    if (currentLow <= currentPosition.getStopLossPrice()) isStopLoss = true;
                } else {
                    if (currentHigh >= currentPosition.getStopLossPrice()) isStopLoss = true;
                }

                if (isStopLoss) {
                    currentPosition.setSellDate(currentDate);
                } else {
                    currentPosition.setSellDate(futureDate);
                }

                currentPosition.setSellPrice(exitPrice);
                currentPosition.setIsClosed(true);
                currentPosition.setExitContextData(context->getContextData());

                string positionStats = currentPosition.getBasePositionInfo();
                string totalStats = combineSideBySide(currentPosition.getStats(), context->getStats());
                positionStats.append(totalStats);
                currentPosition.setStats(positionStats);

                this->appendClosedPosition(currentPosition);
                if (currentPosition.getPositionType().getPositiontype() == "LONG"){
                    this->addToBalance((currentPosition.getNumShares() * currentPosition.getSellPrice() * currentPosition.getContractSize()));
                }
                else if (currentPosition.getPositionType().getPositiontype() == "SHORT"){
                    this->addToBalance((currentPosition.getNumShares() * (currentPosition.getPurchasePrice() + (currentPosition.getPurchasePrice() - currentPosition.getSellPrice())) * currentPosition.getContractSize()));
                }
                
                Position emptyPosition = Position();
                this->setPosition(emptyPosition);
                context->onPositionSold();
            }
            else{
                sizer->updateStopLossPrice(currentPosition, currentInstance);
            }
        }

        if (currentPosition.getIsClosed() && currentBalance > 0 && i < size - 2){
            // Check if the position size and context instances are ready and full of data
            if (sizer->isValid() && context->isValid()){
                Trade trade = context->shouldExecuteTrade(currentInstance, *this->macroFeatures, stockName);
                // Trade trade = context->shouldExecuteTrade(currentInstance);
                if (trade.isValid){
                    sizer->processNewData(currentInstance, previousInstance);
                    hasSizerUpdated = true;
                    // Execute trade
                    Position newPosition = sizer->purchasePosition(currentBalance, stockName, trade.positionType, futureInstance);
                    newPosition.setTradeType(trade.tradeType);
                    string positionStats = context->getStats();
                    newPosition.setStats(positionStats);
                    nlohmann::json entryCtx = context->getContextData();
                    if (this->macroFeatures != nullptr) {
                        entryCtx["macroContext"] = this->macroFeatures->compute(stockName, currentDate);
                    }
                    newPosition.setEntryContextData(entryCtx);

                    this->addToBalance(-1 * (newPosition.getNumShares() * newPosition.getPurchasePrice() * newPosition.getContractSize()));
                    
                    this->setPosition(newPosition);
                }
            }
        }

        // Update CONTEXT with the current data points after the shouldExecuteTrade and shouldSellTrade functions are executed
        context->updateContext(currentInstance, previousInstance);
        if (!hasSizerUpdated) { sizer->processNewData(currentInstance, previousInstance); }
    }

    if (!this->getPosition().getIsClosed()){
        Position &lastPosition = this->getPosition();
        lastPosition.setSellPrice(data.close.back());
        lastPosition.setSellDate(data.date.back());
        lastPosition.setIsClosed(true);
        lastPosition.setExitContextData(context->getContextData());

        string positionStats = lastPosition.getBasePositionInfo();
        string totalStats = combineSideBySide(lastPosition.getStats(), context->getStats());
        positionStats.append(totalStats);
        lastPosition.setStats(positionStats);

        this->appendClosedPosition(lastPosition);
        if (lastPosition.getPositionType().getPositiontype() == "LONG"){
            this->addToBalance((lastPosition.getNumShares() * lastPosition.getSellPrice() * lastPosition.getContractSize()));
        }
        else if (lastPosition.getPositionType().getPositiontype() == "SHORT"){
            this->addToBalance((lastPosition.getNumShares() * (lastPosition.getPurchasePrice() + (lastPosition.getPurchasePrice() - lastPosition.getSellPrice())) * lastPosition.getContractSize()));
        }
        
        Position emptyPosition = Position();
        this->setPosition(emptyPosition);
        context->onPositionSold();
    }

    context->clear();
    sizer->clear();
}

void CustomStrategy::ExecuteStrategy(const string &stockName, const StockData &data, const StockData &minuteData){
    auto* sizer = this->getPositionSizer();
    auto* context = this->getContext();
    int size = data.close.size();

    string initialMinuteDate = minuteData.date[0];
    initialMinuteDate = initialMinuteDate.substr(0, 10);
    int dateIndex = binarySearchDate(data.date, initialMinuteDate);

    if (dateIndex == -1) {
        cerr << "[Warning] " << stockName << ": minute start date " << initialMinuteDate
             << " not in daily data - skipping ticker\n";
        return;
    }

    for (int i = max(dateIndex, 1); i<(size - 1); i++){
        double previousOpen = data.open[i - 1];
        double previousClose = data.close[i - 1];
        double previousLow = data.low[i - 1];
        double previousHigh = data.high[i - 1];
        string previousDate = data.date[i - 1];
        double previousVolume = data.volume[i - 1];

        double currentOpen = data.open[i];
        double currentClose = data.close[i];
        double currentLow = data.low[i];
        double currentHigh = data.high[i];
        string currentDate = data.date[i];
        double currentVolume = data.volume[i];

        double futureOpen = data.open[i + 1];
        double futureClose = data.close[i + 1];
        double futureLow = data.low[i + 1];
        double futureHigh = data.high[i + 1];
        string futureDate = data.date[i + 1];
        double futureVolume = data.volume[i + 1];

        bool hasSizerUpdated = false;

        double currentBalance = this->getBalance();

        StockDataInstance previousInstance(i - 1, previousOpen, previousClose, previousHigh, previousLow, previousVolume, previousDate, data.contractSize);
        StockDataInstance currentInstance(i, currentOpen, currentClose, currentHigh, currentLow, currentVolume, currentDate, data.contractSize);
        StockDataInstance futureInstance(i + 1, futureOpen, futureClose, futureHigh, futureLow, futureVolume, futureDate, data.contractSize);

        Position &currentPosition = this->getPosition();

        if (!currentPosition.getIsClosed()){
            bool shouldSell = context->shouldSellTrade(currentPosition, currentInstance);

            if (shouldSell || i == size - 2){
                double exitPrice = -1;
                // Dukascopy daily bars are UTC-calendar-day indexed (verified empirically:
                // see DownloadDataPython/verify_minute_convention.py and verification_report.txt
                // - Hypothesis A matched 100% of OHLC across LIGHT.CMD/USD, XAU/USD, COFFEE.CMD/USX).
                // Slice = minute bars whose timestamp is within [currentDate T00:00:00, currentDate T23:59:59].
                const string sessionStart = currentDate + "T00:00:00";
                const string sessionEnd   = currentDate + "T23:59:59";
                auto startIt = std::lower_bound(minuteData.date.begin(), minuteData.date.end(), sessionStart);
                auto endIt   = std::upper_bound(minuteData.date.begin(), minuteData.date.end(), sessionEnd);
                if (startIt == minuteData.date.end() || startIt >= endIt) {
                    cerr << "[Warning] " << stockName << " " << currentDate
                         << ": no minute bars in session window - falling back to daily exit price\n";
                    exitPrice = currentPosition.getExitPrice(currentInstance, futureInstance);
                }
                else{
                    int initialMinuteIndex = static_cast<int>(std::distance(minuteData.date.begin(), startIt));
                    int finalMinuteIndex   = static_cast<int>(std::distance(minuteData.date.begin(), endIt)) - 1;
                    vector<StockDataInstance> minuteSlice;
                    minuteSlice.reserve(finalMinuteIndex - initialMinuteIndex + 1);
                    for (int idx = initialMinuteIndex; idx <= finalMinuteIndex; idx++) {
                        minuteSlice.emplace_back(idx,
                            minuteData.open[idx], minuteData.close[idx],
                            minuteData.high[idx], minuteData.low[idx],
                            minuteData.volume[idx], minuteData.date[idx],
                            data.contractSize);
                    }
                    exitPrice = currentPosition.getExitPrice(currentInstance, futureInstance, minuteSlice);
                }

                bool isStopLoss = false;
                if (currentPosition.getPositionType().getPositiontype() == "LONG") {
                    if (currentLow <= currentPosition.getStopLossPrice()) isStopLoss = true;
                } else {
                    if (currentHigh >= currentPosition.getStopLossPrice()) isStopLoss = true;
                }

                if (isStopLoss) {
                    currentPosition.setSellDate(currentDate);
                } else {
                    currentPosition.setSellDate(futureDate);
                }

                currentPosition.setSellPrice(exitPrice);
                currentPosition.setIsClosed(true);
                currentPosition.setExitContextData(context->getContextData());

                string positionStats = currentPosition.getBasePositionInfo();
                string totalStats = combineSideBySide(currentPosition.getStats(), context->getStats());
                positionStats.append(totalStats);
                currentPosition.setStats(positionStats);

                this->appendClosedPosition(currentPosition);
                if (currentPosition.getPositionType().getPositiontype() == "LONG"){
                    this->addToBalance((currentPosition.getNumShares() * currentPosition.getSellPrice() * currentPosition.getContractSize()));
                }
                else if (currentPosition.getPositionType().getPositiontype() == "SHORT"){
                    this->addToBalance((currentPosition.getNumShares() * (currentPosition.getPurchasePrice() + (currentPosition.getPurchasePrice() - currentPosition.getSellPrice())) * currentPosition.getContractSize()));
                }

                Position emptyPosition = Position();
                this->setPosition(emptyPosition);
                context->onPositionSold();
            }
            else{
                sizer->updateStopLossPrice(currentPosition, currentInstance);
            }
        }

        if (currentPosition.getIsClosed() && currentBalance > 0 && i < size - 2){
            // Check if the position size and context instances are ready and full of data
            if (sizer->isValid() && context->isValid()){
                Trade trade = context->shouldExecuteTrade(currentInstance, *this->macroFeatures, stockName);
                // Trade trade = context->shouldExecuteTrade(currentInstance);
                if (trade.isValid){
                    sizer->processNewData(currentInstance, previousInstance);
                    hasSizerUpdated = true;
                    // Execute trade
                    Position newPosition = sizer->purchasePosition(currentBalance, stockName, trade.positionType, futureInstance);
                    newPosition.setTradeType(trade.tradeType);
                    string positionStats = context->getStats();
                    newPosition.setStats(positionStats);
                    nlohmann::json entryCtx = context->getContextData();
                    if (this->macroFeatures != nullptr) {
                        entryCtx["macroContext"] = this->macroFeatures->compute(stockName, currentDate);
                    }
                    newPosition.setEntryContextData(entryCtx);

                    this->addToBalance(-1 * (newPosition.getNumShares() * newPosition.getPurchasePrice() * newPosition.getContractSize()));

                    this->setPosition(newPosition);
                }
            }
        }

        // Update CONTEXT with the current data points after the shouldExecuteTrade and shouldSellTrade functions are executed
        context->updateContext(currentInstance, previousInstance);
        if (!hasSizerUpdated) { sizer->processNewData(currentInstance, previousInstance); }
    }

    if (!this->getPosition().getIsClosed()){
        Position &lastPosition = this->getPosition();
        lastPosition.setSellPrice(data.close.back());
        lastPosition.setSellDate(data.date.back());
        lastPosition.setIsClosed(true);
        lastPosition.setExitContextData(context->getContextData());

        string positionStats = lastPosition.getBasePositionInfo();
        string totalStats = combineSideBySide(lastPosition.getStats(), context->getStats());
        positionStats.append(totalStats);
        lastPosition.setStats(positionStats);

        this->appendClosedPosition(lastPosition);
        if (lastPosition.getPositionType().getPositiontype() == "LONG"){
            this->addToBalance((lastPosition.getNumShares() * lastPosition.getSellPrice() * lastPosition.getContractSize()));
        }
        else if (lastPosition.getPositionType().getPositiontype() == "SHORT"){
            this->addToBalance((lastPosition.getNumShares() * (lastPosition.getPurchasePrice() + (lastPosition.getPurchasePrice() - lastPosition.getSellPrice())) * lastPosition.getContractSize()));
        }

        Position emptyPosition = Position();
        this->setPosition(emptyPosition);
        context->onPositionSold();
    }

    context->clear();
    sizer->clear();

}