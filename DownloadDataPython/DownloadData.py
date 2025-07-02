import yfinance as yf
import pandas as pd
from datetime import datetime
import matplotlib.pyplot as plt
import matplotlib.dates as mdates
from datetime import datetime
import numpy as np

def format_array(data):
    data = list(data)
    string = ""
    for x in data:
        string = string + f"{x} "
    string = string[0:len(string) - 2]
    return string

tickers = ['CL=F', 'BZ=F', 'NG=F', 'HO=F', 'GC=F', 'SI=F', 'PL=F', "PA=F", 'HG=F', 'ZC=F']
data = dict()

for ticker in tickers:
    try:
        df = yf.download(ticker,
                        start=datetime(2000, 1, 1),
                        end=datetime(2024, 1, 1),
                        progress=False,
                        multi_level_index=False)
        if df.empty:
            print(f"Warning: No data found for {ticker}")
            data[ticker] = None
        else:
            data[ticker] = df
            print(f"  Downloaded {ticker} ({len(df)} rows)")
    except Exception as e:
        print(f"Error downloading {ticker}: {e}")
        data[ticker] = None

# Writes Stock Data to file
file = open("data.txt", "w")
for stock_name, stock_data in data.items():
    file.write(f"Stock: {stock_name}\n")
    file.write(f"Open:\n")
    file.write(f"{format_array(stock_data['Open'])}\n")
    file.write("Close:\n")
    file.write(f"{format_array(stock_data['Close'])}\n")
    file.write("High:\n")
    file.write(f"{format_array(stock_data['High'])}\n")
    file.write("Low:\n")
    file.write(f"{format_array(stock_data['Low'])}\n")
    file.write("Volume:\n")
    file.write(f"{format_array(stock_data['Volume'])}\n")
    file.write("Date:\n")
    file.write(f"{format_array([str(date.date()) for date in stock_data.index])}\n\n")

file.close()

start_date = '2008-09-11'
end_date = '2008-10-15'
end_location = data["PA=F"].index.asof(pd.to_datetime(end_date))
end_location = data["PA=F"].index.get_loc(end_location)
start_location = max(0, end_location - 105)
sliced_data = data["PA=F"].iloc[start_location : end_location + 1]
# 24:200
dates = [x for x in range(0, len(sliced_data))]
doubles = sliced_data["Close"]
prev_avg_p = np.mean(doubles[0:20])
prev_std_p = np.std(doubles[0:35])
curr_avg_p = np.mean(doubles[35:70])
curr_std_p = np.std(doubles[35:70])
volumes = sliced_data['Volume']
prev_avg_v = np.mean(volumes[0:35])
prev_std_v = np.std(volumes[0:35])
curr_avg_v = np.mean(volumes[35:70])
curr_std_v = np.std(volumes[35:70])
print(f"Prev Avg Price: ${prev_avg_p}\nCurrent Avg Price: ${curr_avg_p}")
print(f"Prev STD Price: {prev_std_p}\nCurrent STD Price: {curr_std_p}")
print(f"Prev Avg Volume: {prev_avg_v}\nCurrent Avg Volume: {curr_avg_v}")
print(f"Prev STD Volume: {prev_std_v}\nCurrent STD Volume: {curr_std_v}")
plt.figure(figsize=(8, 6))
plt.subplot(2, 1, 1)
plt.plot(dates, doubles, marker='o')
plt.title('Price')

plt.subplot(2, 1, 2)
plt.plot(dates, volumes, marker='o', color='red')
plt.title('Volume')

plt.tight_layout()
plt.show()

    


