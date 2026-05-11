from datetime import datetime
import dukascopy_python
import pandas as pd
import os
import json


def format_array(data):
    return " ".join(map(str, data))

# Fix: Use Dukascopy's specific WTI Crude Oil identifier
tickers = ["NATGAS.CMD/USD"]
data = dict()

# Start date set post-1970 to bypass Windows mktime limits
start_date = datetime(2026, 1, 1)
end_date = datetime(2026, 2, 1)
interval = dukascopy_python.INTERVAL_DAY_1

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
                # Reset index so 'timestamp' becomes a standard column for printing
                df = df.reset_index()
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

file = open("MinuteData1.txt", "w")
for stock_name, stock_data in data.items():
    if stock_data is None:
        continue
    stock_data = stock_data.reset_index()
    file.write(f"Stock: {stock_name}\n")
    file.write(f"Open:\n")
    file.write(f"{format_array(stock_data['open'])}\n")
    file.write("Close:\n")
    file.write(f"{format_array(stock_data['close'])}\n")
    file.write("High:\n")
    file.write(f"{format_array(stock_data['high'])}\n")
    file.write("Low:\n")
    file.write(f"{format_array(stock_data['low'])}\n")
    file.write("Volume:\n")
    file.write(f"{format_array(stock_data['volume'])}\n")
    file.write("Date:\n")
    file.write(f"{format_array(stock_data['timestamp'].dt.strftime("%Y-%m-%d"))}\n\n")

file.close()