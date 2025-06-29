import numpy as np
import pandas as pd
import re
import os
import TestSystem as TS
import time

# --- 2. Data Parsing Function ---
def parse_returns_file(file_path):
    """
    Corrected parser to handle two issues:
    1. Removes extraneous single quotes (') from data lines.
    2. Correctly handles float subKeys (e.g., 0.1) without turning them into integers.
    """
    results = dict()
    # Initialize subKey as a float to handle all number types
    key, subKey, subsubKey, prevSymbol = "", 0.0, 0, ""

    with open(file_path, 'r') as file:
        for line in file:
            line = line.strip()
            if not line: continue
            
            # FIX 1: Clean any leading/trailing single quotes from the line
            line = line.strip("'")
            
            # Check if the line is a number (integer or float)
            if re.fullmatch(r"[-+]?\d*\.?\d+", line):
                val = float(line)
                if prevSymbol == "%":
                    # FIX 2: Do not convert subKey to int, keep its original value
                    subKey = val
                    results[key][subKey] = dict()
                elif prevSymbol in ["^", "&"]:
                    # Years can remain integers
                    subsubKey = int(val)
                    results[key][subKey][subsubKey] = []
                elif prevSymbol == "$":
                    # PnL values are floats
                    results[key][subKey][subsubKey].append(val)
                else:
                    # FIX 2: Handle the first subkey, keeping its original value
                    subKey = val
                    results[key][subKey] = dict()

            elif line in ["^", "&", "%", "$"]:
                prevSymbol = line
            else:
                # This handles the main parameter key (e.g., "Lookback")
                key = line
                results[key] = dict()
                prevSymbol = "" # Reset symbol after a new key
                
    return results



# --- 3. Main Processing and CSV Writing ---
if __name__ == '__main__':
    # --- Configuration ---
    # MACOS
    INPUT_FILE = "/Users/brandonhannan/Documents/Repos/backtrader/BackTrader/Returns.txt"
    OUTPUT_CSV = 'strategy_lookback_optimization_results.csv'
    INITIAL_CAPITAL = 10000.0
    
    # 1. "Download" all data by parsing the local file
    if not os.path.exists(INPUT_FILE):
        raise FileNotFoundError(f"Error: Input file '{INPUT_FILE}' not found.")
    
    print("1. Reading data from local file...")
    all_parsed_data = parse_returns_file(INPUT_FILE)
    print("   ...Data read complete.")

    param_sets_to_test = {k: sorted(list(v.keys())) for k, v in all_parsed_data.items()}
    years = sorted(list(set(y for p in all_parsed_data.values() for l in p.values() for y in l.keys())))
    all_parameter_yearly_results = {}

    # 2. Iterate through each parameter type
    for param_name, param_values in param_sets_to_test.items():
        print(f"\n--- Testing Parameter: {param_name} ---")
        current_param_results = {}

        # 3. Iterate through each value of the current parameter
        for value in param_values:
            print(f"  Testing {param_name} = {value}")
            all_pnls_for_overall_stats = []
            current_param_results[value] = {}
            run_start_time = time.time()
            
            # 4. Iterate through each year for the current parameter value
            yearly_data_source = all_parsed_data.get(param_name, {}).get(value, {})
            for year, yearly_pnls in yearly_data_source.items():
                all_pnls_for_overall_stats.extend(yearly_pnls)
                
                # 5. Store stats for this value and year
                current_param_results[value][year] = {
                    'Num Trades': len(yearly_pnls),
                    'Total PnL': sum(yearly_pnls),
                    'Average PnL': np.mean(yearly_pnls),
                    'Standard Deviation': TS.StandardDeviation(yearly_pnls),
                    'T Stat': TS.T_Stat(yearly_pnls),
                    'Mean Wins': TS.Mean_Wins(yearly_pnls),
                    'Mean Losses': TS.Mean_Losses(yearly_pnls), # Will be negative as per your function
                    'Confidence Interval': TS.Confidence_Interval(yearly_pnls),
                    'Sharpe Ratio': TS.Sharpe_Ratio(yearly_pnls),
                    'Sortino Ratio': TS.Sortino_Ratio(yearly_pnls),
                    'Max Drawdown':TS.Max_Drawdown(yearly_pnls, initial_capital=INITIAL_CAPITAL),
                    'Calmar Ratio': TS.Calmar_Ratio(yearly_pnls, initial_capital=INITIAL_CAPITAL),
                    'Risk of Ruin': TS.Risk_Of_Ruin(yearly_pnls, initial_capital=INITIAL_CAPITAL),
                    'Probability of Winning': TS.POW(yearly_pnls),
                    'Probability of Losing': TS.POL(yearly_pnls),
                    'Profit Factor': TS.Profit_Factor(yearly_pnls)
                }

            # 6. Calculate overall summary statistics
            overall_values_dict = {}
            def get_stat_list(stat_key):
                return [current_param_results[value][y][stat_key] for y in yearly_data_source if y in current_param_results[value] and np.isfinite(current_param_results[value][y].get(stat_key, np.nan))]

            overall_values_dict['Avg Total PnL'] = np.mean(get_stat_list('Total PnL')) if get_stat_list('Total PnL') else 0.0
            overall_values_dict['Avg Num Trades'] = np.mean(get_stat_list('Num Trades')) if get_stat_list('Num Trades') else 0.0
            overall_values_dict['Avg POW'] = np.mean(get_stat_list('Probability of Winning')) if get_stat_list('Probability of Winning') else 0.0
            overall_values_dict['Avg Avg PnL'] = np.mean(get_stat_list('Average PnL')) if get_stat_list('Average PnL') else 0.0
            overall_values_dict['Avg Max Drawdown'] = np.mean(get_stat_list('Max Drawdown')) if get_stat_list('Max Drawdown') else 0.0
            overall_values_dict['Avg Sortino Ratio'] = np.mean(get_stat_list('Sortino Ratio')) if get_stat_list('Sortino Ratio') else 0.0
            overall_values_dict['Avg Sharpe Ratio'] = np.mean(get_stat_list('Sharpe Ratio')) if get_stat_list('Sharpe Ratio') else 0.0
            overall_values_dict['Avg Mean Losses'] = np.mean(get_stat_list('Mean Losses')) if get_stat_list('Mean Losses') else 0.0
            overall_values_dict['Avg Mean Wins'] = np.mean(get_stat_list('Mean Wins')) if get_stat_list('Mean Wins') else 0.0
            overall_values_dict['Average T Stat'] = np.mean(get_stat_list('T Stat')) if get_stat_list('T Stat') else 0.0
            overall_values_dict['Avg Risk of Ruin'] = np.mean(get_stat_list('Risk of Ruin')) if get_stat_list('Risk of Ruin') else 0.0

            overall_values_dict['Overall T Stat'] = TS.T_Stat(all_pnls_for_overall_stats)
            overall_values_dict['Overall Std'] = TS.StandardDeviation(all_pnls_for_overall_stats)
            overall_values_dict['Overall Risk of Ruin'] = TS.Risk_Of_Ruin(all_pnls_for_overall_stats, initial_capital=INITIAL_CAPITAL)
            
            current_param_results[value]['overall'] = overall_values_dict
            
            run_end_time = time.time()
            total_trades = sum(s.get('Num Trades', 0) for s in current_param_results[value].values())
            print(f"    Completed {param_name}={value}: Total Trades={total_trades} across {len(yearly_data_source)} years (Time: {run_end_time - run_start_time:.2f}s)")
        
        if current_param_results:
            all_parameter_yearly_results[param_name] = current_param_results

    # 7. Save all results to CSV
    print(f"\n--- Writing results to {OUTPUT_CSV} ---")
    stats_to_table = ['Num Trades', 'Total PnL', 'Average PnL', 'Standard Deviation', 'T Stat', 'Mean Wins', 'Mean Losses', 'Confidence Interval', 'Profit Factor', 'Sharpe Ratio', 'Sortino Ratio', 'Max Drawdown', 'Calmar Ratio', 'Risk of Ruin', 'Probability of Winning', 'Probability of Losing']
    overall_summary_keys = ['Avg Total PnL', 'Avg Num Trades', 'Avg POW', 'Avg Avg PnL', 'Avg Max Drawdown', 'Avg Sortino Ratio', 'Avg Sharpe Ratio', 'Avg Mean Losses', 'Avg Mean Wins', 'Average T Stat', 'Overall T Stat', 'Overall P-Value', 'Overall Std', 'Avg Risk of Ruin', 'Overall Risk of Ruin']
    overall_headers = ['Average Total PnL', 'Average Trades', 'Average POW', 'Average Avg PnL', 'Average Max Drawdown', 'Average Sortino', 'Average Sharpe', 'Average Mean Losses', 'Average Mean Wins', 'Average T Stat', 'Overall T Stat', 'Overall P-Value', 'Overall Std', 'Avg Risk of Ruin', 'Overall Risk of Ruin']

    try:
        with open(OUTPUT_CSV, 'w', newline='') as f:
            for stat_key in stats_to_table:
                f.write(f"{stat_key.replace('_', ' ').title()}\n")
                for param, tested_values in all_parameter_yearly_results.items():
                    f.write(f"--- Varying: {param} ---\n")
                    data_rows, columns = [], [param] + years + overall_headers
                    for val_tested in sorted(tested_values.keys()):
                        row = [val_tested]
                        for year in years:
                            row.append(tested_values.get(val_tested, {}).get(year, {}).get(stat_key, 'N/A'))
                        overall = tested_values.get(val_tested, {}).get('overall', {})
                        for overall_key in overall_summary_keys:
                            row.append(overall.get(overall_key, 'N/A'))
                        data_rows.append(row)
                    
                    if data_rows:
                        df = pd.DataFrame(data_rows, columns=columns)
                        df.to_csv(f, index=False, float_format='%.4f')
                    f.write("\n")
                f.write("\n")
    except IOError as e:
        print(f"Error writing to CSV file {OUTPUT_CSV}: {e}")

    print(f"\nAnalysis complete. Results saved to {OUTPUT_CSV}")