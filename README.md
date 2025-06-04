# BackTrader C++ Refactor for Performance

## Repo Purpose
This repository contains a C++ refactor of Python-based BackTrader trading strategies. The primary goal is to achieve significant performance improvements by leveraging the speed and efficiency of C++.

---

## Workflow

The trading analysis workflow is divided into two main parts: data acquisition using Python and strategy execution using C++.

### 1. Data Acquisition (Python) 🐍
A Python script is responsible for fetching historical stock data.
- It utilizes the **`yfinance`** library to download data for specified stock tickers.
- The downloaded data (Opening prices, Closing prices, and Dates) is then written to a **text file** with the following format:

Stock Name: AAPL
Opening: 170.50, 171.20, 170.80, 172.00
Closing: 171.00, 170.90, 172.10, 172.50
Date: 01/01/2025, 02/01/2025, 03/01/2025, 04/01/2025
Stock Name: MSFT
Opening: 400.10, 401.50, 400.80, 402.20
Closing: 401.00, 400.70, 402.30, 402.80
Date: 01/01/2025, 02/01/2025, 03/01/2025, 04/01/2025
...

### 2. Data Processing and Strategy Execution (C++) ⚙️

The C++ application reads the data from the text file and performs the trading strategy backtesting.

#### Data Storage
- The C++ code parses the text file.
- Each stock's data is stored in an **`std::unordered_map`**.
    - **Key**: Stock Name (e.g., "AAPL")
    - **Value**: An instance of a `StockData` class (or struct) containing:
        - `OpeningPrices`: An array (e.g., `std::vector<double>`) of opening prices.
        - `ClosingPrices`: An array (e.g., `std::vector<double>`) of closing prices.
        - `Dates`: An array (e.g., `std::vector<std::string>`) of corresponding dates.

#### Base Trading Strategy (`TradingStrategy`)
A base class named `TradingStrategy` is defined to encapsulate common attributes and functionalities for any trading strategy.
- **Properties**:
    - `previous_stock_data`: An array (e.g., `std::vector<double>`) storing historical price data relevant to the strategy.
    - `previous_stock_data_range`: An `int` defining the maximum length of `previous_stock_data`.
    - `balance`: A `double` representing the current trading account balance.
    - `open_positions`: An array (e.g., `std::vector<Position>`) storing currently active trades. Each `Position` object would contain:
        - `purchase_date`: The date the position was opened.
        - `purchase_price`: The price at which the asset was bought/sold short.
        - `position_type`: An enum or string indicating long or short.
    - `closed_positions`: An array (e.g., `std::vector<Position>`) storing executed trades. The `Position` object here would additionally include:
        - `sell_date`: The date the position was closed.
        - `sell_price`: The price at which the asset was sold/covered.
        - `is_closed`: A boolean flag, set to `true`.
- **Virtual Function**:
    - `virtual bool shouldExecuteTrade() = 0;`: A pure virtual function that must be overridden by derived strategy classes. This function implements the core logic of the specific trading strategy and returns `true` if a trade (buy or sell) should be executed based on the current data, and `false` otherwise.

#### Specific Trading Strategy
- A specific trading strategy (e.g., `MeanReversionStrategy`, `MovingAverageCrossoverStrategy`) is created by inheriting from the `TradingStrategy` base class.
- This derived class **overrides** the `shouldExecuteTrade()` virtual function to define its unique trade execution logic.

#### Strategy Application and Results
1. An instance of the specific trading strategy class is created.
2. This strategy instance is applied iteratively to each stock's data loaded from the `unordered_map`.
3. The `shouldExecuteTrade()` method is called at each time step (e.g., for each date) for the current stock.
4. If `shouldExecuteTrade()` returns `true`, a new position is opened or an existing one might be modified/closed based on the strategy's rules.
5. After iterating through all the historical data for all stocks, any remaining `open_positions` are automatically closed (e.g., liquidated at the last available price).
6. The instance of the specific trading strategy now contains all `open_positions` (which should be empty if all were closed) and `closed_positions`.
7. Various performance **metrics** can then be extracted from the `closed_positions` data, such as:
    - Total Profit/Loss
    - Profit/Loss per trade
    - Win rate
    - Sharpe ratio (if applicable)
    - etc.

---

This refactoring aims to provide a robust and high-performance framework for backtesting trading strategies on historical stock data.