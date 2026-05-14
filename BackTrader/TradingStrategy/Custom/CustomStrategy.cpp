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

        StockDataInstance previousInstance(i - 1, previousOpen, previousClose, previousHigh, previousLow, previousVolume, previousDate);
        StockDataInstance currentInstance(i, currentOpen, currentClose, currentHigh, currentLow, currentVolume, currentDate);
        StockDataInstance futureInstance(i + 1, futureOpen, futureClose, futureHigh, futureLow, futureVolume, futureDate);

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
                    this->addToBalance((currentPosition.getNumShares() * currentPosition.getSellPrice()));
                }
                else if (currentPosition.getPositionType().getPositiontype() == "SHORT"){
                    this->addToBalance((currentPosition.getNumShares() * (currentPosition.getPurchasePrice() + (currentPosition.getPurchasePrice() - currentPosition.getSellPrice()))));
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

                    this->addToBalance(-1 * (newPosition.getNumShares() * newPosition.getPurchasePrice()));
                    
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
            this->addToBalance((lastPosition.getNumShares() * lastPosition.getSellPrice()));
        }
        else if (lastPosition.getPositionType().getPositiontype() == "SHORT"){
            this->addToBalance((lastPosition.getNumShares() * (lastPosition.getPurchasePrice() + (lastPosition.getPurchasePrice() - lastPosition.getSellPrice()))));
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
        // Handle the case where the date is not found
        return;
    }

    for (int i = dateIndex; i<(size - 1); i++){
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

        StockDataInstance previousInstance(i - 1, previousOpen, previousClose, previousHigh, previousLow, previousVolume, previousDate);
        StockDataInstance currentInstance(i, currentOpen, currentClose, currentHigh, currentLow, currentVolume, currentDate);
        StockDataInstance futureInstance(i + 1, futureOpen, futureClose, futureHigh, futureLow, futureVolume, futureDate);

        Position &currentPosition = this->getPosition();

        if (!currentPosition.getIsClosed()){
            bool shouldSell = context->shouldSellTrade(currentPosition, currentInstance);

            if (shouldSell || i == size - 2){
                double exitPrice = -1;
                int initialMinuteIndex = binarySearchInitialMinuteDate(minuteData.date, currentDate);
                int finalMinuteIndex = binarySearchLastMinuteDate(minuteData.date, futureInstance.date);
                if (initialMinuteIndex == -1 || finalMinuteIndex == -1 || initialMinuteIndex > finalMinuteIndex) {
                    exitPrice = currentPosition.getExitPrice(currentInstance, futureInstance);
                }
                else{
                    vector<StockDataInstance> minuteSlice;
                    minuteSlice.reserve(finalMinuteIndex - initialMinuteIndex + 1);
                    for (int idx = initialMinuteIndex; idx <= finalMinuteIndex; idx++) {
                        minuteSlice.emplace_back(idx,
                            minuteData.open[idx], minuteData.close[idx],
                            minuteData.high[idx], minuteData.low[idx],
                            minuteData.volume[idx], minuteData.date[idx]);
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
                    this->addToBalance((currentPosition.getNumShares() * currentPosition.getSellPrice()));
                }
                else if (currentPosition.getPositionType().getPositiontype() == "SHORT"){
                    this->addToBalance((currentPosition.getNumShares() * (currentPosition.getPurchasePrice() + (currentPosition.getPurchasePrice() - currentPosition.getSellPrice()))));
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

                    this->addToBalance(-1 * (newPosition.getNumShares() * newPosition.getPurchasePrice()));

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
            this->addToBalance((lastPosition.getNumShares() * lastPosition.getSellPrice()));
        }
        else if (lastPosition.getPositionType().getPositiontype() == "SHORT"){
            this->addToBalance((lastPosition.getNumShares() * (lastPosition.getPurchasePrice() + (lastPosition.getPurchasePrice() - lastPosition.getSellPrice()))));
        }

        Position emptyPosition = Position();
        this->setPosition(emptyPosition);
        context->onPositionSold();
    }

    context->clear();
    sizer->clear();

}

int binarySearchDate(const vector<string> &dates, const string &targetDate) {
    int left = 0;
    int right = dates.size() - 1;

    while (left <= right) {
        int mid = left + (right - left) / 2;

        if (dates[mid] == targetDate) {
            return mid;
        } else if (dates[mid] < targetDate) {
            left = mid + 1;
        } else {
            right = mid - 1;
        }
    }

    return -1; // Not found
}

// Finds the FIRST occurrence of the target date
int binarySearchInitialMinuteDate(const vector<string> &minuteDates, const string &targetDailyDate) {
    int left = 0;
    int right = minuteDates.size() - 1;
    int firstIndex = -1;

    while (left <= right) {
        int mid = left + (right - left) / 2;
        string currentDailyDate = minuteDates[mid].substr(0, 10);

        if (currentDailyDate == targetDailyDate) {
            firstIndex = mid; // Record match, but keep searching LEFT for earlier ones
            right = mid - 1;
        } else if (currentDailyDate < targetDailyDate) {
            left = mid + 1;
        } else {
            right = mid - 1;
        }
    }

    return firstIndex;
}

// Finds the LAST occurrence of the target date
int binarySearchLastMinuteDate(const vector<string> &minuteDates, const string &targetDailyDate) {
    int left = 0;
    int right = minuteDates.size() - 1;
    int lastIndex = -1;

    while (left <= right) {
        int mid = left + (right - left) / 2;
        string currentDailyDate = minuteDates[mid].substr(0, 10);

        if (currentDailyDate == targetDailyDate) {
            lastIndex = mid; // Record match, but keep searching RIGHT for later ones
            left = mid + 1;
        } else if (currentDailyDate < targetDailyDate) {
            left = mid + 1;
        } else {
            right = mid - 1;
        }
    }

    return lastIndex;
}