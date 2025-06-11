import yfinance as yf
from datetime import datetime

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

    


