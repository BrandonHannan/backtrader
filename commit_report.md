# Logic-Soundness Audit — Full BackTrader Trading System

**Scope:** the code paths exercised by all four execution modes in [Main.cpp:137-377](BackTrader/Main.cpp#L137-L377) — daily-only choices `1` (`ExecuteBaseCase`) and `2` (`ExecuteAllSweeps`) running on yfinance `data.txt`, and minute-precision choices `3` and `4` running on Dukascopy `DailyData.txt` + `MinuteData.txt`.
**Method:** static review of the order-execution loop, sizing, exit logic, aggregation, and data-realism assumptions. No live backtest run.
**Date of review:** 2026-05-16.
**Grading:** Critical / Medium / Low by impact on the trustworthiness of the reported P&L. Each finding is tagged with which paths it affects.

> The long-only behaviour and the simplified `close > maxPrice` entry rule in [DowContext::shouldExecuteTrade](BackTrader/TradingContext/DowContext/DowContext.cpp#L122-L163) are **intentional design choices** and are **not** flagged as findings.

---

## Executive Verdict

The BackTrader system **builds and runs end-to-end in all four modes**, and the core mechanics — next-bar-open fills, trailing ATR stops, minute-bar exit-price refinement, and per-ticker state isolation — are wired correctly. However, **dollar quantities reported by the engine are not in real dollars**: futures contract multipliers are hard-coded to 1, no frictions are modelled, and yfinance/Dukascopy continuous-front series carry roll gaps that fire phantom stops. Additionally, the engine treats each ticker as an independent $1,000 paper account, which is sound mechanically but means the risk-per-trade sizing is **not coupled to the user's total capital**. Two critical fixes (contract multipliers, commission/slippage) and one architectural fix (portfolio-level risk sizing) are required before any sweep result can be interpreted economically.

**Counts:** Critical: 2 · Medium: 8 · Low: 8 · Sound: 9.

---

## Section 1 — Critical Findings

### C1 — Futures contract multipliers treated as 1.0   *(Affects: All modes)*

**Location:** [Position.cpp:263-268](BackTrader/Objects/PositionType/Position.cpp#L263-L268), [ATRPositionSize.cpp:14-24](BackTrader/PositionSizing/ATRPositionSize/ATRPositionSize.cpp#L14-L24), [CustomStrategy.cpp:74,108,252-256](BackTrader/TradingStrategy/Custom/CustomStrategy.cpp#L108), [BaseStrategy.cpp:77-99](BackTrader/TradingStrategy/BaseStrategy.cpp#L77-L99)

**The code (P&L, used by every print/CSV path):**
```cpp
// Position.cpp:263-268
if (positionType.getPositiontype() == "LONG") {
    pnl = (sellPrice - purchasePrice) * numShares;
} else if (positionType.getPositiontype() == "SHORT") {
    pnl = (purchasePrice - sellPrice) * numShares;
}
```

**The code (sizing):**
```cpp
// ATRPositionSize.cpp:14-24
stopLossPrice = data.open - (atr.getATR() * this->ATRMultiplier);
riskPerShare  = data.open - stopLossPrice;
if (riskPerShare <= 0) { return PositionPriceInfo(0.0, 0.0); }
double result = dollarRisk / riskPerShare;
double maxSharesByBalance = balance / data.open;
if (result > maxSharesByBalance) { result = maxSharesByBalance; }
```

**Simple example.** Start with `$1,000` (the `DowBaseCase.balance` default). The engine downloads either `CL=F` (yfinance, modes 1–2) or `LIGHT.CMD/USD` (Dukascopy, modes 3–4) at `$80/barrel`. With `ATR=$2` and a `2×` multiplier, the stop is at `$76`, risk-per-share is `$4`, and at 2% risk = `$20` the engine sizes `$20 / $4 = 5 "shares"`. Crude moves to `$82` and the engine books P&L = `($82 − $80) × 5 = $10`.

But every `=F` / `LIGHT.CMD/USD` symbol is a **futures contract**, not a barrel of crude:
- **CME CL spec — 1 contract = 1,000 barrels.** 5 "shares" is 5 contracts = 5,000 barrels. A `$2` move = **`$10,000`** P&L, not `$10`. Reported P&L is wrong by **1,000×**.
- **Dukascopy CFD spec — ~100 barrels per unit.** Same trade is 5 CFDs × 100 × `$2` = **`$1,000`**. Wrong by 100×.

This affects **every dollar print and every CSV column** the engine produces — including the `Total P&L:` line printed by [DowATRStrategy.h:415](BackTrader/StrategyRunner/DowATRStrategy/DowATRStrategy.h#L415) in the base-case modes and the per-year aggregates in `Returns.txt`. The `maxSharesByBalance = balance / data.open` leverage cap is also wrong for futures, because the cost to enter a futures contract is *margin* (~5–10% of notional), not the full notional — the cap is simultaneously too tight (rejects sizes the broker would allow) and too loose (allows notional vastly exceeding the cash balance).

**Effect.** Every dollar quantity in `Returns.txt`, the printed base-case statistics block ([DowATRStrategy.h:405-434](BackTrader/StrategyRunner/DowATRStrategy/DowATRStrategy.h#L405-L434) and its minute mirror), the per-position JSON in `output/data.json`, and any downstream `Stats.py` CSV is scaled by an unknown per-ticker factor. Cross-ticker aggregation sums these mis-scaled P&Ls, so a sweep result that "wins" `$X` is meaningless until each ticker's multiplier is applied.

**Fix.**
1. Add `output/contractSpecs.json` produced by the download scripts: `{ "CL=F": { "pointValue": 1000.0, "tickSize": 0.01, "initialMargin": 0.10 }, "LIGHT.CMD/USD": { "pointValue": 100.0, "tickSize": 0.01, "initialMargin": 0.05 }, ... }`. Source values from CME spec sheets and Dukascopy CFD docs.
2. Load it in [Main.cpp](BackTrader/Main.cpp) next to `relatedMap` and inject into `ATRPositionSize` and `Position`.
3. Update the two P&L formulas in [Position.cpp:263-268](BackTrader/Objects/PositionType/Position.cpp#L263-L268) **and** the duplicate aggregation in [BaseStrategy.cpp:77-99](BackTrader/TradingStrategy/BaseStrategy.cpp#L77-L99) to `(sellPrice - purchasePrice) * numShares * pointValue` and its SHORT mirror.
4. Update sizing to `riskPerShare = (data.open - stopLossPrice) * pointValue`; replace the balance cap with a margin check: `notional = numShares * data.open * pointValue; margin = notional * initialMargin; shrink numShares while margin > balance`.
5. Round `numShares` to whole contracts (futures are not fractional).

---

### C2 — No commission, slippage, or bid-ask spread; the sweep overfits to a frictionless world   *(Affects: All modes)*

**Location:** [CustomStrategy.cpp:108,252-256](BackTrader/TradingStrategy/Custom/CustomStrategy.cpp#L108) (and the daily-overload mirror at 74), [DownloadMinuteDataDukas.py:21](DownloadDataPython/DownloadMinuteDataDukas.py#L21), [DownloadData.py:130-138](DownloadDataPython/DownloadData.py#L130-L138)

**The code:**
```cpp
// CustomStrategy.cpp:108 — entry debit
this->addToBalance(-1 * (newPosition.getNumShares() * newPosition.getPurchasePrice()));

// CustomStrategy.cpp:252-256 — exit credit
this->addToBalance(currentPosition.getNumShares() * currentPosition.getSellPrice());                       // LONG
this->addToBalance(currentPosition.getNumShares() *
                   (currentPosition.getPurchasePrice() + (currentPosition.getPurchasePrice() - currentPosition.getSellPrice())));   // SHORT
```

**Simple example.** A crude-oil sweep produces a config that closes 200 round-trip trades and reports **`+$5,000` net**. Now apply realistic costs:
- **Commission:** ~`$4` per round trip on CME crude.
- **Half-spread:** crude tick is `$0.01` = `$10/contract`; typical spread is 1 tick × 2 sides = `$20` round trip.
- **Slippage** on a market order against a thin 15-minute bar: another `$10–20` per side.

Total round-trip cost ≈ `$40–60`. 200 trades × `$50` ≈ **`$10,000` of trading costs**. The `+$5,000` backtest becomes **`−$5,000` in reality** — the sign of the result flips.

Compounding this in the minute path: Dukascopy data is **bid-only** ([DownloadMinuteDataDukas.py:21](DownloadDataPython/DownloadMinuteDataDukas.py#L21), `OFFER_SIDE_BID`). Every LONG entry "at open" actually transacted at the (higher) ask; every LONG exit "at open" actually received the (lower) bid. The engine systematically captures the bid-bid spread it never paid. In the daily yfinance path, the OHLC are single auction/last-trade prints — neither bid nor ask — so the same idealised-mid bias applies.

The cumulative harm is the **sweep itself**. `ExecuteAllSweeps` ranks dozens of `(lookback, ATRPeriod, ATRMultiplier, riskAmount)` configurations and picks the best by P&L. High-turnover configurations win on a frictionless ledger and lose first when frictions are added. **The sweep selects exactly the configurations that are most fragile to real costs.**

**Effect.** Reported equity curves, Sharpes, and yearly returns are systematically biased high. The *ranking* among swept configs is wrong: winners over-rotate to high-turnover regimes.

**Fix.**
1. Add per-ticker `commissionPerSide`, `slippageTicksPerSide`, and `halfSpreadTicks` fields to `contractSpecs.json` (same file as C1).
2. At every position close, deduct `(commission + (slippageTicks + halfSpreadTicks) * tickSize * pointValue) * 2 * numShares` from balance.
3. Add a comment at [CustomStrategy.cpp:108](BackTrader/TradingStrategy/Custom/CustomStrategy.cpp#L108) naming the frictions and where to tune them.

---

## Section 2 — Medium Findings

### M1 — Stop-loss *trigger* is daily-resolution even in minute mode   *(Affects: Minute (3, 4))*

**Location:** [CustomStrategy.cpp:197,228-233](BackTrader/TradingStrategy/Custom/CustomStrategy.cpp#L197-L233)

The minute slice is only consulted **after** `context->shouldSellTrade` (which receives the daily `currentInstance`) returns true, and `isStopLoss` is set from the daily `currentLow/currentHigh`. **Daily-low aggregation correctly brackets all minute extremes**, so the trigger doesn't *miss* intraday stops — but it also can't fire *earlier* than end-of-day. Minute mode therefore delivers **price precision, not time precision**: the fill price is realistic, but the `sellDate` is still bucketed to a calendar day.

**Effect.** No P&L error. Holding-period histograms, intraday max-adverse-excursion, multi-trigger-in-one-day handling, and any minute-resolution attribution cannot be reconstructed from the output.

**Fix.** Cheap: add an `exitMinuteTimestamp` field on `Position` populated by the slice scan. Real: push the entire decision loop down to minute resolution — iterate minute bars inside the day, run `shouldSell` on each, and use the daily bar only as a context-update boundary.

---

### M2 — Sell-date / sell-price inconsistency on non-gap exits   *(Affects: All modes)*

**Location:** [CustomStrategy.cpp:50-61](BackTrader/TradingStrategy/Custom/CustomStrategy.cpp#L50-L61) (daily overload), [228-239](BackTrader/TradingStrategy/Custom/CustomStrategy.cpp#L228-L239) (minute overload)

When the daily low touches the stop but the open did not gap through, `isStopLoss=true` so `setSellDate(currentDate)` — but `Position::getExitPrice` may return `futureData.open`. The recorded date says "today"; the fill is "tomorrow." P&L math is unaffected; only the date field is off.

**Fix.** After `getExitPrice` returns, if the value equals `futureData.open`, override `setSellDate(futureDate)`.

---

### M3 — Silent minute-fallback diagnostics   *(Affects: Minute (3, 4))*

**Location:** [CustomStrategy.cpp:158-161,209-213](BackTrader/TradingStrategy/Custom/CustomStrategy.cpp#L158-L213)

Per-event `cerr` warnings now fire on missing minute starts and empty slices, but there is no aggregate counter. A holiday range or partial dataset silently degrades to daily resolution one bar at a time and the user cannot tell from the output how often minute precision was used.

**Fix.** Maintain `dailyFallbackCount` and `minuteExitCount` per ticker; print a summary at end of `ExecuteStrategy` (e.g., "minute precision used on 412/488 exits").

---

### M4 — Continuous-contract roll gaps affect BOTH yfinance `=F` and Dukascopy series   *(Affects: All modes)*

**Location:** [DownloadData.py](DownloadDataPython/DownloadData.py), [DownloadDailyDataDukas.py](DownloadDataPython/DownloadDailyDataDukas.py), [DownloadMinuteDataDukas.py](DownloadDataPython/DownloadMinuteDataDukas.py)

**The issue.** Both data sources serve *continuous* front-month contracts without back-adjustment. When the exchange rolls (e.g., February crude expires → March takes over), the new front month trades at a different absolute price — there is no overnight market move, but the continuous series shows it as a gap.

**Simple example.** Contango: on 2024-01-24 the February contract closes at `$78.00`; on 2024-01-25 the March contract takes over and closes at `$79.20`. Continuous series shows an overnight `+$1.20` jump that no trader saw. Backwardation flips the gap negative: continuous open drops `$1` overnight. A LONG with stop near `$78` reads `currentLow ≤ stopLossPrice` and books a fake loss.

**Effect.** Phantom stops and phantom rallies clustered around roll dates. WTI alone has ~12 roll dates per year; over a 5-year backtest that's ~60 artefact bars per ticker — and the sweep can fixate on them.

**Fix.** *Best:* compute **back-adjusted** continuous contracts client-side (subtract each historical roll's gap from all prior bars). *Acceptable:* maintain `rollDates.json` per ticker and either suppress stop-checks on roll days or force close+reopen. *Minimum:* exclude positions entered within N (3–5) trading days of a known roll from swept stats.

---

### M5 — Per-ticker isolated balance: risk sizing is not coupled to portfolio capital   *(Affects: All modes)*

**Location:** [StrategyRunner.h:54,90,150,197](BackTrader/StrategyRunner/StrategyRunner.h#L54), [DowATRStrategy.h:160,323,356,495](BackTrader/StrategyRunner/DowATRStrategy/DowATRStrategy.h#L495)

**The code:**
```cpp
for (const auto& [ticker, stockData] : data) {
    // ... validation ...
    specificStrategy->ExecuteStrategy(ticker, stockData);
    specificStrategy->setBalance(initialBalance);   // <-- reset between tickers
}
```

**Simple example.** Run choice 2 with 32 commodity tickers, base balance `$1,000`. `CL=F` finishes its full history with `$1,200`. The engine **resets to `$1,000`** before `GC=F` starts. Each ticker effectively runs as an independent `$1,000` paper account; the closed-positions list accumulates across tickers, but the *balance the sizer sees during each ticker's run is always reset to the base case*.

This means **risk-per-trade is constant across tickers and decoupled from real portfolio capital**: at 2% risk on `$1,000`, every trade risks `$20`, no matter how many tickers are active. If a user really has `$10,000` to deploy and the strategy happens to open positions in 5 tickers on the same day, the risk *as configured* is 5 × `$20` = `$100` (1% of real portfolio) — fine. If it happens to open in 30, it's `$600` (6% of real portfolio) — still moderate. But the engine's *internal* sizing never reflects this multi-ticker exposure; each ticker thinks it's the only one.

There is a related footgun: `strategy->getBalance()` after a run returns the **last alphabetically-iterated ticker's ending balance**, not a portfolio total. The base-case modes do print `Total P&L:` correctly aggregated from positions ([DowATRStrategy.h:415](BackTrader/StrategyRunner/DowATRStrategy/DowATRStrategy.h#L415)), so users who read that line are fine — but any future caller of `getBalance()` will be misled.

**Effect.** Per-ticker P&Ls are individually correct. Cross-ticker aggregates (`Total P&L:`, yearly returns in `Returns.txt`) are correct as a sum of independent `$1,000`-paper runs. But the **sizing assumes infinite re-funding** between tickers — it is not a representation of a single capital pool. A user planning a real `$10,000` deployment cannot read the printed equity directly without rescaling.

**Fix.**
1. Document the ticker-isolated execution model in [CLAUDE.md](CLAUDE.md) — explicitly state "P&L is the sum of independent `$1,000` paper runs."
2. (Architectural) Introduce a portfolio-level risk budget: a `portfolioBalance` parameter where each ticker is allocated `portfolioBalance / numTickers` as its starting paper balance, or a global risk-budget cap that prevents a per-ticker trade from being sized as if it were the only ticker. Replace the `setBalance(initialBalance)` reset accordingly.

---

### M6 — Yearly P&L attribution lumps multi-year trades into the exit year   *(Affects: All modes)*

**Location:** [BaseStrategy.cpp:77-99](BackTrader/TradingStrategy/BaseStrategy.cpp#L77-L99)

**The code:**
```cpp
map<int, vector<double>> BaseStrategy::getYearlyReturns(){
    int n = this->closedPositions.size();
    map<int, vector<double>> returns;
    for (int i = 0; i < n; i++){
        int year = stoi(this->closedPositions[i].getSellDate().substr(0, 4));   // <-- exit year only
        double profit = 0;
        if (positionType == "LONG"){
            profit = (sellPrice - purchasePrice) * numShares;
        }
        else if (positionType == "SHORT"){
            profit = (purchasePrice - sellPrice) * numShares;
        }
        returns[year].push_back(profit);
    }
    return returns;
}
```

**Simple example.** A LONG opens 2023-11-15 at `$80` and closes 2024-02-10 at `$90`. Full P&L = `$10 × N`. The entire P&L is attributed to **2024** (the exit year). **2023 receives `$0`** from this trade despite the position being open for 6 weeks of 2023.

**Effect.** Per-year totals (`Returns.txt`, the yearly print at [DowATRStrategy.h:429-434](BackTrader/StrategyRunner/DowATRStrategy/DowATRStrategy.h#L429-L434), and any `Stats.py` yearly-Sharpe / per-year-drawdown computation) are distorted by year-straddling trades. Multi-year averages converge to the right answer; *per-year* peaks and drawdowns do not. Bias is largest for trend-following systems whose winners are long-held.

**Fix.** Mark-to-market at year boundaries. For each closed position straddling a 31-Dec, compute `(close_of_last_bar_of_year_Y - purchasePrice) * numShares` (LONG) for year `Y` and attribute the remainder to year `Y+1`. Requires injecting the daily bar series into the aggregator or storing intra-position year-end snapshots when a position is held over 31-Dec.

---

### M7 — Forced final-bar exit uses last close, breaking the next-bar-open convention   *(Affects: All modes)*

**Location:** [CustomStrategy.cpp:120-143](BackTrader/TradingStrategy/Custom/CustomStrategy.cpp#L120-L143) (daily overload), [298-321](BackTrader/TradingStrategy/Custom/CustomStrategy.cpp#L298-L321) (minute overload)

**The code:**
```cpp
if (!this->getPosition().getIsClosed()){
    Position &lastPosition = this->getPosition();
    lastPosition.setSellPrice(data.close.back());     // <-- last close, not future open
    lastPosition.setSellDate(data.date.back());
    ...
}
```

Throughout the normal loop, fills happen at `futureData.open` (next bar's open). At the end of the data there is no next bar, so the engine fills at `data.close.back()` (today's close) — a **different fill convention** from every other exit in the same backtest. Closes are typically tighter to the bar's mean than opens, so positions open on the last bar receive a marginally optimistic exit.

**Effect.** Small per-backtest bias but **systematic across the sweep** — every parameter combo evaluates against the same final bar, so the bias does not cancel across configs.

**Fix.** Force-close on the **second-to-last** bar using the last bar's open, matching the rest of the convention. The loop already runs `i = 1` to `size - 2`, so this means moving the forced close inside the loop on the last iteration rather than after it.

---

### M8 — yfinance daily data uses unadjusted close   *(Affects: Daily (1, 2))*

**Location:** [DownloadData.py:130-138](DownloadDataPython/DownloadData.py#L130-L138)

The yfinance pipeline writes raw OHLC, not `Adj Close`. For commodity-futures `=F` tickers the practical impact is negligible (no dividends, no splits). But if the universe is ever expanded to equities, unadjusted close means every ex-dividend date becomes a phantom price drop that can falsely trigger stops, and splits cause discontinuities.

**Fix.** Document the assumption near [DownloadData.py:130](DownloadDataPython/DownloadData.py#L130). If equities are ever added, switch to `Adj Close` for return calculation while keeping raw `Open/High/Low/Close` for fill prices (or use a roll-adjusted continuous series equivalent).

---

## Section 3 — Low Findings

| ID | Affects | Summary |
|---|---|---|
| **L1** | All | ATR snapshot is one bar optimistic — `processNewData` is called before `purchasePosition`, so bar `i` enters the ATR window before sizing the bar `i+1` fill ([CustomStrategy.cpp:95,273](BackTrader/TradingStrategy/Custom/CustomStrategy.cpp#L273)). Tiny smoothed-average bias. |
| **L2** | All | `DataReader::ReadData` does not validate that OHLCV+date vectors are equal length ([DataReader.cpp:3-74](BackTrader/DataReader/DataReader.cpp#L3-L74)). A truncated `MinuteData.txt`/`data.txt` produces desynced parallel vectors; subsequent indexed access is undefined behaviour. Fix: at end of `ReadData`, assert `open.size() == close.size() == high.size() == low.size() == volume.size() == date.size()` and drop the ticker with a `cerr` if not. |
| **L3** | All | `WindowStatistics` slope / t-statistic can become NaN on small windows ([WindowStatistics.cpp:41-51](BackTrader/Functions/WindowStatistics/WindowStatistics.cpp#L41-L51)). Moot because every consumer (MACD, trendline significance) is commented out in `DowContext`. Resurrect if those filters are re-enabled. |
| **L4** | Minute (3, 4) | `binarySearchDate` returns -1; `max(-1, 1)` clamp at [CustomStrategy.cpp:164](BackTrader/TradingStrategy/Custom/CustomStrategy.cpp#L164) is structurally dead because the explicit `return` at 158–161 catches the −1 case first. Defensive but unreachable. |
| **L5** | Minute (3, 4) | `minuteSlice.emplace_back(idx, ...)` stores the global minute-array index, not the slice position ([CustomStrategy.cpp:220](BackTrader/TradingStrategy/Custom/CustomStrategy.cpp#L220)). `Position::getExitPrice` never reads this field, but a future `toJson()` on a minute instance will emit the wrong index. |
| **L6** | Minute (3, 4) | DailyData / MinuteData universe alignment is not summarised globally ([Main.cpp:294-322](BackTrader/Main.cpp#L294-L322)). Per-ticker mismatches are warned individually; add one top-level "minute data found for X of Y selected tickers" log. |
| **L7** | All | SMAMACD / EMAMACD compute the signal as an SMA / EMA **of close**, not as an EMA of MACD itself ([SMAMACD.cpp:22-26](BackTrader/Indicators/SMAMACD/SMAMACD.cpp#L22-L26)). Non-standard formula. Currently moot — the MACD filter is commented out in `DowContext` — but the field name will mislead the next reader. |
| **L8** | All | Stop-loss level is computed using bar `i+1`'s open AND ATR over bars `0..i` ([ATRPositionSize.cpp:15](BackTrader/PositionSizing/ATRPositionSize/ATRPositionSize.cpp#L15) called via [CustomStrategy.cpp:98,276](BackTrader/TradingStrategy/Custom/CustomStrategy.cpp#L276)). Defensible in backtest (in live you'd set the stop at the actual fill price after the fact), but worth documenting so the convention is explicit. |

---

## Section 4 — What Is Sound

| Area | Why it is sound |
|---|---|
| Entry fill timing | Signal computed at bar `i` close; fill at bar `i+1` open. Standard backtest convention; no look-ahead. (All 4 modes.) |
| Trailing stop ratchet | LONG: monotone increasing; SHORT: monotone decreasing ([ATRPositionSize.cpp:57-79](BackTrader/PositionSizing/ATRPositionSize/ATRPositionSize.cpp#L57-L79)). |
| Minute timestamp convention | UTC calendar-day, empirically verified ([verify_minute_convention.py](DownloadDataPython/verify_minute_convention.py) + [verification_report.txt](verification_report.txt) — 30/30 OHLC match across `LIGHT.CMD/USD`, `XAU/USD`, `COFFEE.CMD/USX`). |
| `MacroFeatures` look-ahead | All computations strictly trailing (≤ current date); no peek at future bars. |
| `TrendIdentifier` / `TrendLineTracker` | Extrema labelled with strictly trailing logic. Not consumed by the current entry rule but the code itself is correct. |
| ATR formula | Textbook: `max(high - low, |high - prevClose|, |low - prevClose|)`. |
| Slice search helpers (minute) | `std::lower_bound` / `std::upper_bound` at [CustomStrategy.cpp:207-208](BackTrader/TradingStrategy/Custom/CustomStrategy.cpp#L207-L208) correctly produce a `[start, end)` half-open range. |
| Per-ticker state reset | `setBalance` + `context->clear()` + `sizer->clear()` between tickers prevents state leakage. (The ticker-isolated *semantics* are M5; the reset *mechanic* itself is sound.) |
| SHORT P&L accounting | `−numShares × purchasePrice` at entry plus `numShares × (2 × purchasePrice − sellPrice)` at exit nets to the correct `numShares × (purchasePrice − sellPrice)`. Real-world shorting mechanics (margin, borrow) are not modelled, but the arithmetic is correct. |

---

## Section 5 — Recommended Order of Remediation

1. **C1 — contract multipliers.** Without this, every dollar quantity in the engine's output is uninterpretable. Touches `Position`, `ATRPositionSize`, `BaseStrategy::getYearlyReturns`.
2. **C2 — friction model.** Required before any sweep ranking is trustworthy. One subtraction at position close, parameterised by the same `contractSpecs.json` as C1.
3. **M5 — portfolio-level risk sizing** (plus the `CLAUDE.md` documentation note). Required for any user planning to deploy real capital across the full universe.
4. **M4 — back-adjust continuous contracts** at download time. Removes phantom roll-day stops in both yfinance and Dukascopy paths.
5. **M6 — split year-straddling P&L** for per-year statistics.
6. **M7 — align forced final exit** with the next-bar-open convention.
7. **L2 — length validation in `DataReader`.** One-time hygiene fix; prevents UB on corrupted data files.

Remaining items (M1, M2, M3, M8, L1, L3, L4, L5, L6, L7, L8) are acceptable to defer.
