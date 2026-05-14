# Commit Correctness Report — c24d260 + 8d6cb6c

**Commits reviewed:** `c24d260` (Add enhanced exit price calculation and minute data handling) and `8d6cb6c` (Refactor data handling in main and improve exit price calculation)
**Date of review:** 2026-05-14
**Method:** Static analysis + build verification (no full backtest run)

---

## Executive Summary

**The program does not build.** Three compilation errors in `CustomStrategy.cpp` prevent `a.exe` from being produced — the binary search helper functions are used before they are declared. This is the highest-priority issue to fix before anything else can be evaluated at runtime.

Beyond the build failure, the minute-data execution path has a second crash risk: if the minute data begins on the same date as the first daily bar, the loop accesses `data.open[-1]` (undefined behaviour). The core entry/exit logic is otherwise sound for a futures backtester — fills at next-bar open, stop-losses at the triggering bar's price, no look-ahead bias in the P&L math. The main realism gap is no commission/slippage model, which is expected for a first pass but overstates returns.

---

## Section 1: Implementation Correctness

| # | File | Line | Issue | Severity | Trigger condition |
|---|---|---|---|---|---|
| **C1** | `CustomStrategy.cpp` | 156, 200, 201 | **Build error:** `binarySearchDate`, `binarySearchInitialMinuteDate`, `binarySearchLastMinuteDate` called before definition; no forward declarations exist in `.cpp` or `.h` | **FATAL** | Every build |
| **C2** | `CustomStrategy.cpp` | 163 | **Potential UB:** loop starts at `i = dateIndex`; if `dateIndex == 0` (minute data starts on first daily bar), `data.open[i - 1]` → `data.open[-1]` on first iteration | **CRASH** | Minute data first date == first daily bar date |
| **C3** | `CustomStrategy.cpp` | 158–160 | **Silent early return:** if minute start date not found in daily data, function returns with no log, no error, zero trades | Medium | Dukascopy minute start date differs from daily data start date |
| **C4** | `CustomStrategy.cpp` | 202–203 | **Silent downgrade:** if binary search returns -1 for either minute index, falls back to daily-only exit price with no indication to the caller or any log | Low | Weekend / holiday gap between daily and minute data |
| **C5** | `CustomStrategy.cpp` | 208–212 | **Wrong index in slice:** `minuteSlice.emplace_back(idx, ...)` passes the global array index as `StockDataInstance::index`. In `getExitPrice`, this field is not read, so no crash; but the field is wrong if used elsewhere (e.g., serialization) | Low | Every minute-mode exit |
| **C6** | `Position.cpp` | 209–218 | **Empty slice safety:** if `minuteSlice` is empty, the range-for loop simply does not execute and falls through to daily logic. Safe. | OK | — |
| **C7** | `Position.cpp` | 210, 235 | **Stop-loss direction correct:** LONG uses `minuteInstance.low <= stopLossPrice`, SHORT uses `minuteInstance.high >= stopLossPrice`. Both correct. | OK | — |
| **C8** | `StrategyRunner.h` | 68 | **Pass-by-const-ref correct:** `minuteData` parameter is `const unordered_map<string, StockData>&`. No unnecessary copy. | OK | — |

### Fix for C1 (build-blocking)

Add forward declarations at the top of `CustomStrategy.cpp`, after the `#include`:

```cpp
static int binarySearchDate(const vector<string>& dates, const string& targetDate);
static int binarySearchInitialMinuteDate(const vector<string>& minuteDates, const string& targetDailyDate);
static int binarySearchLastMinuteDate(const vector<string>& minuteDates, const string& targetDailyDate);
```

### Fix for C2 (crash risk)

Change the loop start in the minute `ExecuteStrategy` at line 163:

```cpp
// Before (crashes if dateIndex == 0):
for (int i = dateIndex; i < (size - 1); i++) {

// After:
for (int i = max(dateIndex, 1); i < (size - 1); i++) {
```

---

## Section 2: Crash Risks — Ranked

1. **[CERTAIN] Build failure (C1):** The program does not compile. The three binary search functions are defined at lines 317–380 but called at lines 156 and 200–201. C++ requires declarations to precede use in a translation unit. Every build fails until forward declarations are added.

2. **[HIGH] Out-of-bounds access when `dateIndex == 0` (C2):** If the first 15-min bar date matches the first daily bar's date, `binarySearchDate` returns `0`. The loop then tries `data.open[-1]` on the first iteration — undefined behaviour, likely a crash or garbage data. Reproducible any time the Dukascopy 15-min and daily data share the same start date, which is the typical case.

3. **[LOW] Missing `MinuteData.txt` produces silent zero trades:** If `MinuteData.txt` is absent, `ReadData` returns `{}` with a `cerr` message. In minute mode (choices 3/4), `filteredMinuteData` is empty, so `StrategyRunner::run()` skips every ticker (`it == minuteData.end()`) and writes no P&L. The program does not crash, but the user sees zero results with no explanation beyond the `cerr` line at startup.

---

## Section 3: Missing Pieces

| Item | Status | Impact |
|---|---|---|
| Forward declarations for `binarySearchDate` etc. in `CustomStrategy.cpp` | **Absent** | Build fails — program cannot run |
| Guard for `dateIndex < 1` in minute `ExecuteStrategy` | **Absent** | Crash when minute and daily data share start date |
| Log message on silent early return (line 158–160) | **Absent** | Zero trades with no diagnostic when start dates mismatch |
| Log or counter when silent downgrade fires (line 202–203) | **Absent** | Cannot tell how often minute precision was actually used |
| Unit tests for 4-param `getExitPrice` | **Absent** | No regression protection for the new minute exit logic |
| User-facing message when `MinuteData.txt` is missing | **Partial** | `cerr` from DataReader exists; no guidance in the menu path |
| Comments explaining 3-file data setup in `Main.cpp` | **Absent** | Unintuitive: yfinance `data.txt` for modes 1/2, Dukascopy files for modes 3/4 |
| `BaseStrategy` subclasses implementing 3-param override | **Complete** | `CustomStrategy` is the only subclass and implements both overloads correctly |

---

## Section 4: Backtesting Logic Soundness

### Entry timing — Sound

Signal fires at bar `i`'s close (`DowContext.cpp`: `currentData.close > maxPrice`). Entry price is bar `i+1`'s open (`ATRPositionSize.cpp`: `data.open` where `data = futureInstance`, set at `CustomStrategy.cpp:98`). No look-ahead bias: the decision uses yesterday's close, the fill uses tomorrow's open.

### Exit timing — Sound with minor date-record inconsistency

**Daily path (`Position.cpp:160–197`):**
- Stop hit today → exit at `currentOpen` if the open already gapped through the stop, otherwise exit at `stopLossPrice`. Realistic.
- Stop not hit → exit at `futureData.open`. Realistic.

**Date-record inconsistency (`CustomStrategy.cpp:57–61`):** When a stop fires, `setSellDate(currentDate)` is called, but the exit price comes from the `(currentInstance, futureInstance)` pair — i.e., it may be next bar's open. The record says "sold today" but may be priced at tomorrow's open. P&L math is correct; only the recorded date is potentially off by one day for non-gap stop-outs.

**Minute path (`Position.cpp:209–218`):** Iterates 15-min bars in chronological order and exits at the first bar that hits the stop. Exit price is that bar's open (if already gapped through) or `stopLossPrice`. More precise than the daily path and internally consistent.

### ATR position sizing — Minor timing issue

`processNewData(currentInstance, previousInstance)` is called at `CustomStrategy.cpp:95` *after* the trade signal has been evaluated. This means the ATR used to size the position entered at bar `i+1` is calculated from bar `i`'s data (the signal bar), not bar `i-1`. Practically this introduces slight optimism: the stop distance reflects the most recent volatility rather than the prior bar's volatility. ATR stabilises over its lookback period, so the effect is small but structurally imprecise.

### Stop-loss check off-by-one — Minor

The stop is checked against `currentLow` / `currentHigh` at bar `i` (`CustomStrategy.cpp:52–54`), but the position was entered at bar `i+1`'s open. The bar whose low triggered the stop came before the entry was taken. In practice this is rare because the ATR multiplier places the stop far from the entry price, but it is technically possible for the stop to "fire" on data that precedes the actual entry.

### Commission and slippage — Absent (expected for this stage)

No commission, spread, or slippage model exists. Balance updates use raw fill prices (`CustomStrategy.cpp:74, 108`). For commodity futures, exchange fees are ~$5–10/contract and can be added later. Stated returns are overstated by the round-trip cost. Document this assumption somewhere (e.g., a comment in `CustomStrategy.cpp` near the balance update).

### Short selling — Realistic for futures

Shorts are always available with no borrow cost (`ATRPositionSize.cpp:26–35`). This is correct for commodity futures, which are symmetric instruments. It would be unrealistic for equity shorts, but the universe is futures-only.

### Date alignment and loop boundaries — Correct

Daily-only loop: `for (int i = 1; i < size - 1; i++)` — `previousInstance`, `currentInstance`, and `futureInstance` are always valid. Entry guard `i < size - 2` prevents entering on the second-to-last bar. End-of-data forced exit uses `data.close.back()`. All correct.

Minute-only loop: starts at `max(dateIndex, 1)` **after the C2 fix is applied.** Without the fix, this is a crash.

### Overall verdict

The core backtesting economics are **sound for a commodity futures backtester**: entries at next-bar open, exits at the triggering bar's price, no look-ahead on fill prices. The two issues to act on before trusting results are the build error (C1) and the `dateIndex == 0` crash (C2). The soundness gaps (no commission, ATR timing, sell-date record) are acceptable for an initial implementation but should be documented.

---

## Appendix: Build Output (annotated)

```
./TradingStrategy/Custom/CustomStrategy.cpp:156:21: error: 'binarySearchDate' was not declared in this scope
./TradingStrategy/Custom/CustomStrategy.cpp:200:42: error: 'binarySearchInitialMinuteDate' was not declared in this scope
./TradingStrategy/Custom/CustomStrategy.cpp:201:40: error: 'binarySearchLastMinuteDate' was not declared in this scope
make: *** [Makefile:32: a] Error 1
```

→ All three errors are caused by C1. Adding forward declarations (or moving the function definitions above `ExecuteStrategy`) resolves all three.

**Pre-existing warnings (not introduced by these commits):**

```
WindowStatistics.h:19: warning: 'indexSum' will be initialized after 'sumXY' [-Wreorder]
WindowStatistics.cpp:70: warning: comparison of integer expressions of different signedness [-Wsign-compare]
TrendIdentifier.h:18: warning: 'currentTrend' will be initialized after 'mode' [-Wreorder]
Position.h:32: warning: 'isClosed' will be initialized after 'entryContextData' [-Wreorder]
BaseStrategy.h:28: warning: 'closedPositions' will be initialized after 'position' [-Wreorder]
BreakoutContext.cpp:115-121: warning: unused variable (multiple) [-Wunused-variable]
DowContext.cpp:103, 126: warning: unused variable 'macd', 'signal' [-Wunused-variable]
```

These predate the two commits and are not regressions.
