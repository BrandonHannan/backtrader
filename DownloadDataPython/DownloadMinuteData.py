from datetime import datetime
import dukascopy_python
import pandas as pd
import os
import json

tickers = ["LIGHT.CMD/USD"]
data = dict()

start_date = datetime(2026, 1, 1)
end_date = datetime(2026, 2, 1)
interval = dukascopy_python.INTERVAL_MIN_15

for ticker in tickers:
        try:
            df = dukascopy_python.fetch(
                    ticker,
                    interval,
                    dukascopy_python.OFFER_SIDE_BID,
                    start_date,
                    end_date
                )
            if df.empty:
                print(f"Warning: No data found for {ticker}")
                data[ticker] = None
            else:
                data[ticker] = df
                print(f"  Downloaded {ticker} ({len(df)} rows)")
        except Exception as e:
            print(f"Error downloading {ticker}: {e}")
            data[ticker] = None

for stock_name, stock_data in data.items():
    if stock_data is None:
        continue
    stock_data = stock_data.reset_index()
    print("Stock: ", stock_name)
    print("Open:")
    print(stock_data["open"][0:20])
    print("Close:")
    print(stock_data["close"][0:20])
    print("High:")
    print(stock_data["high"][0:20])
    print("Low:")
    print(stock_data["low"][0:20])
    print("Volume:")
    print(stock_data["volume"][0:20])
    print("timestamp:")
    print(stock_data["timestamp"][0:20])
    