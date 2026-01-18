import pandas as pd
import numpy as np
import re
import sys

# Change this to your filename
# FILE_NAME = 'C:\\Users\\BrandonHannan\\source\\repos\\backtrader\\TradeTypeStatisticsPython\\Analysis.txt'
FILE_NAME = 'C:\\Users\\brand\\Documents\\Repos\\backtrader\\TradeTypeStatisticsPython\\Analysis.txt'

def parse_line_kv(text):
    """
    Parses a string containing 'Key: Value' pairs separated by multiple spaces.
    This version splits by 2+ spaces to ensure fields like 'Stock Name' 
    do not get merged into 'Trade Type'.
    """
    data = {}
    # Split the line by 2 or more spaces
    parts = re.split(r'\s{2,}', text.strip())
    
    for part in parts:
        # Only process parts that have a colon
        if ':' in part:
            # Split only on the first colon found
            key, val = part.split(':', 1)
            data[key.strip()] = val.strip()
            
    return data

def load_data(filename):
    """
    Reads the text file and converts it into a structured DataFrame.
    """
    try:
        with open(filename, 'r') as f:
            lines = f.readlines()
    except FileNotFoundError:
        print(f"Error: File '{filename}' not found.")
        sys.exit(1)
        
    trades = []
    current_trade = {}
    
    print("Reading file...")
    
    for line in lines:
        stripped = line.strip()
        if not stripped:
            continue
            
        # Detect Start of New Trade Block
        # We use 'Position Type:' at the start of the line (and ensure it's not the indented one inside Stats)
        if stripped.startswith("Position Type:") and "Stats:" not in line:
            if current_trade:
                trades.append(current_trade)
            current_trade = {}
            
        # Check for Split Line (|) inside Stats
        if '|' in stripped:
            parts = stripped.split('|')
            left_part = parts[0].strip()
            right_part = parts[1].strip()
            
            # Left side (L_)
            l_data = parse_line_kv(left_part)
            for k, v in l_data.items():
                current_trade[f"L_{k}"] = v
            
            # Right side (R_)
            r_data = parse_line_kv(right_part)
            for k, v in r_data.items():
                current_trade[f"R_{k}"] = v
                
        else:
            # Normal line (Header or Inline Stats)
            if stripped == "Stats:":
                continue
            
            # Parse Key-Values
            kv_data = parse_line_kv(stripped)
            current_trade.update(kv_data)
            
    # Append the last trade
    if current_trade:
        trades.append(current_trade)
        
    df = pd.DataFrame(trades)
    
    # --- Data Cleaning ---
    
    # 1. Clean Profit/Loss (Remove '$')
    if 'Profit/Loss' in df.columns:
        df['Profit/Loss'] = df['Profit/Loss'].astype(str).str.replace('$', '', regex=False)
        df['Profit/Loss'] = pd.to_numeric(df['Profit/Loss'], errors='coerce')
        
    # 2. Convert Numeric Columns
    # Explicitly exclude identifier columns from numeric conversion
    cols_to_ignore = ['Position Type', 'Trade Type', 'Stock Name', 'Purchase Date', 'Sell Date', 'Outcome']
    
    for col in df.columns:
        if col not in cols_to_ignore:
            try:
                # Handle Boolean text "True"/"False" -> 1/0
                if df[col].astype(str).str.contains('True|False').all():
                     df[col] = df[col].map({'True': 1, 'False': 0})
                else:
                    df[col] = pd.to_numeric(df[col], errors='ignore')
            except:
                pass
                
    # 3. Create Outcome Column
    df['Outcome'] = df['Profit/Loss'].apply(lambda x: 'WIN' if x > 0 else 'LOSS')
    
    return df

def analyze_stats(df, title_suffix="Global"):
    """
    Performs the requested analysis:
    1. Aggregates
    2. Winners vs Losers Table
    3. Correlation Analysis
    """
    
    # --- 1. Aggregates ---
    total_pl = df['Profit/Loss'].sum()
    n_wins = (df['Outcome'] == 'WIN').sum()
    n_loss = (df['Outcome'] == 'LOSS').sum()
    total_trades = len(df)
    win_rate = (n_wins / total_trades * 100) if total_trades > 0 else 0
    avg_pl = df['Profit/Loss'].mean() if total_trades > 0 else 0
    
    if title_suffix == "Global":
        print(f"- the total summation of profit/loss of all the trades made: {total_pl:.4f}")
        print(f"- Number of winning trades made: {n_wins}")
        print(f"- Number of losing trades made: {n_loss}")
        print(f"- Total number of trades made: {total_trades}")
    else:
        # Per Trade Type Output Format
        print(f"- the total summation of profit/loss of all the trades made for the given trade type: {total_pl:.4f}")
        print(f"- Number of winning trades made for the given trade type: {n_wins}")
        print(f"- Number of losing trades made for the given trade type: {n_loss}")
        print(f"- Total number of trades made for the given trade type: {total_trades}")
        print(f"- Win rate for the given trade type: {win_rate:.2f}%")
        print(f"- Profit/loss for the given trade type: {avg_pl:.4f}")

    # --- 2. Winners vs Losers Table ---
    print("\n" + "="*50)
    print(f" WINNERS VS LOSERS ({title_suffix} Averages)")
    print("="*50)
    
    # Select only numeric columns
    numeric_df = df.select_dtypes(include=[np.number])
    
    # Group by Outcome and calculate Mean
    if not numeric_df.empty:
        grouped = df.groupby('Outcome')[numeric_df.columns].mean().T
        
        # Ensure LOSS and WIN columns exist even if one is missing in data
        if 'LOSS' not in grouped.columns: grouped['LOSS'] = np.nan
        if 'WIN' not in grouped.columns: grouped['WIN'] = np.nan
        
        # Reorder to match user format (LOSS then WIN)
        grouped = grouped[['LOSS', 'WIN']]
        
        # Print the dataframe
        print(grouped)
    else:
        print("No numeric data available for statistics.")

    # --- 3. Correlation Analysis ---
    print("\n" + "="*50)
    print(f" CORRELATION ANALYSIS{(' FOR ' + title_suffix.upper()) if title_suffix != 'Global' else ''}: What drives Profit?")
    print("="*50)
    
    if len(df) > 1:
        # Calculate correlation with Profit/Loss
        corr = df.corr(numeric_only=True)['Profit/Loss'].sort_values(ascending=False)
        # Drop Profit/Loss itself from the list
        corr = corr.drop('Profit/Loss', errors='ignore')
        
        print("Top 5 Positive Correlations (Factors that increase Profit):")
        print(corr.head(5))
        
        print("\nTop 5 Negative Correlations (Factors that decrease Profit):")
        print(corr.tail(5))
    else:
        print("Not enough data to calculate correlations (need at least 2 trades).")

# --- Main Execution ---
if __name__ == "__main__":
    # Load Data
    df = load_data(FILE_NAME)
    
    # 1. Global Statistics
    print("--- GLOBAL STATISTICS ---")
    analyze_stats(df, "Global")
    
    # 2. Per Trade Type Statistics
    # Filter to get unique trade types only (ensuring no 'Stock Name' pollution)
    if 'Trade Type' in df.columns:
        trade_types = df['Trade Type'].unique()
        
        for trade_type in trade_types:
            print("\n\n") 
            print(f"--- ANALYZING: {trade_type} ---")
            
            # Filter for specific trade type
            sub_df = df[df['Trade Type'] == trade_type]
            
            # Run analysis
            analyze_stats(sub_df, trade_type)
    else:
        print("\nError: 'Trade Type' column not found. Check file format.")