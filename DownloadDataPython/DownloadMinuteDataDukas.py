from datetime import datetime
import dukascopy_python
import pandas as pd
import os
import json

def format_array(data):
    return " ".join(map(str, data))

dukas_tickers = []
data = dict()

start_date = datetime(2000, 1, 1)
end_date = datetime(2026, 4, 1)
interval = dukascopy_python.INTERVAL_MIN_15

HERE = os.path.dirname(os.path.abspath(__file__))
TICKERS = os.path.join(HERE, "DukascopyTickers.json")

with open(TICKERS, "r") as f:
    tickers = json.load(f)

    for info in tickers.values():
        if info["dukascopy"] is not None:
            dukas_tickers.append(info["dukascopy"])

for ticker in dukas_tickers:
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

file = open("MinuteData.txt", "w")
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
    formatted_dates = stock_data['timestamp'].dt.strftime('%Y-%m-%dT%H:%M:%S')
    file.write(f"{format_array(formatted_dates)}\n\n")

file.close()

# for stock_name, stock_data in data.items():
#     if stock_data is None:
#         continue
#     stock_data = stock_data.reset_index()
#     print("Stock: ", stock_name)
#     print("Open:")
#     print(stock_data["open"][0:20])
#     print("Close:")
#     print(stock_data["close"][0:20])
#     print("High:")
#     print(stock_data["high"][0:20])
#     print("Low:")
#     print(stock_data["low"][0:20])
#     print("Volume:")
#     print(stock_data["volume"][0:20])
#     print("timestamp:")
#     print(stock_data["timestamp"][0:20])
    