from datetime import datetime
import dukascopy_python
import os
import json
import concurrent.futures
import threading

def format_array(data):
    return " ".join(map(str, data))

# Initialize the lock for thread-safe file writing
file_lock = threading.Lock()

def fetch_and_write(ticker, contract_size, start, end, interval, file_handle):
    """Worker function to download data and write it directly to the file."""
    try:
        df = dukascopy_python.fetch(
            ticker,
            interval,
            dukascopy_python.OFFER_SIDE_BID,
            start,
            end
        )

        if df.empty:
            return ticker, False, "No data found", 0

        df = df.reset_index()

        # Pre-format the massive strings OUTSIDE the lock to keep the lock fast
        formatted_dates = df['timestamp'].dt.strftime('%Y-%m-%dT%H:%M:%S')
        output_block = (
            f"Stock: {ticker}\n"
            f"ContractSize:\n{contract_size}\n"
            f"Open:\n{format_array(df['open'])}\n"
            f"Close:\n{format_array(df['close'])}\n"
            f"High:\n{format_array(df['high'])}\n"
            f"Low:\n{format_array(df['low'])}\n"
            f"Volume:\n{format_array(df['volume'])}\n"
            f"Date:\n{format_array(formatted_dates)}\n\n"
        )
        
        # Acquire the lock to ensure only one thread writes to the file at a time
        with file_lock:
            file_handle.write(output_block)
            file_handle.flush() # Force write to disk immediately
            
        return ticker, True, None, len(df)
        
    except Exception as e:
        return ticker, False, str(e), 0

if __name__ == "__main__":
    dukas_tickers = []
    
    start_date = datetime(2000, 1, 1)
    end_date = datetime(2026, 4, 1)
    interval = dukascopy_python.INTERVAL_MIN_15
    
    HERE = os.path.dirname(os.path.abspath(__file__))
    TICKERS = os.path.join(HERE, "../DukascopyTickers.json")
    
    with open(TICKERS, "r") as f:
        tickers = json.load(f)

        seen = set()
        for info in tickers.values():
            duk = info.get("dukascopy")
            cs = info.get("contractSize")
            if duk is None or cs is None:
                continue
            if duk in seen:
                continue
            seen.add(duk)
            dukas_tickers.append((duk, cs))

    total_tickers = len(dukas_tickers)
    completed_count = 0

    print(f"Starting downloads for {total_tickers} tickers...")

    with open("OfferBid_SellPriceData15MinuteData.txt", "w") as file_handle:
        # Using 10 worker threads. You can increase max_workers (e.g., 20 or 30)
        # but don't set it too high or the Dukascopy server might rate-limit/block you.
        with concurrent.futures.ThreadPoolExecutor(max_workers=20) as executor:

            # Submit all ticker tasks to the thread pool
            future_to_ticker = {
                executor.submit(fetch_and_write, ticker, cs, start_date, end_date, interval, file_handle): ticker
                for ticker, cs in dukas_tickers
            }
            
            # Process results as they finish, out of order
            for future in concurrent.futures.as_completed(future_to_ticker):
                completed_count += 1
                try:
                    ticker, success, msg, row_count = future.result()
                    if success:
                        progress = (completed_count / total_tickers) * 100
                        print(f"  Downloaded {ticker} ({row_count} rows) | Progress: {progress:.2f}%")
                    else:
                        print(f"Warning/Error for {ticker}: {msg} | Progress: {(completed_count / total_tickers) * 100:.2f}%")
                except Exception as exc:
                    ticker = future_to_ticker[future]
                    print(f"{ticker} generated an exception: {exc}")

    print("All downloads complete and saved to OfferBid_SellPriceData15MinuteData.txt")