import numpy as np
import math
from scipy import stats
import pandas as pd


def StandardDeviation(results):
    if len(results) < 2:  # ddof=1 requires at least 2 elements for a non-NaN result
        return 0.0
    returns = np.array(results)
    if np.all(np.isnan(returns)):  # All inputs are NaN
        return np.nan
    return np.nan_to_num(np.std(returns[~np.isnan(returns)], ddof=1), nan=0.0)


def Mean(results):
    if not results:
        return 0.0
    returns = np.array(results)
    if np.all(np.isnan(returns)):
        return np.nan
    return np.nan_to_num(np.mean(returns[~np.isnan(returns)]), nan=0.0)


def T_Stat(results):
    if len(results) == 0:
        return np.nan
    returns = np.array(results)

    # T-test
    n = len(returns)
    if n < 2:  # Need at least 2 for std_dev with ddof=1 and t-test
        return np.nan
    mean_return = np.mean(returns)
    std_dev = np.std(returns, ddof=1)

    if pd.isna(std_dev) or std_dev == 0:
        return np.nan

    denominator = std_dev / np.sqrt(n)
    if denominator == 0:
        return np.nan  # t-statistic would be infinite or undefined.

    t_stat_val = (mean_return - 0) / denominator
    df = n - 1

    # Step 2: One-tailed p-value (greater than test)
    p_value = 1 - stats.t.cdf(t_stat_val, df)
    return np.nan_to_num(p_value, nan=np.nan)


def Mean_Wins(results):
    if not results:
        return 0.0
    returns = np.array(results)
    positive = returns[returns > 0]
    if positive.size == 0:  # .size is more robust for empty numpy arrays
        return 0.0
    return np.mean(positive)


def Mean_Losses(results):
    if not results:
        return 0.0
    returns = np.array(results)
    negative = returns[returns < 0]
    if negative.size == 0:
        return 0.0
    return np.mean(negative)


def Confidence_Interval(results):
    if len(results) == 0:
        return "(0.00, 0.00)"
    returns = np.array(results)
    n = len(returns)
    if n < 2:
        return "(NaN, NaN)"
    mean_return = np.mean(returns)
    std_dev = np.std(returns, ddof=1)
    df = n - 1

    if pd.isna(std_dev):
        return "(NaN, NaN)"

    if std_dev == 0:  # All values are the same
        return f"({mean_return:.2f}, {mean_return:.2f})"

    confidence_level = 0.95
    se = std_dev / np.sqrt(n)  # Standard error
    if pd.isna(se) or se == 0:
        return f"({mean_return:.2f}, {mean_return:.2f})" if std_dev == 0 else "(NaN, NaN)"

    # t critical value
    t_crit = stats.t.ppf((1 + confidence_level) / 2, df)
    if pd.isna(t_crit):  # Should not happen for df >= 1
        return "(NaN, NaN)"

    # Confidence Interval
    ci_lower = mean_return - t_crit * se
    ci_upper = mean_return + t_crit * se
    return f"({ci_lower}, {ci_upper})"


def Profit_Factor(results):
    if not results:
        return 0.0
    returns = np.array(results)
    if returns.size == 0:
        return 0.0
    winning_trades_sum = np.sum(returns[returns > 0])
    losing_trades_sum_abs = np.abs(np.sum(returns[returns < 0]))

    if losing_trades_sum_abs == 0:
        if winning_trades_sum > 0:
            return float('inf')
        else:  # No profit, no loss
            return 0.0  # Or 1.0, convention varies. 0.0 if no profit.
    if winning_trades_sum == 0:  # Covers cases with losses but no wins
        return 0.0

    return winning_trades_sum / losing_trades_sum_abs


def Sharpe_Ratio(results):
    if not results:
        return 0.0
    returns = np.array(results, dtype=float)
    n = len(returns)
    if n == 0:
        return 0.0
    mean_return_per_trade = np.mean(returns)
    std_dev_per_trade = np.std(returns, ddof=1)

    # Avoid division by zero if std_dev is zero
    if std_dev_per_trade == 0:
        # If mean is positive and std dev is 0, Sharpe is infinite
        # If mean is zero or negative and std dev is 0, Sharpe is undefined/zero
        return float('inf') if mean_return_per_trade > 0 else 0.0

    sharpe = (mean_return_per_trade / std_dev_per_trade) * np.sqrt(n)  # Assuming 1 year of trading
    return sharpe


def Sortino_Ratio(results):
    if not results:
        return 0.0
    returns = np.array(results, dtype=float)
    n = len(returns)
    if n == 0:
        return 0.0
    mean_return_per_trade = np.mean(returns)
    downside_returns = returns[returns < 0]

    if len(downside_returns) == 0:
        # No losses, Sortino is infinite if mean return is positive
        return float('inf') if mean_return_per_trade > 0 else 0.0

    downside_std_dev = np.std(downside_returns, ddof=1)

    if downside_std_dev == 0:
        # Should only happen if all losses are identical and non-zero,
        # or if there are no losses (handled above).
        # If mean is positive, treat as infinite. Otherwise zero/undefined.
        return float('inf') if mean_return_per_trade > 0 else 0.0

    # Simplified: (Mean / DownsideStdDev) * Sqrt(Trades Per Year)
    # Assumes Risk-Free Rate = 0
    sortino = (mean_return_per_trade / downside_std_dev) * np.sqrt(n) # Assuming 1 year of trading
    return sortino


def Max_Drawdown(results, initial_capital=10000):
    if not results:
        return 0.0
    returns = np.array(results)
    if returns.size == 0:
        return 0.0
    equity_curve = initial_capital + np.cumsum(returns)
    if len(equity_curve) < 2:
        return 0.0
    peaks = np.maximum.accumulate(equity_curve)
    drawdowns = (peaks - equity_curve) / peaks
    return np.max(drawdowns)


def Calmar_Ratio(results, initial_capital=10000):
    if not results:
        return 0.0
    returns = np.array(results, dtype=float)
    n = len(returns)
    if n == 0:
        return 0.0
    annualized_return = np.mean(returns) * n
    max_dd = Max_Drawdown(results)  # Max DD calculated over trade sequence

    if max_dd == 0:
        # If max_dd is 0, Calmar is infinite if annualized return is positive
        return float('inf') if annualized_return > 0 else 0.0
    elif annualized_return <= 0:
        # If return is not positive, Calmar is typically not considered meaningful or is <= 0
        return annualized_return / max_dd  # Will be <= 0

    return annualized_return / max_dd


def Risk_Of_Ruin(results, initial_capital=10000):
    if not results:
        return 0.0
    returns = np.array(results)
    n = len(returns)
    if n == 0:
        return 0.0
    mean_return = np.mean(returns)
    std_dev = np.std(returns, ddof=1)
    if std_dev == 0:
        return 0.0 if mean_return >= 0 else 1.0
    minimum_loss = abs(np.min(returns))
    if minimum_loss == 0:
        return 0.0
    N = int(initial_capital / minimum_loss)
    try:
        exponent = -(2 * mean_return * N) / (std_dev ** 2)
        ror = math.exp(exponent)
        return max(0.0, min(1.0, ror))
    except OverflowError:
        return 0.0 if exponent < 0 else 1.0
    except ZeroDivisionError:
        return 0.0 if mean_return >= 0 else 1.0



def POW(results):
    if not results:
        return 0.0
    returns = np.array(results)
    positive = returns[returns > 0]
    n = len(returns)
    p = len(positive)
    if n == 0 or p == 0:
        return 0.0
    POW = float(p)/n
    return POW


def POL(results):
    if not results:
        return 0.0
    returns = np.array(results)
    negative = returns[returns < 0]
    n = len(returns)
    p = len(negative)
    if n == 0 or p == 0:
        return 0.0
    POL = float(p) / n
    return POL


def statistics(results, initial_capital):
    returns = np.array(results)

    # T-test
    n = len(returns)
    mean_return = np.mean(returns)
    std_dev = np.std(returns, ddof=1)

    t_stat = (mean_return - 0) / (std_dev / np.sqrt(n))  # popmean = 0
    df = n - 1

    # Step 2: One-tailed p-value (greater than test)
    p_value = 1 - stats.t.cdf(t_stat, df)

    print(f"Sample mean: {mean_return:.4f}")
    print(f"Sample Standard Deviation (Volatility): {std_dev:.4f}")
    # print(f"T-statistic: {t_stat:.4f}")
    print(f"One-tailed p-value: {p_value:.4f}")

    alpha = 0.05  # significance level
    if p_value < alpha:
        print("✅ Result is statistically significant: Positive expectancy is likely real.")
    else:
        print("❌ Result is NOT statistically significant: Can't confirm positive expectancy.")

    # Confidence Level
    confidence_level = 0.95
    se = std_dev / np.sqrt(n)          # Standard error

    # t critical value
    t_crit = stats.t.ppf((1 + confidence_level) / 2, df)

    # Confidence Interval
    ci_lower = mean_return - t_crit * se
    ci_upper = mean_return + t_crit * se

    print(f"{int(confidence_level*100)}% Confidence Interval: ({ci_lower:.4f}, {ci_upper:.4f})")


    def calculate_profit_factor(trades):
        winning_trades = [trade for trade in trades if trade > 0]
        losing_trades = [trade for trade in trades if trade < 0]

        gross_profit = sum(winning_trades)
        gross_loss = abs(sum(losing_trades))

        if gross_loss == 0:
            return float('inf')  # No losses, infinite profit factor

        return gross_profit / gross_loss

    profit_factor = calculate_profit_factor(returns)
    print("Profit Factor:", profit_factor)

    trades_per_year = n/6

    # Sharpe Ratio
    def annualized_sharpe_ratio(returns, trades_per_year):
        mean_return_per_trade = np.mean(returns)
        std_dev_per_trade = np.std(returns, ddof=1)

        # Avoid division by zero if std_dev is zero
        if std_dev_per_trade == 0:
            # If mean is positive and std dev is 0, Sharpe is infinite
            # If mean is zero or negative and std dev is 0, Sharpe is undefined/zero
            return float('inf') if mean_return_per_trade > 0 else 0.0

        sharpe = (mean_return_per_trade / std_dev_per_trade) * np.sqrt(trades_per_year)
        return sharpe

    sharpe_ratio = annualized_sharpe_ratio(returns, trades_per_year)
    print(f'Sharpe Ratio = {sharpe_ratio}')

    # Sortino Ratio
    def sortino_ratio(returns, trades_per_year):
        mean_return_per_trade = np.mean(returns)
        downside_returns = returns[returns < 0]

        if len(downside_returns) == 0:
            # No losses, Sortino is infinite if mean return is positive
            return float('inf') if mean_return_per_trade > 0 else 0.0

        downside_std_dev = np.std(downside_returns, ddof=1)

        if downside_std_dev == 0:
            # Should only happen if all losses are identical and non-zero,
            # or if there are no losses (handled above).
            # If mean is positive, treat as infinite. Otherwise zero/undefined.
            return float('inf') if mean_return_per_trade > 0 else 0.0

        # Simplified: (Mean / DownsideStdDev) * Sqrt(Trades Per Year)
        # Assumes Risk-Free Rate = 0
        sortino = (mean_return_per_trade / downside_std_dev) * np.sqrt(trades_per_year)
        return sortino

    sortino_ratio = sortino_ratio(returns, trades_per_year)
    print(f'Sortino Ratio = {sortino_ratio}')

    # Max Drawdown
    def calculate_max_drawdown(equity_curve):
        peaks = np.maximum.accumulate(equity_curve)
        drawdowns = (peaks - equity_curve) / peaks
        return np.max(drawdowns)

    equity_curve = initial_capital + np.cumsum(returns)
    max_dd = calculate_max_drawdown(equity_curve)
    print(f"Max Drawdown: {max_dd:.2%}")

    # Calmar Ratio
    def calculate_calmar_ratio(returns, equity_curve, trades_per_year):
        annualized_return = np.mean(returns) * trades_per_year
        max_dd = calculate_max_drawdown(equity_curve)  # Max DD calculated over trade sequence

        if max_dd == 0:
            # If max_dd is 0, Calmar is infinite if annualized return is positive
            return float('inf') if annualized_return > 0 else 0.0
        elif annualized_return <= 0:
            # If return is not positive, Calmar is typically not considered meaningful or is <= 0
            return annualized_return / max_dd  # Will be <= 0

        return annualized_return / max_dd

    calmar = calculate_calmar_ratio(returns, equity_curve, trades_per_year)
    print(f"Calmar Ratio: {calmar:.2f}")

    # 8. Portfolio Simulation (Improved)
    num_simulations = 10000
    simulated_means = np.zeros(num_simulations)
    for i in range(num_simulations):
        resampled = np.random.choice(returns, size=n, replace=True)
        simulated_means[i] = np.mean(resampled)

    print(f"Probability of profit: {np.mean(simulated_means > 0):.1%}")

    # Risk of Ruin
    minimum_loss = abs(np.min(returns))
    N = int(initial_capital/minimum_loss)
    ror = math.exp(-(2*mean_return * N)/(std_dev**2))
    print(f"Risk of Ruin: {ror:.5%}")

    # POW and POL
    POW = 0
    POL = 0
    for trade in returns:
        if trade > 0:
            POW = POW + 1
        else:
            POL = POL + 1
    POW = float(POW)/n
    POL = float(POL)/n
    print(f"Percentage of Wins: {POW:.4%}")
    print(f"Percentage of Losses: {POL:.4%}")

    # Monte Carlo Simulation
    print()
    num_trades_per_sim = 50
    num_simulations = 5000

    # simulation_results = MonteCarloSimulation.run_monte_carlo_simulation(returns, initial_capital, num_trades_per_sim, num_simulations)

