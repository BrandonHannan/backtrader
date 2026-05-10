from datetime import datetime
import os
import json

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

def parse_related_stocks_raw(md_path):
    """Return the full primary→{related: sign} dict from the markdown JSON block."""
    try:
        with open(md_path, encoding="utf-8") as f:
            text = f.read()
    except OSError as e:
        print(f"Warning: cannot read {md_path}: {e}. Related-ticker expansion disabled.")
        return {}

    fence = text.find("```json")
    if fence == -1:
        print(f"Warning: no ```json block found in {md_path}. Related-ticker expansion disabled.")
        return {}
    json_start = text.find("{", fence)
    close_fence = text.find("```", json_start)
    if json_start == -1 or close_fence == -1:
        print(f"Warning: malformed json fence in {md_path}. Related-ticker expansion disabled.")
        return {}
    json_end = text.rfind("}", json_start, close_fence)
    if json_end == -1:
        print(f"Warning: malformed json body in {md_path}. Related-ticker expansion disabled.")
        return {}

    try:
        return json.loads(text[json_start:json_end + 1])
    except json.JSONDecodeError as e:
        print(f"Warning: invalid JSON in {md_path}: {e}. Related-ticker expansion disabled.")
        return {}


def parse_related_stocks(md_path):
    return {primary: list(related.keys()) for primary, related in parse_related_stocks_raw(md_path).items()}

def expand_with_related(selected, related_map):
    expanded = []
    for ticker in selected:
        expanded.append(ticker)
        expanded.extend(related_map.get(ticker, []))
    deduped = list(dict.fromkeys(expanded))
    added = len(deduped) - len(selected)
    print(f"Expanded {len(selected)} -> {len(deduped)} tickers (added {added} related).")
    return deduped

if __name__ == "__main__":
    import yfinance as yf

    HERE = os.path.dirname(os.path.abspath(__file__))
    MD_PATH = os.path.join(HERE, "Possible Stocks.md")
    REL_PATH = os.path.join(HERE, "RelatedStocks.md")

    categories = parse_ticker_index(MD_PATH)
    related_raw = parse_related_stocks_raw(REL_PATH)

    universe = []
    for cat_tickers in categories.values():
        universe.extend(cat_tickers)
    for primary, related in related_raw.items():
        universe.append(primary)
        universe.extend(related.keys())
    tickers = list(dict.fromkeys(universe))
    print(f"Downloading {len(tickers)} unique tickers across {len(categories)} categories.")

    # Emit clean JSON dict (with +/-/mixed signs) for the C++ MacroFeatures module to consume.
    out_dir = os.path.join(HERE, "..", "output")
    os.makedirs(out_dir, exist_ok=True)
    with open(os.path.join(out_dir, "related_stocks.json"), "w", encoding="utf-8") as f:
        json.dump(related_raw, f, indent=2)
    with open(os.path.join(out_dir, "categories.json"), "w", encoding="utf-8") as f:
        json.dump(categories, f, indent=2)

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
