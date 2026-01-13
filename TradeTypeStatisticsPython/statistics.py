import pandas as pd
import numpy as np
import re
import sys

# Name of your file
FILE_NAME = 'C:\\Users\\BrandonHannan\\source\\repos\\backtrader\\TradeTypeStatisticsPython\\Analysis.txt'

def parse_line_kv(text):
    """
    Parses a string containing one or more 'Key: Value' pairs separated by 
    2 or more spaces.
    """
    data = {}
    # Split by 2 or more spaces to separate distinct pairs on one line
    # e.g., "Price: 20   Date: 2020-01-01" -> ["Price: 20", "Date: 2020-01-01"]
    parts = re.split(r'\s{2,}', text.strip())
    
    for part in parts:
        if ':' in part:
            key, val = part.split(':', 1)
            data[key.strip()] = val.strip()
    return data

def load_trade_data(filepath):
    """
    Reads the text file and converts it into a structured Pandas DataFrame.
    """
    trades = []
    current_trade = {}
    
    try:
        with open(filepath, 'r') as f:
            lines = f.readlines()
    except FileNotFoundError:
        print(f"Error: File '{filepath}' not found.")
        sys.exit(1)

    print(f"Reading {len(lines)} lines...")
    
    for line in lines:
        stripped = line.strip()
        if not stripped:
            continue

        # Detect the start of a new trade block
        # (The file starts with Position Type, but Position Type also appears inside Stats)
        # We use the Position Type at the start of a block (not indented/inside stats) 
        # as the trigger, or simply check if 'Stats:' is not in the line
        if stripped.startswith("Position Type:") and "Stats:" not in line:
            if current_trade:
                trades.append(current_trade)
            current_trade = {}

        # Handle the two-column stats section separated by '|'
        if '|' in stripped:
            left_part, right_part = stripped.split('|')
            
            # Process Left Column (Prefix with L_ for Left/Setup)
            left_data = parse_line_kv(left_part)
            for k, v in left_data.items():
                current_trade[f"L_{k}"] = v
                
            # Process Right Column (Prefix with R_ for Right/Result)
            right_data = parse_line_kv(right_part)
            for k, v in right_data.items():
                current_trade[f"R_{k}"] = v
                
        else:
            # Handle standard lines (Header or Single column stats)
            if stripped == "Stats:":
                continue
                
            # Update current trade with found Key-Values
            current_trade.update(parse_line_kv(stripped))

    # Append the final trade
    if current_trade:
        trades.append(current_trade)

    df = pd.DataFrame(trades)
    print(f"Successfully parsed {len(df)} trades.")
    return df

def clean_dataframe(df):
    """
    Converts string columns to numeric/boolean types for analysis.
    """
    # 1. Clean Currency Columns (Remove '$')
    if 'Profit/Loss' in df.columns:
        df['Profit/Loss'] = df['Profit/Loss'].astype(str).str.replace('$', '', regex=False)
        df['Profit/Loss'] = pd.to_numeric(df['Profit/Loss'], errors='coerce')

    # 2. Convert Boolean text to actual Booleans
    # Boolean columns often contain 'True'/'False'
    for col in df.columns:
        # Check if column looks boolean
        if df[col].astype(str).str.contains('True|False').any():
            df[col] = df[col].astype(str).map({'True': True, 'False': False})

    # 3. Convert all other possible columns to numeric
    # We ignore Date columns and text identifiers
    exclude_cols = ['Position Type', 'Trade Type', 'Stock Name', 'Purchase Date', 'Sell Date']
    
    for col in df.columns:
        if col not in exclude_cols and df[col].dtype == 'object':
            # Try converting to numeric
            df[col] = pd.to_numeric(df[col], errors='ignore')
            
    # 4. Create a "Win" column
    df['Outcome'] = df['Profit/Loss'].apply(lambda x: 'WIN' if x > 0 else 'LOSS')
    
    return df

def generate_report(df):
    """
    Prints analysis to the console and saves detailed CSVs.
    """
    pd.set_option('display.max_columns', None)
    pd.set_option('display.width', 1000)
    
    # --- Analysis 1: General Stats by Trade Type ---
    print("\n" + "="*50)
    print(" SUMMARY BY TRADE TYPE")
    print("="*50)
    
    # We aggregate numeric columns by mean
    trade_type_stats = df.groupby('Trade Type').mean(numeric_only=True)
    
    # Add Count and Win Rate
    trade_counts = df['Trade Type'].value_counts()
    win_rates = df[df['Outcome'] == 'WIN'].groupby('Trade Type').size() / df.groupby('Trade Type').size()
    
    summary_df = trade_type_stats[['Profit/Loss']].copy()
    summary_df['Count'] = trade_counts
    summary_df['Win Rate'] = win_rates.fillna(0) * 100
    
    print(summary_df)
    summary_df.to_csv('analysis_by_trade_type.csv')
    print("\n(Saved full breakdown to 'analysis_by_trade_type.csv')")

    # --- Analysis 2: Winners vs Losers ---
    print("\n" + "="*50)
    print(" WINNERS VS LOSERS (Global Averages)")
    print("="*50)
    
    win_loss_stats = df.groupby('Outcome').mean(numeric_only=True)
    # Transpose for easier reading of many columns
    print(win_loss_stats.T.head(20)) # Print first 20 metrics
    
    win_loss_stats.T.to_csv('analysis_winners_vs_losers.csv')
    print("\n(Saved full breakdown to 'analysis_winners_vs_losers.csv')")

    # --- Analysis 3: Correlation Analysis (Why are they winning?) ---
    print("\n" + "="*50)
    print(" CORRELATION ANALYSIS: What drives Profit?")
    print("="*50)
    
    # Calculate correlation of all features with Profit/Loss
    correlations = df.corr(numeric_only=True)['Profit/Loss'].sort_values(ascending=False)
    
    print("Top 5 Positive Correlations (Factors that increase Profit):")
    print(correlations.drop('Profit/Loss').head(5))
    
    print("\nTop 5 Negative Correlations (Factors that decrease Profit):")
    print(correlations.drop('Profit/Loss').tail(5))


# --- Main Execution ---
if __name__ == "__main__":
    raw_df = load_trade_data(FILE_NAME)
    clean_df = clean_dataframe(raw_df)
    generate_report(clean_df)