#include "CustomStrategy.h"

CustomStrategy::CustomStrategy(double balance, unique_ptr<BasePositionSize> sizer, unique_ptr<BaseContext> context): BaseStrategy(balance, move(sizer), move(context)) {}


void CustomStrategy::ExecuteStrategy(const string &stockName, const StockData &data){

    auto* sizer = this->getPositionSizer();
    auto* context = this->getContext();
    int size = data.close.size();

    for (int i = 1; i<size; i++){
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

        StockDataInstance previousInstance(i - 1, previousOpen, previousClose, previousHigh, previousLow, previousVolume, previousDate);
        StockDataInstance currentInstance(i, currentOpen, currentClose, currentHigh, currentLow, currentVolume, currentDate);

        sizer->processNewData(currentInstance, previousInstance);
        context->updateContext(currentInstance, previousInstance);

        Position currentPosition = this->getPosition();

        if (currentPosition.getIsClosed()){
            // Check if the position size and context instances are ready and full of data
            if (sizer->isValid() && context->isValid()){
                Trade trade = context->shouldExecuteTrade(currentInstance);

                if (!trade.isValid){
                    // Execute trade
                    Position newPosition = sizer->purchasePosition(this->getBalance(), trade.positionType, currentInstance);
                    newPosition.setTradeType(trade.tradeType);
                    this->setPosition(newPosition);
                }
            }
        }
        else{
            bool shouldSell = context->shouldSellTrade(currentPosition, currentInstance);
            if (shouldSell){
                currentPosition.setSellDate(currentDate);
                currentPosition.setSellPrice(currentClose);
                currentPosition.setIsClosed(true);

                this->appendClosedPosition(currentPosition);
            }
        }

    }
}