---
name: stats-code-reviewer
description: >
  This skill should be used when the user invokes "/stats-code-review", asks to
  "review the StatisticsPython code", "check statistical correctness", "verify the
  stats workflow", or wants to ensure changes to StatisticsPython/ are still correct.
  Performs a structured code review of Stats.py and TestSystem.py: validates the
  data-flow pipeline against CLAUDE.md, audits every statistical formula for
  mathematical accuracy, flags known bugs, and runs end-to-end verification.
version: 0.1.0
allowed-tools:
  - Read
  - Grep
  - Glob
  - Bash
---

# stats-code-reviewer

Perform a complete correctness review of `StatisticsPython/Stats.py` and
`StatisticsPython/TestSystem.py`. The review has five steps executed in order.
Report findings for every step before moving to the next.

---

## Step 1 — Identify What Changed

Run `git diff HEAD -- StatisticsPython/` to list changed lines.
Also run `git status StatisticsPython/` to catch untracked files.

Summarize which functions were added, removed, or modified. This scopes the
remaining steps — focus deeper scrutiny on changed functions, but still perform
the full formula audit in Step 3.

---

## Step 2 — Workflow Integrity (CLAUDE.md compliance)

Read `StatisticsPython/Stats.py` and verify each of the following. Mark each
**PASS** or **FAIL**.

### 2a. File paths
- Input file: must be `os.path.join(OUTPUT_DIR, 'Returns.txt')` where
  `OUTPUT_DIR` resolves to the repo-root `output/` directory.
  Must **not** point to `BackTrader/Returns.txt`.
- Output file: must write to `output/strategy_lookback_optimization_results.csv`.

### 2b. Initial capital
`INITIAL_CAPITAL` must equal `num_valid_stocks × balance`, where:
- `balance` is the value in
  `BackTrader/StrategyRunner/DowATRStrategy/DowATRBaseCase.h` (currently `1000.0`).
- `num_valid_stocks` is the count of tickers in `data.txt` that pass all
  six OHLCV size-consistency checks in `Main.cpp` (currently 68, giving `68000.0`).

To verify: read the `balance` field from `DowATRBaseCase.h`, count valid tickers
by grepping `"Stock:"` in `data.txt`, then confirm
`INITIAL_CAPITAL == count × balance`. Flag if it does not match.

### 2c. Parser state machine (`parse_returns_file`)
Trace through the five delimiter states. Each must map correctly:

| Condition | Action |
|---|---|
| Non-delimiter, non-numeric line | Sets `key` (parameter name); clears `prevSymbol` |
| Numeric line AND `prevSymbol in ["%", ""]` | New `subKey` (parameter value) |
| Numeric line AND `prevSymbol in ["^", "&"]` | New `subsubKey` (year, cast to `int`) |
| Numeric line AND `prevSymbol == "$"` | Appends float to `results[key][subKey][subsubKey]` |
| Line is `^`, `&`, `%`, or `$` | Updates `prevSymbol` only |

Verify no delimiter is silently swallowed and no numeric PnL line is
misclassified as a year or parameter value.

---

## Step 3 — Statistical Formula Audit

Read `StatisticsPython/TestSystem.py` and audit each function below.
Mark each **PASS**, **FAIL**, or **WARN**.

### Simple functions

**`StandardDeviation(results)`**
- Uses `np.std(ddof=1)` (sample standard deviation). ✓
- Returns `0.0` when `len(results) < 2` (ddof=1 is undefined for n=1). ✓
- Must not return `np.nan` for valid non-empty input.

**`Mean(results)`**
- Uses `np.mean` with NaN values stripped before averaging.
- Returns `0.0` for empty list.

**`Mean_Wins(results)`**
- Filters `returns > 0`, then takes mean.
- Returns `0.0` (not NaN) when no winning trades exist.

**`Mean_Losses(results)`**
- Filters `returns < 0`, then takes mean.
- Return value is **negative** (mean of negative numbers). Callers must not
  negate it again.
- Returns `0.0` when no losing trades exist.

**`POW(results)` — Probability of Winning**
- Formula: `count(returns > 0) / len(returns)`.
- Trades with PnL == 0 are **excluded** from wins (strict `> 0`).
- Returns `0.0` for empty input or zero wins.

**`POL(results)` — Probability of Losing**
- Formula: `count(returns < 0) / len(returns)`.
- Trades with PnL == 0 are **excluded** from losses (strict `< 0`).
- Consequence: `POW + POL ≤ 1.0` is always true; equality holds only when no
  trades land exactly at 0. **WARN** if `POW + POL > 1.0` is observed.

**`Profit_Factor(results)`**
- Formula: `sum(returns > 0) / abs(sum(returns < 0))`.
- Returns `float('inf')` when gross losses == 0 and gross profit > 0.
- Returns `0.0` when gross profit == 0 (no wins, regardless of losses).

**`Confidence_Interval(results)`**
- 95% confidence interval using the t-distribution with `df = n - 1`.
- Formula: `mean ± t_crit × (std_ddof1 / sqrt(n))`.
- Returns the string `"(lower, upper)"` — not a tuple; callers must not unpack it.
- Returns `"(0.00, 0.00)"` for empty input, `"(NaN, NaN)"` for n < 2.

### Complex functions

**`T_Stat(results)`**
> **NAMING WARNING:** Despite the name, this function returns a **one-tailed
> p-value** (`1 - t.cdf(t_stat, df)`), not the t-statistic itself.

- Formula: compute `t = mean / (std_ddof1 / sqrt(n))`, then return
  `p = 1 - t.cdf(t, df)` where `df = n - 1`.
- A lower p-value indicates stronger evidence of positive expectancy.
- Verify every call site in `Stats.py` uses the return value as a p-value
  (e.g., stored under keys like `'T Stat'`, `'Overall T Stat'`). **FAIL** if
  any caller treats it as a raw t-statistic.

**`Sharpe_Ratio(results)`**
- Formula: `(mean / std_ddof1) × sqrt(n)`.
- Annualization uses trade count `n`, not a calendar factor. This is
  intentional and self-consistent within this project.
- Returns `float('inf')` if std == 0 and mean > 0; `0.0` if std == 0 and mean ≤ 0.

**`Sortino_Ratio(results)`**
- Formula: `(mean / downside_std) × sqrt(n)` where `downside_std =
  np.std(returns[returns < 0], ddof=1)`.
- Uses `ddof=1` on the downside subset — can be unstable when there are fewer
  than 3 losing trades. **WARN** if typical trade counts make this common.
- Returns `float('inf')` if no losses or downside_std == 0 and mean > 0.

**`Max_Drawdown(results, initial_capital)`**
- Formula: `max((running_peak - equity) / running_peak)` over the equity curve
  `initial_capital + cumsum(pnls)`.
- **Requires `initial_capital` to be passed explicitly.** The default is `10000`,
  not the project's `68000` — any call that omits this arg silently uses the
  wrong starting equity.

**`Calmar_Ratio(results, initial_capital)`**
> **KNOWN BUG:** The current implementation calls `Max_Drawdown(results)` inside
> `Calmar_Ratio` **without forwarding `initial_capital`**, so Max Drawdown is
> computed with a `10000` equity base instead of `68000`. This causes
> Calmar values to be incorrect.

- Expected formula: `(mean × n) / Max_Drawdown(results, initial_capital)`.
- **FAIL** if this bug persists in modified code.
- **FAIL** if `Stats.py` calls `Calmar_Ratio` without passing `INITIAL_CAPITAL`.

**`Risk_Of_Ruin(results, initial_capital)`**
- Formula: `exp(-(2 × mean × N) / std²)` where `N = floor(initial_capital / |min_pnl|)`.
- Clamped to `[0.0, 1.0]`.
- Verify `initial_capital` is passed from `Stats.py` (currently is — check it
  remains so after any changes).

---

## Step 4 — Known Gaps in Stats.py

Check whether these pre-existing issues have been resolved or worsened:

1. **`Overall P-Value` is never computed.**
   `overall_summary_keys` references `'Overall P-Value'` but
   `overall_values_dict` never sets that key — the CSV always outputs `'N/A'`
   for that column. Flag if a fix was attempted but implemented incorrectly
   (e.g., computing a t-statistic instead of the p-value from `T_Stat()`).

2. **`Calmar_Ratio` missing `initial_capital`** (see Step 3 above).

---

## Step 5 — End-to-End Run Verification

Run the script and confirm correct output:

```bash
cd /c/Users/brand/Documents/Repos/backtrader
python StatisticsPython/Stats.py
```

Check:
- Script exits with no exceptions or tracebacks.
- `output/strategy_lookback_optimization_results.csv` is created or updated.
- Open the CSV and verify at least one data row has non-zero, non-`'N/A'` values
  for `Average Total PnL`, `Average Sharpe`, and `Average Sortino`.
- Spot-check **Sharpe Ratio** for one parameter value and year:
  1. Read the raw PnL list for that parameter/year from `output/Returns.txt`.
  2. Compute `mean / std_ddof1 × sqrt(n)` by hand (or with a one-liner).
  3. Confirm it matches the CSV value to 4 decimal places.

---

## Pass / Fail Summary

After completing all steps, output a summary table:

| Check | Status | Notes |
|---|---|---|
| Input path | PASS/FAIL | |
| Output path | PASS/FAIL | |
| INITIAL_CAPITAL formula | PASS/FAIL | Expected vs actual |
| Parser state machine | PASS/FAIL | |
| StandardDeviation | PASS/FAIL | |
| Mean | PASS/FAIL | |
| Mean_Wins / Mean_Losses | PASS/FAIL | |
| POW / POL | PASS/WARN/FAIL | |
| Profit_Factor | PASS/FAIL | |
| Confidence_Interval | PASS/FAIL | |
| T_Stat (returns p-value) | PASS/WARN/FAIL | |
| Sharpe_Ratio | PASS/FAIL | |
| Sortino_Ratio | PASS/WARN/FAIL | |
| Max_Drawdown (capital arg) | PASS/FAIL | |
| Calmar_Ratio bug | PASS/FAIL | Fixed or still present? |
| Risk_Of_Ruin (capital arg) | PASS/FAIL | |
| Overall P-Value gap | PASS/WARN/FAIL | |
| End-to-end run | PASS/FAIL | |
| Sharpe spot-check | PASS/FAIL | |

A review **passes** when all items are PASS or WARN. Any FAIL blocks approval.
