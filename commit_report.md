# Logic-Soundness Audit — Full BackTrader Trading System

**Scope:** the code paths exercised by all eight execution modes in [Main.cpp:143-295](BackTrader/Main.cpp#L143-L295). The menu is a two-step pick: data source (1 = yfinance daily, 2 = Dukascopy bid+ask), then execution mode. For yfinance the modes are `1 = ExecuteBaseCase` and `2 = ExecuteAllSweeps`. For Dukascopy the modes are `1/2 = 1-minute`, `3/4 = 15-minute`, `5/6 = daily` — odd = base case, even = sweep. Data is loaded from `../data.txt` (yfinance) or `../OfferBid_…` + `../OfferAsk_…` (Dukascopy) and is dispatched at [Main.cpp:291-295](BackTrader/Main.cpp#L291-L295).
**Method:** static review of the order-execution loop, sizing, exit logic, aggregation, and data-realism assumptions against the current `main` branch. No live backtest run.
**Date of review:** 2026-05-19. This supersedes the 2026-05-16 audit; resolved findings have moved to Section 4.
**Grading:** Critical / Medium / Low by impact on the trustworthiness of the reported P&L.

> The long-only behaviour and the simplified `currentData.close > maxPrice` entry rule in [DowContext::shouldExecuteTrade](BackTrader/TradingContext/DowContext/DowContext.cpp#L122-L163) are **intentional design choices** and are **not** flagged as findings.

---

## Executive Verdict

The recent commits land the two original Critical fixes (contract multipliers + frictions) and four of the previous Medium items. The engine now correctly multiplies P&L by `contractSize` in `Position::toJson`, `BaseStrategy::getYearlyReturns`, and `BaseStrategy::getAllReturns`; deducts a per-round-trip friction at every close; and uses Ask vs. Bid sides for the appropriate fills. However, **two Critical issues remain or were introduced by the new menu structure**:

1. The base-case printout block in `DowATRStrategy.h` re-computes P&L *without* `contractSize`, so the user-visible "Total P&L", "Best Trade", "Worst Trade", and "Avg P&L per Trade" lines are still off by the contract multiplier — and disagree with the per-year totals printed five lines later.
2. Every Dukascopy intra-day menu option (1-minute, 15-minute) dispatches through the daily-only overload. The 2-arg `ExecuteAllSweeps(data, minuteData, related)` and the minute-slice exit-price refinement inside `CustomStrategy.cpp` are dead code, and the strategy iterates minute bars as if each were a day.

**Counts:** Critical: 2 · Medium: 7 · Low: 8 · Sound: 14.

---

## Section 1 — Critical Findings

### C1 — Base-case printout sums P&L without `contractSize`, contradicting the JSON / yearly outputs   *(Affects: All BaseCase modes)*

**Location:** [DowATRStrategy.h:372-395](BackTrader/StrategyRunner/DowATRStrategy/DowATRStrategy.h#L372-L395) (daily overload), [DowATRStrategy.h:511-534](BackTrader/StrategyRunner/DowATRStrategy/DowATRStrategy.h#L511-L534) (minute overload)

**The code:**
```cpp
// DowATRStrategy.h:373-376 — printout aggregator
for (const Position& pos : positions) {
    bool isLong = pos.getPositionType().getPositiontype() == "LONG";
    double pnl = isLong
        ? (pos.getSellPrice() - pos.getPurchasePrice()) * pos.getNumShares()
        : (pos.getPurchasePrice() - pos.getSellPrice()) * pos.getNumShares();

    totalPnL += pnl;
    if (pnl > 0) { winningTrades++; totalWin += pnl; }
    ...
```

This block feeds **every dollar quantity printed by the base case**: `Total P&L`, `Avg P&L per Trade`, `Avg Winning Trade`, `Avg Losing Trade`, `Best Trade`, `Worst Trade`, `Profit Factor`, and the `Top 10 Most / Least Profitable Trades` lists at [DowATRStrategy.h:438-462](BackTrader/StrategyRunner/DowATRStrategy/DowATRStrategy.h#L438-L462). Yet the corresponding "fixed" code paths *do* multiply by `contractSize`:

```cpp
// Position.cpp:270-276 — toJson (used to write output/data.json)
double pnl = 0.0;
if (positionType.getPositiontype() == "LONG") {
    pnl = (sellPrice - purchasePrice) * numShares * contractSize;
} else if (positionType.getPositiontype() == "SHORT") {
    pnl = (purchasePrice - sellPrice) * numShares * contractSize;
}

// BaseStrategy.cpp:94-99 — getYearlyReturns (used by Returns.txt and the per-year print)
if (pos.getPositionType().getPositiontype() == "LONG"){
    totalProfit = (pos.getSellPrice() - pos.getPurchasePrice()) * pos.getNumShares() * pos.getContractSize();
}
else if (pos.getPositionType().getPositiontype() == "SHORT"){
    totalProfit = (pos.getPurchasePrice() - pos.getSellPrice()) * pos.getNumShares() * pos.getContractSize();
}
```

**Simple example.** Run choice `2→1` (Dukascopy daily ExecuteBaseCase) on `LIGHT.CMD/USD` (`contractSize = 100`). A trade buys 1 contract at $80 and exits at $82.
- `output/data.json` records `pnl = (82 - 80) × 1 × 100 = $200`. ✓
- The yearly P&L line prints `2026: $200 (1 trades)`. ✓
- But the `Total P&L` line, computed at [DowATRStrategy.h:374-376](BackTrader/StrategyRunner/DowATRStrategy/DowATRStrategy.h#L374-L376), prints `Total P&L: $2.00`. ✗

The user reading the same screen sees `Total P&L: $2.00` immediately above `2026: $200.00`. The two disagree by exactly `contractSize`.

**Effect.** Every aggregate-statistics line in the base-case console output is wrong by each ticker's `pointValue`. Across a mixed universe (CL=F at 1000, LIGHT.CMD/USD at 100, GC=F at 100, equities at 1) the per-position discrepancies *do not even cancel* in the sum, because each trade is scaled by its own ticker's factor. The minute overload at [DowATRStrategy.h:513-515](BackTrader/StrategyRunner/DowATRStrategy/DowATRStrategy.h#L513-L515) duplicates the bug. The "Top 10 Most/Least Profitable Trades" rankings are also wrong because `pnlList` is populated from the same local recomputation.

**Fix.**
1. Replace the local recomputation with `pos.toJson()["pnl"].get<double>()` — or, better, expose `Position::getPnL()` (the formula already exists at [Position.cpp:270-276](BackTrader/Objects/Position/Position.cpp#L270-L276)) and call it in both blocks.
2. Audit any other place that recomputes P&L locally. `getAllReturns` at [BaseStrategy.cpp:62-75](BackTrader/TradingStrategy/BaseStrategy.cpp#L62-L75) is correct; `getYearlyReturns` at [BaseStrategy.cpp:77-104](BackTrader/TradingStrategy/BaseStrategy.cpp#L77-L104) is correct. Only the two `ExecuteBaseCase` printout blocks need fixing.
3. Add a one-line invariant test: after a base case run, the printed `Total P&L` must equal `sum_of_yearly_pnl_lines`. The current code fails this trivially on any non-unit `contractSize`.

---

### C2 — Silent minute-dispatch fallthrough: intra-day Dukascopy modes run through the daily code path   *(Affects: Dukascopy modes 1, 2, 3, 4)*

**Location:** [Main.cpp:291-295](BackTrader/Main.cpp#L291-L295), [DowATRStrategy.h:177](BackTrader/StrategyRunner/DowATRStrategy/DowATRStrategy.h#L177) and [DowATRStrategy.h:477](BackTrader/StrategyRunner/DowATRStrategy/DowATRStrategy.h#L477) (the unreachable 2-arg overloads), [StrategyRunner.h:68-102](BackTrader/StrategyRunner/StrategyRunner.h#L68-L102) (unreachable 2-arg sweep), [CustomStrategy.cpp:176-356](BackTrader/TradingStrategy/Custom/CustomStrategy.cpp#L176-L356) (unreachable minute-precision exit-price overload)

**The code (dispatch):**
```cpp
// Main.cpp:291-295
if (choice % 2 == 1) {
    ExecuteBaseCase(filteredData, relatedMap);
} else {
    ExecuteAllSweeps(filteredData, relatedMap);
}
```

Both call sites take exactly **two arguments**. The 2-arg-data overloads — `ExecuteBaseCase(data, minuteData, related)` at [DowATRStrategy.h:477](BackTrader/StrategyRunner/DowATRStrategy/DowATRStrategy.h#L477) and `ExecuteAllSweeps(data, minuteData, related)` at [DowATRStrategy.h:177](BackTrader/StrategyRunner/DowATRStrategy/DowATRStrategy.h#L177) — have no caller anywhere in the codebase. `grep -n "ExecuteBaseCase\|ExecuteAllSweeps" BackTrader/Main.cpp` returns only the two lines above.

**Call-path trace (what actually runs when the user picks "1-minute Dukascopy"):**

1. [Main.cpp:148](BackTrader/Main.cpp#L148) advertises `1 - ExecuteBaseCase (1-minute, Dukascopy)`.
2. [Main.cpp:171-173](BackTrader/Main.cpp#L171-L173) loads `OfferBid_SellPriceDataMinuteData.txt` + `OfferAsk_BuyPriceDataMinuteData.txt` into a single `unordered_map<string, StockData> data`. The map is keyed by ticker; each `StockData` contains **minute** OHLCV.
3. [Main.cpp:292](BackTrader/Main.cpp#L292) dispatches `ExecuteBaseCase(filteredData, relatedMap)` — the 2-arg overload at [DowATRStrategy.h:340](BackTrader/StrategyRunner/DowATRStrategy/DowATRStrategy.h#L340).
4. That overload iterates with `strategy->ExecuteStrategy(ticker, stockData)` ([DowATRStrategy.h:355](BackTrader/StrategyRunner/DowATRStrategy/DowATRStrategy.h#L355)) — the **single-data** overload at [CustomStrategy.cpp:6-174](BackTrader/TradingStrategy/Custom/CustomStrategy.cpp#L6-L174), which has no minute slice and no intra-day exit refinement.
5. Result: every minute bar is iterated as if it were a daily bar. ATR period 20 ⇒ 20 minutes of ATR. Lookback 20 ⇒ 20 minutes of price max. `DowBaseCase` is calibrated for daily.

**Unreachable code (will never run as the menu is wired today):**

```cpp
// CustomStrategy.cpp:232-253 — the minute-slice exit refinement
const string sessionStart = currentDate + "T00:00:00";
const string sessionEnd   = currentDate + "T23:59:59";
auto startIt = std::lower_bound(minuteData.date.begin(), minuteData.date.end(), sessionStart);
auto endIt   = std::upper_bound(minuteData.date.begin(), minuteData.date.end(), sessionEnd);
...
for (int idx = initialMinuteIndex; idx <= finalMinuteIndex; idx++) {
    minuteSlice.emplace_back(idx, ...);
}
exitPrice = currentPosition.getExitPrice(currentInstance, futureInstance, minuteSlice);
```

The `getExitPrice` 3-arg variant at [Position.cpp:207-268](BackTrader/Objects/Position/Position.cpp#L207-L268) — the only function that walks the minute bars within a daily session and pins the exit price to the first minute that breaches the stop — is also dead.

**Simple example.** User picks choice `2→1` ("1-minute Dukascopy ExecuteBaseCase"). They expect daily decisions with intra-day fill precision. What runs: every minute bar is treated as a calendar day. A 5-year dataset (~525,000 minute bars per ticker) is iterated as if it were a 525,000-trading-day timeline. The lookback-20 entry filter now requires only 20 *minutes* of consolidation; ATR-20 measures volatility over 20 *minutes* with the per-trade risk dollar amount left unchanged. The strategy semantics are silently rewritten.

**Effect.** Four of the six Dukascopy menu options produce results that have nothing to do with what the menu label implies. There is no warning. No `Returns.txt` annotation. The user has no way to detect the mismatch from the engine's output.

**Three fix options, ranked:**

1. **Recommended — wire up the dual-data path.** Change [Main.cpp:165-182](BackTrader/Main.cpp#L165-L182) to load BOTH a daily series and an intra-day series when choices 1–4 are picked: `OfferBid_SellPriceDataDailyData.txt` becomes the *context* timeline (`data`), and the chosen intra-day file becomes the *refinement* timeline (`minuteData`). Then at [Main.cpp:291-295](BackTrader/Main.cpp#L291-L295), dispatch to the 2-arg overload for choices 1–4 and the 1-arg overload for choices 5–6. This is the only option that delivers the "minute precision" the menu has been promising.

2. **Acceptable — delete the dead overloads.** Remove `ExecuteAllSweeps(data, minuteData, related)`, `ExecuteBaseCase(data, minuteData, related)`, `StrategyRunner::run(..., minuteData)`, `CustomStrategy::ExecuteStrategy(stockName, data, minuteData)`, and `Position::getExitPrice(curr, fut, minuteSlice)`. Relabel the menu so options 1–4 are clearly "1-minute bars driving the daily-frequency strategy machinery" and document that no intra-day refinement is performed. Simpler, honest about current behaviour, smaller maintenance surface.

3. **Minimum — warn and run.** Add a `cerr` at [Main.cpp:291](BackTrader/Main.cpp#L291) stating "minute-precision refinement is not active in this build" whenever choices 1–4 are picked. Buys time but does not fix anything.

---

## Section 2 — Medium Findings

### M1 — Continuous-contract roll gaps affect BOTH yfinance `=F` and Dukascopy series   *(Affects: All modes)*

**Location:** [DownloadDataPython/DownloadData.py](DownloadDataPython/DownloadData.py), [DownloadDataPython/DownloadDailyDataDukas.py](DownloadDataPython/DownloadDailyDataDukas.py), [DownloadDataPython/DownloadMinuteDataDukas.py](DownloadDataPython/DownloadMinuteDataDukas.py)

**The issue.** Both data sources serve *continuous* front-month contracts without back-adjustment. When the exchange rolls (e.g., February crude expires → March takes over), the new front month trades at a different absolute price — there is no overnight market move, but the continuous series shows it as a gap. No `rollDates.json` exists; no back-adjustment is applied in any of the downloader scripts.

**Simple example.** Contango: on 2024-01-24 the February contract closes at $78.00; on 2024-01-25 the March contract takes over and closes at $79.20. Continuous series shows an overnight +$1.20 jump that no trader saw. Backwardation flips the gap negative: continuous open drops $1 overnight. A LONG with stop near $78 reads `currentLow ≤ stopLossPrice` and books a fake loss.

**Effect.** Phantom stops and phantom rallies clustered around roll dates. WTI alone has ~12 roll dates per year; over a 5-year backtest that's ~60 artefact bars per ticker — and the sweep can fixate on them.

**Fix.** *Best:* compute **back-adjusted** continuous contracts client-side (subtract each historical roll's gap from all prior bars). *Acceptable:* maintain `rollDates.json` per ticker and either suppress stop-checks on roll days or force close+reopen. *Minimum:* exclude positions entered within N (3–5) trading days of a known roll from swept stats.

---

### M2 — Per-ticker isolated balance: risk sizing is not coupled to portfolio capital   *(Affects: All modes)*

**Location:** [StrategyRunner.h:54](BackTrader/StrategyRunner/StrategyRunner.h#L54), [StrategyRunner.h:90](BackTrader/StrategyRunner/StrategyRunner.h#L90), [StrategyRunner.h:150](BackTrader/StrategyRunner/StrategyRunner.h#L150), [StrategyRunner.h:197](BackTrader/StrategyRunner/StrategyRunner.h#L197), [DowATRStrategy.h:160](BackTrader/StrategyRunner/DowATRStrategy/DowATRStrategy.h#L160), [DowATRStrategy.h:356](BackTrader/StrategyRunner/DowATRStrategy/DowATRStrategy.h#L356), [DowATRStrategy.h:495](BackTrader/StrategyRunner/DowATRStrategy/DowATRStrategy.h#L495), [Main.cpp:281-285](BackTrader/Main.cpp#L281-L285)

**The code:**
```cpp
// StrategyRunner.h:49-55 — daily 1D sweep loop
for (const auto& [ticker, stockData] : data) {
    ...
    specificStrategy->ExecuteStrategy(ticker, stockData);
    specificStrategy->setBalance(initialBalance);   // reset between tickers
}
```

`initialBalance` is whatever was passed into the `StrategyRunner` constructor. At every call site that value is `dowBase.balance = 1000.0` (see e.g. [DowATRStrategy.h:26](BackTrader/StrategyRunner/DowATRStrategy/DowATRStrategy.h#L26), [DowATRStrategy.h:56](BackTrader/StrategyRunner/DowATRStrategy/DowATRStrategy.h#L56), [DowATRStrategy.h:66](BackTrader/StrategyRunner/DowATRStrategy/DowATRStrategy.h#L66)). So each ticker starts with a fresh $1,000 paper account regardless of what came before.

Note also [Main.cpp:281-285](BackTrader/Main.cpp#L281-L285):
```cpp
double initial_balance = static_cast<double>(dataSize) * dowBase.balance;
{
    filesystem::create_directories("../output");
    ofstream configFile("../output/configuration.json");
    configFile << "{\n  \"initial_balance\": " << initial_balance << "\n}\n";
}
```
This `numStocks × 1000` value is **only** consumed by `Stats.py` to denominate the equity curve. It is **not** passed into the C++ engine — the per-ticker reset still uses `dowBase.balance`. So `configuration.json` is a Stats-side artefact, not a portfolio-pool implementation.

**Simple example.** Choice `1→2` (yfinance ExecuteAllSweeps) on 32 commodity tickers. `CL=F` finishes its history with $1,200 in its paper account. The engine resets to $1,000 before `GC=F` runs. Across 32 tickers, the sweep reports the sum of 32 independent $1,000 paper accounts. At 2% risk = $20/trade, every trade risks $20 regardless of how many tickers happen to be in a position concurrently. If real capital is $10,000 deployed across 32 tickers, simultaneous positions in 30 tickers represent 30 × $20 = $600 = 6% of real portfolio at risk; the engine has no representation of that aggregate exposure.

**Effect.** Per-ticker P&Ls are individually consistent. Cross-ticker aggregates (`Total P&L`, yearly Returns.txt, Stats.py equity curve) are the sum of independent $1,000-paper runs. Sizing assumes infinite re-funding between tickers — not a single capital pool. A user planning a real $10,000 deployment cannot read the printed equity directly without rescaling. Additionally, `strategy->getBalance()` after a sweep returns the last alphabetically-iterated ticker's ending balance, which is *not* a portfolio total.

**Fix.**
1. Document the ticker-isolated execution model in [CLAUDE.md](CLAUDE.md) — explicitly state "P&L is the sum of independent $1,000 paper runs."
2. (Architectural) Introduce a portfolio-level risk budget: a `portfolioBalance` parameter where each ticker is allocated `portfolioBalance / numTickers` as its starting paper balance, or a global risk-budget cap that prevents a per-ticker trade from being sized as if it were the only ticker. Replace the `setBalance(initialBalance)` reset accordingly.

---

### M3 — Yearly P&L attribution lumps multi-year trades into the exit year   *(Affects: All modes)*

**Location:** [BaseStrategy.cpp:77-104](BackTrader/TradingStrategy/BaseStrategy.cpp#L77-L104)

**The code:**
```cpp
map<int, vector<double>> BaseStrategy::getYearlyReturns(){
    int n = this->closedPositions.size();
    map<int, vector<double>> returns;

    auto addToYear = [&](int year, double amount) {
        returns[year].push_back(amount);
    };

    for (int i = 0; i < n; i++){
        const Position &pos = this->closedPositions[i];
        ...
        int exitYear  = stoi(sDate.substr(0, 4));

        double totalProfit = 0;
        if (pos.getPositionType().getPositiontype() == "LONG"){
            totalProfit = (pos.getSellPrice() - pos.getPurchasePrice()) * pos.getNumShares() * pos.getContractSize();
        }
        ...
        addToYear(exitYear, totalProfit);
    }
    return returns;
}
```

The lambda `addToYear` is invoked exactly once per position — with `exitYear`. There is no purchase-year split.

**Simple example.** A LONG opens 2023-11-15 at $80 and closes 2024-02-10 at $90 with `contractSize = 100`. Full P&L = `10 × 1 × 100 = $1000`. The entire $1000 is attributed to **2024**. **2023 receives $0** from this trade despite the position being open for 6 weeks of 2023.

**Effect.** Per-year totals (`Returns.txt`, the yearly print at [DowATRStrategy.h:429-434](BackTrader/StrategyRunner/DowATRStrategy/DowATRStrategy.h#L429-L434), and any `Stats.py` yearly-Sharpe / per-year-drawdown computation) are distorted by year-straddling trades. Multi-year averages converge to the right answer; *per-year* peaks and drawdowns do not. Bias is largest for trend-following systems whose winners are long-held.

**Fix.** Mark-to-market at year boundaries. For each closed position straddling a 31-Dec, compute `(close_of_last_bar_of_year_Y - purchasePrice) * numShares * contractSize` (LONG) for year `Y` and attribute the remainder to year `Y+1`. Requires injecting the daily bar series into the aggregator or storing intra-position year-end snapshots when a position is held over 31-Dec.

---

### M4 — Stop-loss *trigger* is daily-resolution; minute mode (when wired) would refine fill price only, not fill time   *(Affects: Dukascopy modes 1, 2, 3, 4 — moot today per C2)*

**Location:** [CustomStrategy.cpp:191-261](BackTrader/TradingStrategy/Custom/CustomStrategy.cpp#L191-L261)

The minute-overload `ExecuteStrategy` (unreachable today per C2) is structured so the minute slice is consulted **after** `context->shouldSellTrade(currentInstance)` returns true; `isStopLoss` is set from the daily `currentLow/currentHigh`. **Daily-low aggregation correctly brackets all minute extremes**, so the trigger doesn't *miss* intraday stops — but it also can't fire *earlier* than end-of-day. The intended minute mode therefore delivers **price precision, not time precision**: the fill price is realistic, but `sellDate` is bucketed to a calendar day.

**Effect.** No P&L error once C2 is fixed and the minute path runs. Holding-period histograms, intraday max-adverse-excursion, multi-trigger-in-one-day handling, and any minute-resolution attribution cannot be reconstructed from the output.

**Fix.** *Cheap:* add an `exitMinuteTimestamp` field on `Position` populated by the slice scan. *Real:* push the entire decision loop down to minute resolution — iterate minute bars inside the day, run `shouldSell` on each, and use the daily bar only as a context-update boundary.

---

### M5 — Forced final-bar exit can produce a zero-P&L close on positions opened on the penultimate bar   *(Affects: All modes)*

**Location:** [CustomStrategy.cpp:144-170](BackTrader/TradingStrategy/Custom/CustomStrategy.cpp#L144-L170) (daily overload), [CustomStrategy.cpp:327-351](BackTrader/TradingStrategy/Custom/CustomStrategy.cpp#L327-L351) (minute overload)

**The code:**
```cpp
if (!this->getPosition().getIsClosed()){
    Position &lastPosition = this->getPosition();
    // Use the last bar's open to match the next-bar-open fill convention used throughout
    // the loop. data.close.back() would be a different convention from every other exit.
    lastPosition.setSellPrice(data.open.back());
    lastPosition.setSellDate(data.date.back());
    ...
}
```

The fix from the 2026-05-16 audit (M7) landed cleanly — the convention now matches the rest of the loop. But the loop body runs `i = 1 .. size-2`, so a position whose entry triggers on `i = size - 2` fills at `futureInstance.open = data.open[size-1] = data.open.back()`. The forced exit at line 148 then uses **the same price**, yielding `P&L = 0` minus the round-trip friction. In live trading the position would carry exposure into the next bar; the engine instead force-closes at the entry fill, dropping the friction on the floor.

**Effect.** Small per-backtest bias on positions opened on the penultimate bar. Across the sweep the same bar is hit by every parameter combo, so the bias does not cancel across configs.

**Fix.** Either (a) restrict the entry rule to `i < size - 2` so positions cannot be opened on the penultimate bar, or (b) when force-closing, use `data.close.back()` for any position that was opened on the very last reachable bar — accepting the convention break is honest about what data is available.

---

### M6 — yfinance pipeline silently defaults `contractSize=1.0` and `frictionPerRoundTrip=0.0`   *(Affects: yfinance modes 1, 2)*

**Location:** [DownloadData.py:85-99](DownloadDataPython/DownloadData.py#L85-L99) (looks up `DukascopyTickers.json`), [DownloadData.py:145-152](DownloadDataPython/DownloadData.py#L145-L152) (falls back to `1.0` / `0.0` when not found)

**The code:**
```python
# DownloadData.py:145-152
cs = contract_sizes.get(stock_name)
if cs is None:
    print(f"Warning: no contractSize for {stock_name} in DukascopyTickers.json — defaulting to 1.0")
    cs = 1.0
fr = friction_costs.get(stock_name, 0.0)
file.write(f"Stock: {stock_name}\n")
file.write(f"ContractSize:\n{cs}\n")
file.write(f"FrictionPerRoundTrip:\n{fr}\n")
```

The yfinance downloader writes one shared `data.txt` and looks up *each ticker* in `DukascopyTickers.json`. yfinance tickers are `CL=F`, `GC=F`, etc. — the file is keyed by Dukascopy symbols (`LIGHT.CMD/USD`, `XAU/USD`, …). The lookup misses for every yfinance symbol, every per-ticker write logs a warning, and the file is populated with `ContractSize: 1.0`, `FrictionPerRoundTrip: 0.0`. The C++ side then performs sizing and P&L with the post-fix formulas but the input is functionally the pre-fix world.

**Simple example.** Choice `1→2` (yfinance ExecuteAllSweeps) on `CL=F` at $80, ATR=$2. Sizer at 2% risk = $20 on $1000 balance, `riskPerContract = 4 × 1 = $4`, `numContracts = floor(20/4) = 5`. P&L at $82 exit: `(82 - 80) × 5 × 1 = $10`. CME crude is 1000 bbl/contract — the real P&L is $10,000, the engine reports $10. The C1 fix is structurally in place but defeated by the input data.

**Effect.** Both yfinance modes (1 and 2) silently produce results scaled wrong by each ticker's true `pointValue`. Sweeps rank configs against unit-scaled P&L. Frictions are zero, so the C2 fix is also defeated on this path.

**Fix.** Author `DownloadDataPython/yfinanceTickers.json` mirroring the Dukascopy file (keyed by `=F` symbols) with CME-spec `contractSize`, `frictionPerRoundTrip`, and any other future fields. Change [DownloadData.py:83](DownloadDataPython/DownloadData.py#L83) to read both files and merge, or to pick one based on the active download target. Until that exists, the warning at line 147 should be promoted from `print` to a hard error that drops the ticker — silently writing `1.0` is the worst of both options.

---

### M7 — yfinance daily data uses unadjusted close   *(Affects: yfinance modes 1, 2)*

**Location:** [DownloadData.py:153-156](DownloadDataPython/DownloadData.py#L153-L156)

The yfinance pipeline writes raw `Open/Close/High/Low`, not `Adj Close`. For commodity-futures `=F` tickers the practical impact is negligible (no dividends, no splits). But if the universe is ever expanded to equities, unadjusted close means every ex-dividend date becomes a phantom price drop that can falsely trigger stops, and splits cause discontinuities.

**Fix.** Document the assumption near [DownloadData.py:153](DownloadDataPython/DownloadData.py#L153). If equities are ever added, switch to `Adj Close` for return calculation while keeping raw `Open/High/Low/Close` for fill prices (or use a roll-adjusted continuous series equivalent).

---

## Section 3 — Low Findings

| ID | Affects | Summary |
|---|---|---|
| **L1** | All | ATR snapshot is one bar optimistic — `processNewData` is called at the end of each loop iteration ([CustomStrategy.cpp:141](BackTrader/TradingStrategy/Custom/CustomStrategy.cpp#L141)), then again before sizing the `i+1` fill ([CustomStrategy.cpp:115](BackTrader/TradingStrategy/Custom/CustomStrategy.cpp#L115)). Tiny smoothed-average bias. |
| **L2** | All | `WindowStatistics` slope / t-statistic can become NaN on small windows ([WindowStatistics.cpp:13-65](BackTrader/Functions/WindowStatistics.cpp#L13-L65)). Moot today because every consumer (MACD, trendline significance) is commented out in `DowContext`. Resurrect if those filters are re-enabled. |
| **L3** | Dukascopy 1/2/3/4 | `binarySearchDate` returns -1; `max(dateIndex, 1)` clamp at [CustomStrategy.cpp:191](BackTrader/TradingStrategy/Custom/CustomStrategy.cpp#L191) is structurally dead because the explicit `return` at 185-189 catches the -1 case first. Now doubly unreachable because the entire minute overload is dead code per C2. |
| **L4** | Dukascopy 1/2/3/4 | `minuteSlice.emplace_back(idx, ...)` stores the *global* minute-array index, not the slice position ([CustomStrategy.cpp:247-251](BackTrader/TradingStrategy/Custom/CustomStrategy.cpp#L247-L251)). `Position::getExitPrice` never reads this field, but a future `toJson()` on a minute instance will emit the wrong index. Moot until C2 is fixed. |
| **L5** | Dukascopy 1/2/3/4 | Per-event `cerr` warnings on missing minute starts and empty slices ([CustomStrategy.cpp:186-189](BackTrader/TradingStrategy/Custom/CustomStrategy.cpp#L186-L189), [CustomStrategy.cpp:236-240](BackTrader/TradingStrategy/Custom/CustomStrategy.cpp#L236-L240)) but no aggregate counter. Add `dailyFallbackCount` and `minuteExitCount` per ticker; print a summary at end of `ExecuteStrategy` (e.g., "minute precision used on 412/488 exits"). Currently moot per C2. |
| **L6** | All | SMAMACD / EMAMACD compute the signal as an SMA / EMA **of close**, not as an EMA of MACD itself ([SMAMACD.cpp:22-26](BackTrader/Indicators/SMAMACD/SMAMACD.cpp#L22-L26)). Non-standard formula. Currently moot — the MACD filter is commented out in `DowContext`. Field name will mislead the next reader. |
| **L7** | All | Stop-loss level is computed using bar `i+1`'s open AND ATR over bars `0..i` ([ATRPositionSize.cpp:22](BackTrader/PositionSizing/ATRPositionSize/ATRPositionSize.cpp#L22) called via [CustomStrategy.cpp:115,122](BackTrader/TradingStrategy/Custom/CustomStrategy.cpp#L115)). Defensible in backtest (in live you'd set the stop after the fill); worth documenting so the convention is explicit. |
| **L8** | All | `DataReader::ReadData` accepts `Stock:` blocks where `ContractSize:` is present but `FrictionPerRoundTrip:` is absent, falling back to `0.0` silently ([DataReader.cpp:20](BackTrader/DataReader/DataReader.cpp#L20)). For futures this is wrong; the missing field should be a hard error or at minimum a `cerr` warning per ticker. Today it is silent. |

---

## Section 4 — What Is Sound

| Area | Why it is sound |
|---|---|
| Entry fill timing | Signal computed at bar `i` close; fill at bar `i+1` open. Standard backtest convention; no look-ahead. |
| Trailing stop ratchet | LONG: monotone increasing; SHORT: monotone decreasing ([ATRPositionSize.cpp:70-92](BackTrader/PositionSizing/ATRPositionSize/ATRPositionSize.cpp#L70-L92)). |
| ATR formula | Textbook: `max(high - low, abs(high - prevClose), abs(low - prevClose))`. |
| Per-ticker state-reset *mechanic* | `setBalance` + `context->clear()` + `sizer->clear()` between tickers prevents state leakage ([StrategyRunner.h:54](BackTrader/StrategyRunner/StrategyRunner.h#L54), [CustomStrategy.cpp:172-173](BackTrader/TradingStrategy/Custom/CustomStrategy.cpp#L172-L173)). The ticker-isolated *semantics* are M2; the reset *mechanic* itself is sound. |
| SHORT P&L accounting | Entry debit `-numShares × purchasePrice × contractSize` plus exit credit `numShares × (2 × purchasePrice − sellPrice) × contractSize` nets to `numShares × (purchasePrice − sellPrice) × contractSize`. ([CustomStrategy.cpp:92-93](BackTrader/TradingStrategy/Custom/CustomStrategy.cpp#L92-L93)) |
| `MacroFeatures` look-ahead | All computations strictly trailing (≤ current date); no peek at future bars. |
| `TrendIdentifier` / `TrendLineTracker` | Extrema labelled with strictly trailing logic. Not consumed by the current entry rule but the code itself is correct. |
| `WindowStatistics` numerical guards | Division-by-zero guard at WindowStatistics.cpp slope path; R² clamped to `[-1, 1]`. |
| **Position::toJson and Position::getExitPrice** | Now multiply by `contractSize` ([Position.cpp:270-276](BackTrader/Objects/Position/Position.cpp#L270-L276)). `getExitPrice` correctly returns `currentOpen` if the open gapped past the stop, the stop level otherwise, and `futureData.open` when not stopped. ([Position.cpp:168-205](BackTrader/Objects/Position/Position.cpp#L168-L205)) |
| **`ATRPositionSize` contract-aware sizing** | Risk-per-contract and the balance cap both use `data.contractSize` ([ATRPositionSize.cpp:15-44](BackTrader/PositionSizing/ATRPositionSize/ATRPositionSize.cpp#L15-L44)); `std::floor` enforces whole contracts ([ATRPositionSize.cpp:30](BackTrader/PositionSizing/ATRPositionSize/ATRPositionSize.cpp#L30) and `:44`). |
| **Friction deduction at close** | Round-trip friction subtracted at every position close ([CustomStrategy.cpp:88-93](BackTrader/TradingStrategy/Custom/CustomStrategy.cpp#L88-L93), [:159-164](BackTrader/TradingStrategy/Custom/CustomStrategy.cpp#L159-L164), [:279-284](BackTrader/TradingStrategy/Custom/CustomStrategy.cpp#L279-L284), [:340-345](BackTrader/TradingStrategy/Custom/CustomStrategy.cpp#L340-L345)). Per-ticker `frictionPerRoundTrip` is loaded from `DukascopyTickers.json` (Dukascopy path only — yfinance is M6). |
| **Ask/Bid sidedness** | LONG entry uses Ask, SHORT exit / trailing stop uses Ask; LONG exit and SHORT entry use Bid (the canonical fields). Implemented at [CustomStrategy.cpp:42-69](BackTrader/TradingStrategy/Custom/CustomStrategy.cpp#L42-L69) and [:118-121](BackTrader/TradingStrategy/Custom/CustomStrategy.cpp#L118-L121). |
| **Sell-date / sell-price intraday convention** | `sellDate = currentDate` when the daily low triggered the stop; `sellDate = futureDate` when exiting at the next bar's open ([CustomStrategy.cpp:72-76](BackTrader/TradingStrategy/Custom/CustomStrategy.cpp#L72-L76)). Old 2026-05-16 M2 finding is closed. |
| **`DataReader::ReadData` vector-length validation** | Validates `open.size() == close == high == low == volume == date.size()` and drops the ticker with a `cerr` if not ([DataReader.cpp:29-38](BackTrader/DataReader/DataReader.cpp#L29-L38)). Old L2 finding is closed. |

---

## Section 5 — Recommended Order of Remediation

1. **C1 — route printout aggregation through `Position::toJson` or a new `Position::getPnL`.** Without this, the user-visible base-case statistics block disagrees with the JSON and the per-year totals printed five lines below it. A trivial code change with very high trust impact.
2. **C2 — wire up minute dispatch (or delete the dead overloads).** Four of six menu options currently advertise a feature the engine does not deliver. Recommended path: load daily + chosen intra-day file when modes 1–4 are picked and dispatch to the 2-arg overload.
3. **M6 — author `yfinanceTickers.json`.** Without it the entire yfinance path silently defeats the C1+C2 fixes by feeding `contractSize=1.0` and `frictionPerRoundTrip=0.0` to a correctly-coded engine.
4. **M1 — back-adjust continuous contracts** at download time. Removes phantom roll-day stops in both yfinance and Dukascopy paths.
5. **M2 — portfolio-level risk sizing** (plus the [CLAUDE.md](CLAUDE.md) documentation note). Required for any user planning to deploy real capital across the full universe.
6. **M3 — split year-straddling P&L** for per-year statistics.
7. **M5, L8** — short cleanup items: avoid penultimate-bar entries; promote missing `FrictionPerRoundTrip` from silent to warned.

Remaining items (M4, M7, L1–L7) are acceptable to defer.
