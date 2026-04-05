import yfinance as yf
import pandas as pd
from datetime import datetime
import matplotlib.pyplot as plt
import matplotlib.dates as mdates
from datetime import datetime
import numpy as np
import os
import random
from math import ceil

def format_array(data):
    data = list(data)
    string = ""
    for x in data:
        string = string + f"{x} "
    string = string[0:len(string) - 2]
    return string

def parse_ticker_index(md_path):
    categories = {}
    current = None
    in_index = False
    with open(md_path, encoding="utf-8") as f:
        for line in f:
            line = line.rstrip()
            if line == "## Ticker Index":
                in_index = True
                continue
            if not in_index:
                continue
            if line.startswith("## "):
                break
            if line.startswith("### "):
                current = line[4:].strip()
                categories[current] = []
            elif current and line.strip() and not line.startswith(">"):
                categories[current].extend(line.split())
    return categories

def _prompt_custom_blend(categories, names, n):
    raw = input("\nSelect categories (comma-separated numbers, e.g. 1,3,5): ").strip()
    parts = [p.strip() for p in raw.split(",")]
    indices = []
    for p in parts:
        if not p.isdigit() or not (1 <= int(p) <= n):
            print(f"Invalid category number '{p}'. Falling back to Commodity Futures — Core.")
            return categories[names[0]]
        idx = int(p) - 1
        if idx not in indices:
            indices.append(idx)

    if not indices:
        print("No categories selected. Falling back to Commodity Futures — Core.")
        return categories[names[0]]

    print()
    samples = []
    summary_parts = []
    for idx in indices:
        name = names[idx]
        pool = categories[name]
        pct_raw = input(f"  % of '{name}' to include ({len(pool)} tickers): ").strip()
        if not pct_raw.isdigit() or not (1 <= int(pct_raw) <= 100):
            print(f"Invalid percentage '{pct_raw}'. Falling back to Commodity Futures — Core.")
            return categories[names[0]]
        pct = int(pct_raw)
        count = max(1, ceil(len(pool) * pct / 100))
        selected = random.sample(pool, count)
        samples.append((name, selected))
        summary_parts.append(f"{count} from '{name}' ({pct}%)")

    combined = list(dict.fromkeys(ticker for _, s in samples for ticker in s))
    print(f"\nCustom blend: {' + '.join(summary_parts)} = {len(combined)} tickers.")
    return combined


def select_tickers(categories):
    names = list(categories.keys())
    n = len(names)

    print("\nSelect tickers to download:")
    for i, name in enumerate(names, 1):
        count = len(categories[name])
        print(f"  {i:>2}. {name}  ({count} tickers)")
    print(f"  {n + 1:>2}. [random]       — 100 random tickers from all categories")
    print(f"  {n + 2:>2}. [blend]        — 50% from one category, 50% from another")
    print(f"  {n + 3:>2}. [custom blend] — choose multiple categories with % of each to include")

    choice = input("\nEnter selection: ").strip()

    if choice == str(n + 1):
        pool = list(dict.fromkeys(
            ticker for tickers in categories.values() for ticker in tickers
        ))
        selected = random.sample(pool, min(100, len(pool)))
        print(f"\nRandomly selected {len(selected)} tickers from all categories.")
        return selected

    if choice == str(n + 2):
        print("\nBlend: enter two category numbers to split 50/50.")
        a = input("  First category number: ").strip()
        b = input("  Second category number: ").strip()
        if not a.isdigit() or not b.isdigit():
            print("Invalid input. Falling back to Commodity Futures — Core.")
            return categories[names[0]]
        a_idx, b_idx = int(a) - 1, int(b) - 1
        if not (0 <= a_idx < n and 0 <= b_idx < n):
            print("Invalid category numbers. Falling back to Commodity Futures — Core.")
            return categories[names[0]]
        pool_a = categories[names[a_idx]]
        pool_b = categories[names[b_idx]]
        take_a = min(50, len(pool_a))
        take_b = min(50, len(pool_b))
        if take_a < 50:
            print(f"  Warning: '{names[a_idx]}' only has {take_a} tickers (fewer than 50).")
        if take_b < 50:
            print(f"  Warning: '{names[b_idx]}' only has {take_b} tickers (fewer than 50).")
        selected_a = random.sample(pool_a, take_a)
        selected_b = random.sample(pool_b, take_b)
        combined = list(dict.fromkeys(selected_a + selected_b))
        print(f"\nBlend: {take_a} from '{names[a_idx]}' + {take_b} from '{names[b_idx]}' = {len(combined)} tickers.")
        return combined

    if choice == str(n + 3):
        return _prompt_custom_blend(categories, names, n)

    if choice.isdigit():
        idx = int(choice) - 1
        if 0 <= idx < n:
            selected = categories[names[idx]]
            print(f"\nSelected '{names[idx]}': {len(selected)} tickers.")
            return selected

    print("Invalid selection. Falling back to Commodity Futures — Core.")
    return categories[names[0]]

MD_PATH = os.path.join(os.path.dirname(os.path.abspath(__file__)), "Possible Stocks.md")
categories = parse_ticker_index(MD_PATH)
tickers = select_tickers(categories)

data = dict()

for ticker in tickers:
    try:
        df = yf.download(ticker,
                        start=datetime(1900, 1, 1),
                        end=datetime(2026, 1, 1),
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
    if stock_data is None:
        continue
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

# start_date = '2008-09-11'
# end_date = '2008-10-15'
# end_location = data["PA=F"].index.asof(pd.to_datetime(end_date))
# end_location = data["PA=F"].index.get_loc(end_location)
# start_location = max(0, end_location - 105)
# sliced_data = data["PA=F"].iloc[start_location : end_location + 1]
# # 24:200
# dates = [x for x in range(0, len(sliced_data))]
# doubles = sliced_data["Close"]
# prev_avg_p = np.mean(doubles[0:20])
# prev_std_p = np.std(doubles[0:35])
# curr_avg_p = np.mean(doubles[35:70])
# curr_std_p = np.std(doubles[35:70])
# volumes = sliced_data['Volume']
# prev_avg_v = np.mean(volumes[0:35])
# prev_std_v = np.std(volumes[0:35])
# curr_avg_v = np.mean(volumes[35:70])
# curr_std_v = np.std(volumes[35:70])
# print(f"Prev Avg Price: ${prev_avg_p}\nCurrent Avg Price: ${curr_avg_p}")
# print(f"Prev STD Price: {prev_std_p}\nCurrent STD Price: {curr_std_p}")
# print(f"Prev Avg Volume: {prev_avg_v}\nCurrent Avg Volume: {curr_avg_v}")
# print(f"Prev STD Volume: {prev_std_v}\nCurrent STD Volume: {curr_std_v}")
# plt.figure(figsize=(8, 6))
# plt.subplot(2, 1, 1)
# plt.plot(dates, doubles, marker='o')
# plt.title('Price')

# plt.subplot(2, 1, 2)
# plt.plot(dates, volumes, marker='o', color='red')
# plt.title('Volume')

# plt.tight_layout()
# plt.show()
