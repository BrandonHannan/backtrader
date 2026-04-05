# NeuralNetwork — Improvement Strategies

**Current baseline:** Best model `diamond_32_256_128_32`, AUC-ROC = 0.5250 (barely above random).
**Dataset:** 10,442 closed trades. Data quantity is not the bottleneck — feature quality and evaluation methodology are.

Strategies are ordered from highest to lowest expected impact. Each entry includes what to do, why it helps, a plain-English explanation, and a workflow showing how it integrates with the C++/Python pipeline.

---

## Tier 1 — Fix the Evaluation (Highest Impact, No Model Changes)

### 1. Temporal Train / Val / Test Split

**What to do:**
Sort all trades chronologically by `buyDate`, then split sequentially — earliest 70% to train, next 15% to val, latest 15% to test. Remove `train_test_split(..., stratify=y)` entirely.

**Why it improves results:**
The current random split draws from all time periods simultaneously. A model trained on a random mix of 2010 and 2024 trades and tested on another random mix of 2010 and 2024 trades is cheating — it has seen future market regimes during training. A model that passes this test may completely fail in live deployment, because real trading is always forward in time. Temporal splitting enforces this constraint and gives a true estimate of out-of-sample performance.

**Simple explanation:**
Imagine studying for a history exam using questions randomly pulled from *last year's* and *this year's* test. You'd score well on practice, but you cheated — some of the practice questions were the real test questions. Temporal splitting ensures the model studies only the past and is tested only on the future, exactly like real trading.

**Workflow:**
```
output/data.json  →  load_positions() returns (X, y, dates)
                  →  sort by dates (ascending)
                  →  X_train = X[:70%], X_val = X[70%:85%], X_test = X[85%:]
                  →  fit scaler on X_train only (no change)
                  →  train all 51 configs as before
```

**Files to change:**
- `NeuralNetwork/neural_network.py` — modify `load_positions()` to return `dates` list alongside `X, y`; replace `train_test_split` with index-based chronological slicing

---

### 2. Switch to Gradient Boosting (XGBoost / LightGBM) as Baseline

**What to do:**
Add XGBoost and LightGBM models to the grid search alongside the neural network families. Train them on the same splits with the same class-imbalance correction (`scale_pos_weight = num_neg / num_pos`). Report their AUC alongside the neural networks.

**Why it improves results:**
On tabular datasets under 100k rows, gradient boosting consistently outperforms neural networks in published benchmarks and machine learning competitions. The AUC plateau at 0.525 *across all 51 neural network architectures* is a strong signal that MLPs are struggling to extract the signal. XGBoost builds hundreds of shallow decision trees sequentially, each one specifically correcting the errors of the previous tree — a process well-suited to discovering rule-based patterns like "MACD positive AND trend up AND trendline active → profitable."

**Simple explanation:**
A neural network adjusts thousands of interconnected knobs simultaneously, searching for the right combination. XGBoost instead learns a series of simple yes/no rules ("if MACD > 0 and trend is UP, predict profitable"), stacks them up, and each new rule specifically fixes the mistakes of the previous ones. Trading signals often follow this kind of logical rule structure, which makes XGBoost more natural for this problem.

**Workflow:**
```
(X_train, y_train, X_val, y_val, X_test, y_test) already prepared by neural_network.py
    →  tree_models.py imports these splits
    →  trains XGBClassifier(scale_pos_weight=pos_weight, n_estimators=500, max_depth=6)
    →  trains LGBMClassifier(class_weight='balanced', n_estimators=500)
    →  evaluate() returns TrainResult for each
    →  results added to the leaderboard table under family="xgboost"/"lightgbm"
```

**Files to change:**
- New `NeuralNetwork/tree_models.py`
- `NeuralNetwork/requirements.txt` — add `xgboost`, `lightgbm`
- `NeuralNetwork/neural_network.py` — import and call `tree_models` after the neural network sweep, merge results into leaderboard

---

## Tier 2 — Add More Indicators to C++ (New Signal Sources)

### 3. Implement RSI and Add to DowContext

**What to do:**
Complete the `RSIContext` stub (currently header-only with no `.cpp`) into a working Relative Strength Index calculation. Integrate an RSI indicator directly into `DowContext` and serialize `rsiValue` and `rsiReady` to the Position JSON.

**Why it improves results:**
RSI measures whether a market is overbought (RSI > 70) or oversold (RSI < 30). Trades entered at extremes have statistically different outcomes — an oversold market has more room to bounce; an overbought market may reverse. This is a classic momentum signal that is entirely absent from the current 64-feature vector. Adding it gives the model information about whether a trade entry caught a market at a stretched extreme or in neutral territory.

**Simple explanation:**
RSI is a "fatigue meter" for price. If a market has been rising continuously for weeks (RSI > 70), it's tired and may reverse. If it has been falling (RSI < 30), it may be due for a bounce. The model currently has no way to know this — it sees a profitable trade but doesn't know whether the market was exhausted or energised at entry. RSI provides that context.

**RSI formula (Wilder's smoothing):**
```
For first RSI_PERIOD candles:
  AvgGain = mean(all gains over period)
  AvgLoss = mean(all losses over period)

For each subsequent candle:
  AvgGain = (AvgGain × (period - 1) + currentGain) / period
  AvgLoss = (AvgLoss × (period - 1) + currentLoss) / period
  RS = AvgGain / AvgLoss
  RSI = 100 - (100 / (1 + RS))
```

**Workflow:**
```
C++:  DowContext::updateContext() computes RSI via RSI helper
          →  stores rsiValue (double), rsiReady (bool)
      DowContext::getContextData() serializes:
          json["rsiValue"] = rsiValue
          json["rsiReady"] = rsiReady
      make run  →  output/data.json now has rsiValue/rsiReady per position

Python:  neural_network.py extract_context_features():
          features.append(float(ctx.get("rsiValue", 50.0)))        # 1 float
          features.append(1.0 if ctx.get("rsiReady") else 0.0)     # 1 binary
         Feature vector grows from 64 → 66 features
```

**Files to change:**
- `BackTrader/TradingContext/RSIContext/RSIContext.cpp` — create, implement Wilder's RSI
- `BackTrader/TradingContext/DowContext/DowContext.h` — add RSI member + serialization
- `BackTrader/TradingContext/DowContext/DowContext.cpp` — update `updateContext()` and `getContextData()`
- `NeuralNetwork/neural_network.py` — add RSI feature extraction

---

### 4. Add ATR Value as a Volatility Feature

**What to do:**
The `ATRPositionSize` already computes ATR for position sizing. Expose the computed ATR value (and a ready flag) in `DowContext`'s JSON output so the neural network can use it as a feature.

**Why it improves results:**
ATR (Average True Range) measures current market volatility — the average size of daily price swings. High-ATR periods (volatile markets) and low-ATR periods (calm markets) have systematically different trade dynamics. A $500 ATR on crude oil is very different from a $50 ATR. Currently the model has no volatility context — it treats a calm-market entry and a volatile-market entry identically.

**Simple explanation:**
ATR is a "how choppy is the market" score. Imagine trying to walk in a straight line on a calm day versus in a windstorm. The model currently can't tell whether the market is calm or stormy at trade entry — ATR gives it that information. A high-ATR trade needs more room to breathe; a low-ATR trade can use tighter targets. Knowing the volatility regime changes how a trade should be evaluated.

**ATR formula:**
```
TrueRange(t) = max(high(t) - low(t),
                   abs(high(t) - close(t-1)),
                   abs(low(t)  - close(t-1)))
ATR(t) = (ATR(t-1) × (period-1) + TrueRange(t)) / period    ← Wilder's smoothing
```

**Workflow:**
```
C++:  ATRPositionSize::purchasePosition() already has ATR value
         →  pass atrValue to DowContext at trade entry (or compute independently in DowContext)
      DowContext::getContextData() serializes:
         json["atrValue"] = atrValue
         json["atrReady"] = atrReady

Python:  extract_context_features():
          features.append(float(ctx.get("atrValue", 0.0)))          # 1 float
          features.append(1.0 if ctx.get("atrReady") else 0.0)      # 1 binary
         Feature vector grows by 2 more features
```

**Files to change:**
- `BackTrader/TradingContext/DowContext/DowContext.h/.cpp` — add atrValue member and serialization
- `BackTrader/PositionSizing/ATRPositionSize/ATRPositionSize.h/.cpp` — expose ATR value after calculation
- `NeuralNetwork/neural_network.py` — add ATR feature extraction

---

### 5. Add Bollinger Bands Indicator

**What to do:**
Create `BackTrader/Indicators/BollingerBands/BollingerBands.h/.cpp` implementing upper band, lower band, middle band (20-period SMA), band width, and %B (price position within bands). Serialize these 5 values + a ready flag to `DowContext`.

**Why it improves results:**
Bollinger Bands provide three distinct signals not currently captured: (1) **price position** — is price near the upper or lower band? (2) **band width** — is the market in a low-volatility squeeze (bands tight) or high-volatility expansion? (3) **band direction** — are the bands widening or narrowing? These signals are orthogonal to RSI and MACD, providing new information.

**Simple explanation:**
Bollinger Bands are like a rubber band stretched around price. When price pushes to the outer edge, it tends to snap back. When the bands squeeze together tightly, the market is "coiling" and a big move is likely coming. `%B` tells you exactly where price sits within the band (0 = at the bottom, 1 = at the top). These signals complement RSI and MACD by adding a volatility-aware view of overbought/oversold conditions.

**Key values to serialize:**
```
upperBand   = SMA(20) + 2 × StdDev(20)
lowerBand   = SMA(20) - 2 × StdDev(20)
middleBand  = SMA(20)
bandWidth   = (upperBand - lowerBand) / middleBand    ← normalised volatility
percentB    = (close - lowerBand) / (upperBand - lowerBand)  ← 0 to 1
```

**Workflow:**
```
C++:  New BackTrader/Indicators/BollingerBands/BollingerBands.h/.cpp
         →  uses deque of recent closes (20 periods)
         →  computes mean, stddev, upper/lower/middle, bandWidth, %B
      DowContext integrates BollingerBands like existing SMAMACD
      DowContext::getContextData() adds 6 new JSON fields

Python:  extract_context_features():
          features += bollinger_features(ctx)   # 6 new values
```

**Files to change:**
- New `BackTrader/Indicators/BollingerBands/BollingerBands.h`
- New `BackTrader/Indicators/BollingerBands/BollingerBands.cpp`
- `BackTrader/TradingContext/DowContext/DowContext.h/.cpp`
- `NeuralNetwork/neural_network.py`

---

### 6. Add Stochastic Oscillator

**What to do:**
Create `BackTrader/Indicators/StochasticOscillator/` implementing %K (fast stochastic) and %D (3-period SMA of %K). Serialize `stochasticK`, `stochasticD`, and `stochasticReady` to `DowContext`.

**Why it improves results:**
Stochastic is complementary to RSI — both measure momentum, but differently. RSI measures the speed of recent price changes; Stochastic measures where price sits relative to its recent high/low range. A market can have an RSI of 60 (not overbought) but a Stochastic of 85 (near its recent high). Their agreement or disagreement is itself a predictive signal. Adding Stochastic provides a second independent momentum oscillator.

**Simple explanation:**
Stochastic answers: "Is the current price near the top or the bottom of where it's been recently?" If price is near the recent high (Stochastic > 80), it's overbought — you might be buying at the peak. If near the recent low (Stochastic < 20), it's oversold — you might be buying at a bargain. The model currently can't distinguish "buying near the top" from "buying near the bottom" — Stochastic adds this context.

**Formula:**
```
%K = 100 × (close - lowest_low[n]) / (highest_high[n] - lowest_low[n])
     where n = 14 periods

%D = 3-period SMA of %K    ← signal line (smoother)
```

**Workflow:**
```
C++:  BackTrader/Indicators/StochasticOscillator/ tracks rolling 14-period high/low
         →  computes %K on each candle, %D as 3-period SMA of %K
      DowContext::getContextData() serializes 3 new fields

Python:  extract_context_features():
          features.append(float(ctx.get("stochasticK", 50.0)))
          features.append(float(ctx.get("stochasticD", 50.0)))
          features.append(1.0 if ctx.get("stochasticReady") else 0.0)
```

**Files to change:**
- New `BackTrader/Indicators/StochasticOscillator/StochasticOscillator.h`
- New `BackTrader/Indicators/StochasticOscillator/StochasticOscillator.cpp`
- `BackTrader/TradingContext/DowContext/DowContext.h/.cpp`
- `NeuralNetwork/neural_network.py`

---

### 7. Enable EMA MACD (Already Implemented in C++, Unused)

**What to do:**
`BackTrader/Indicators/EMAMACD/` already exists and is compiled. Instantiate it in `DowContext` alongside the existing `SMAMACD`. Serialize `emaMACD`, `emaSignal`, and `emaMACDReady` as 3 new JSON fields.

**Why it improves results:**
EMA MACD is more responsive than SMA MACD — it weights recent price changes more heavily because exponential averages decay older values. Having both gives the model a *fast* momentum signal (EMA) and a *slow* momentum signal (SMA) simultaneously. Their agreement ("both positive → strong uptrend"), disagreement ("SMA positive, EMA negative → momentum fading"), and crossover timing are all predictive signals not currently available to the model.

**Simple explanation:**
SMA MACD is like a cautious analyst who treats every day from the past two months equally. EMA MACD is an alert analyst who pays more attention to yesterday than to last month. Having both viewpoints — and noticing when they agree or disagree — gives the model richer momentum information. Currently the model only hears from the cautious analyst.

**Workflow:**
```
C++:  DowContext already has SMAMACDIndicator
         →  add EMAMACDIndicator instance (same pattern, no new files needed)
         →  DowContext::updateContext() computes both
         →  DowContext::getContextData() adds emaMACD, emaSignal, emaMACDReady

Python:  extract_context_features():
          features.append(float(ctx.get("emaMACD", 0.0)))
          features.append(float(ctx.get("emaSignal", 0.0)))
          features.append(1.0 if ctx.get("emaMACDReady") else 0.0)
```

**Files to change:**
- `BackTrader/TradingContext/DowContext/DowContext.h/.cpp` only — `EMAMACD` class already exists
- `NeuralNetwork/neural_network.py`

---

## Tier 3 — Add Temporal and Contextual Features (Python-Only)

### 8. Add Entry Date Temporal Features

**What to do:**
Parse `buyDate` (already in `output/data.json`) in `load_positions()` and derive 4 new features: month of year (1–12), day of week (0–4), quarter (1–4), and year (normalised to 0–1 over the data range).

**Why it improves results:**
Markets have well-documented seasonal patterns: the "sell in May and go away" effect, the January effect (stocks rally in January), end-of-quarter portfolio rebalancing, commodity seasonal demand cycles (energy in winter, grains at harvest), etc. The current model has no temporal awareness — a trade entered in January and a trade entered in August with identical technical signals are treated as identical. Adding month/quarter allows the model to learn these patterns.

**Simple explanation:**
If you always checked the weather at the same time of day under the same conditions, you might notice it rains more in October than July. The model can't currently tell "this trade happened in September" from "this trade happened in December" — adding month and quarter lets it discover seasonal patterns in which months tend to produce profitable trades for each commodity.

**Workflow:**
```
output/data.json  →  load_positions() parses buyDate string (e.g. "2023-09-15")
                  →  from datetime import datetime
                     dt = datetime.strptime(buyDate, "%Y-%m-%d")
                     month    = dt.month / 12.0          # 0.08 to 1.0
                     dayofweek = dt.weekday() / 4.0      # 0.0 to 1.0
                     quarter  = ((dt.month - 1) // 3 + 1) / 4.0
                     year_norm = (dt.year - min_year) / (max_year - min_year)
                  →  append these 4 values to the feature vector
                     Feature vector grows from 64 → 68 features (no C++ changes needed)
```

**Files to change:**
- `NeuralNetwork/neural_network.py` only

---

### 9. Add Ticker Identity Encoding

**What to do:**
Add a `ticker` field to the Position JSON output from the C++ `ExecuteAllSweeps()`. In Python, encode the 32 tickers as a one-hot vector (32 binary features) or a single normalised integer.

**Why it improves results:**
`CL=F` (crude oil) behaves completely differently from `ZC=F` (corn) — different volatility, different seasonal cycles, different response to the same MACD signal. Currently the model treats a crude oil trade and a corn trade identically. Without knowing the ticker, the model must average over all 32 commodities, introducing systematic noise. Ticker encoding lets the model learn commodity-specific patterns.

**Simple explanation:**
Imagine predicting tomorrow's weather for 32 different cities using only temperature readings, without knowing which city each reading came from. The model would produce mediocre predictions for every city because it's averaging over completely different climates. Telling the model "this trade is in Phoenix, this one is in Seattle" immediately allows it to learn that Phoenix is usually sunny and Seattle is often rainy. The same logic applies: crude oil and corn have different "weather patterns."

**Workflow:**
```
C++:  ExecuteAllSweeps() — the ticker string is already in the sweep loop
         →  add "ticker": tickerSymbol to each Position's JSON output

Python:  TICKER_MAP = {"CL=F": 0, "BZ=F": 1, ..., "GNF=F": 31}   # 32 tickers
         load_positions():
           ticker_idx = TICKER_MAP.get(pos.get("ticker", ""), 0)
           one_hot = [0.0] * 32
           one_hot[ticker_idx] = 1.0
           X_rows.append([position_type] + ctx_features + one_hot)
         Feature vector grows from 64 → 96 features
```

**Files to change:**
- `BackTrader/StrategyRunner/DowATRStrategy/DowATRStrategy.h` — add ticker to Position JSON in sweep loop
- `NeuralNetwork/neural_network.py` — add ticker one-hot encoding

---

### 10. Label Refinement — Minimum Profit Threshold

**What to do:**
Change the binary label from `pnl > 0` to `pnl > THRESHOLD` (default: $0, tunable via `--profit-threshold`). Optionally exclude trades where `0 < pnl < THRESHOLD` (ambiguous) from training entirely.

**Why it improves results:**
A trade with pnl = $0.01 is currently labelled "profitable" — identical to a trade with pnl = $5,000. Borderline trades near zero are barely distinguishable from losses in terms of their entry-context features. Including them as positive examples adds noise to the training signal. A minimum threshold creates clearer class separation, allowing the model to find sharper patterns that distinguish clear winners from clear losers.

**Simple explanation:**
If you asked someone "was this restaurant visit good or bad?" and they said "it was okay, nothing special" — was that a yes or a no? Ambiguous answers make it hard to learn what "good" means. Filtering out the "barely profitable" trades leaves only clear winners vs. clear losers, making the patterns the model needs to learn far more distinct. You could even try $0 threshold first, then $100, and compare AUC.

**Workflow:**
```
neural_network.py CLI:
    parser.add_argument("--profit-threshold", type=float, default=0.0)

load_positions():
    if pnl == 0.0 and sell_date == "":  skip (open position)
    if 0.0 < pnl <= args.profit_threshold:  skip (ambiguous)
    y = 1 if pnl > args.profit_threshold else 0
```

**Files to change:**
- `NeuralNetwork/neural_network.py` only

---

## Tier 4 — Better Training and Evaluation Methodology

### 11. Feature Importance Analysis (SHAP Values)

**What to do:**
After training the best model (especially an XGBoost model from strategy 2), compute SHAP values to rank all features by their contribution to predictions. Use this ranking to guide feature engineering and identify zero-signal features to remove.

**Why it improves results:**
Features like `e4` and `e5` extremum points (index, close, isTrough for the 4th and 5th trend extrema) are often zero-padded when the trend doesn't have that many points. If these features contribute zero predictive power, they're adding noise to the model without adding signal. SHAP reveals exactly which features help and which hurt, guiding future engineering efforts toward high-ROI improvements.

**Simple explanation:**
SHAP is like asking: "If I remove this one ingredient from the recipe, how much does the dish change?" For every feature across every prediction, SHAP calculates how much that specific feature contributed to the model's decision. This lets you rank features from "crucial" to "completely useless" and cut the useless ones — making the model's job easier by removing distractions.

**Workflow:**
```
After training XGBoost (strategy 2):
    import shap
    explainer = shap.TreeExplainer(xgb_model)
    shap_values = explainer.shap_values(X_test)
    feature_names = get_feature_names()    # list matching the 64+ feature vector
    shap.summary_plot(shap_values, X_test, feature_names=feature_names)
    → prints/saves ranked bar chart of feature importances
    → features with near-zero importance are candidates for removal
```

**Files to change:**
- New `NeuralNetwork/analysis/feature_importance.py`
- `NeuralNetwork/requirements.txt` — add `shap`

---

### 12. Time-Aware K-Fold Cross-Validation

**What to do:**
Replace the single 70/15/15 split with `sklearn.model_selection.TimeSeriesSplit(n_splits=5)`. For each fold, train on past data, validate on immediately following data. Report mean ± std AUC per config.

**Why it improves results:**
A single split gives one performance estimate — which may be lucky or unlucky depending on which trades happen to fall in the test window. With 10k trades and ~1,500 in the test set, the estimate has high variance. Five time-ordered folds give five independent estimates, averaged to a much more reliable measure of generalisation. Mean ± std also reveals whether a model is consistently good or just got lucky on one fold.

**Simple explanation:**
Instead of studying once and taking one test, you study and test five times — each time the study material is older data and the test is newer data. Your average score across the five tests is a much fairer measure of whether you actually learned the material or just got lucky on the one test. A model that scores 0.62, 0.60, 0.61, 0.59, 0.62 is more trustworthy than one scoring 0.48, 0.51, 0.72, 0.50, 0.49 (high variance = unreliable).

**Workflow:**
```
from sklearn.model_selection import TimeSeriesSplit
tscv = TimeSeriesSplit(n_splits=5)

for fold, (train_idx, test_idx) in enumerate(tscv.split(X_sorted)):
    X_tr, X_te = X_sorted[train_idx], X_sorted[test_idx]
    y_tr, y_te = y_sorted[train_idx], y_sorted[test_idx]
    # use last 15% of train_idx as val_idx
    for config in all_configs:
        result = train_and_evaluate(config, X_tr, y_tr, X_val, y_val, X_te, y_te)
        fold_results[config].append(result.auc_roc)

for config in all_configs:
    mean_auc = mean(fold_results[config])
    std_auc  = std(fold_results[config])
    # report both in leaderboard
```

**Files to change:**
- `NeuralNetwork/neural_network.py`
- `NeuralNetwork/architectures/base.py`

---

### 13. Prediction Threshold Optimisation

**What to do:**
Instead of classifying as "profitable" when `sigmoid(logit) >= 0.5`, find the threshold on the validation set that maximises a chosen metric (F1 or expected profit). Apply that threshold to the test set.

**Why it improves results:**
The 0.5 threshold is arbitrary. With 40% profitable trades, the optimal threshold may be lower (e.g., 0.38) — the model may assign lower probabilities to profitable trades on average, and a lower threshold captures more of them. Alternatively, if precision matters more (you only want high-confidence calls), a threshold of 0.65 might filter to only the clearest predictions. Optimising the threshold for the actual business objective (max profit, max F1) can significantly improve deployed performance without retraining.

**Simple explanation:**
A spam filter doesn't have to be exactly 50% confident before marking something as spam — you can tune it to be more or less aggressive. In trading, you might prefer to catch 80% of winning trades even if some losing ones slip through, rather than missing most winners to be "safe." Threshold tuning finds the confidence level that balances catching winners against avoiding losers, using actual validation data to make the decision.

**Workflow:**
```
After training, compute ROC curve on validation set:
    from sklearn.metrics import roc_curve
    fpr, tpr, thresholds = roc_curve(y_val, probs_val)
    f1_scores = [f1_score(y_val, probs_val >= t) for t in thresholds]
    optimal_threshold = thresholds[argmax(f1_scores)]

Re-evaluate test set:
    preds = (probs_test >= optimal_threshold).astype(int)
    report metrics at optimal_threshold alongside default 0.5 metrics
```

**Files to change:**
- `NeuralNetwork/architectures/base.py` — `evaluate()` function
- `NeuralNetwork/neural_network.py` — report threshold in leaderboard output

---

### 14. Ensemble Predictions from Top-N Models

**What to do:**
Instead of saving only the single best model, average the predicted probabilities from the top-5 models on the leaderboard. Evaluate the ensemble as a whole.

**Why it improves results:**
Each model architecture makes different errors on different trades. A funnel that misses one class of trades and a diamond that catches those trades but misses others — combined, their probability outputs average out the individual blind spots. Ensemble methods are the most reliable technique for squeezing additional performance from a set of trained models, used in virtually all winning machine learning competition solutions.

**Simple explanation:**
Asking one expert for advice is good. Asking five experts and averaging their recommendations is usually better — one expert's blind spot is covered by another. The same applies to neural networks: the funnel makes mistakes where the cylinder is confident, and vice versa. Averaging their probability estimates produces a more balanced and reliable prediction.

**Workflow:**
```
After training all 51 configs, store all test-set probability outputs:
    all_probs = {}  # config_name → np.array of probs on X_test

Sort by AUC, take top-5:
    top5_names = [results[i]['cfg']['name'] for i in range(5)]
    ensemble_probs = mean([all_probs[n] for n in top5_names], axis=0)

Evaluate ensemble:
    result = evaluate_from_probs(ensemble_probs, y_test)
    print("Ensemble AUC:", result.auc_roc)

Save ensemble config:
    model_ensemble.json → [{name, family, hidden_dims, dropout, weight: 1/5}, ...]
```

**Files to change:**
- `NeuralNetwork/neural_network.py`
- New `NeuralNetwork/model_ensemble.json` (generated output)

---

## Tier 5 — Extended Data Collection (C++ Changes)

### 15. Increase Trade Volume via More Parameter Sweeps

**What to do:**
Expand `ExecuteAllSweeps()` to sweep across a wider range of lookback periods (e.g., 10, 15, 20, 30, 40 days instead of a single value) and ATR multipliers. Each parameter combination generates additional closed Position records. Add the parameter values as features in the Python feature vector.

**Why it improves results:**
More trades = more training examples, but only if they represent genuinely different market conditions. Different lookback periods produce entries at different points in the same trends — a 10-day lookback enters earlier, a 40-day lookback enters after more confirmation. This diversity in entry timing, combined with labelled outcomes, helps the model learn which confirmation level is more predictive of profitability. Note: the parameter value itself must be added as a feature to avoid confounding.

**Simple explanation:**
Currently the model learns from trades all generated with the same timing rules. Expanding to multiple parameter settings is like studying trading signals at different timescales simultaneously — some traders use 10-day signals, some use 40-day signals. Adding both to the training data, along with a label saying "this trade used a 10-day lookback," lets the model learn which timing scale is more reliable in different market conditions.

**Workflow:**
```
C++:  DowATRStrategy.h ExecuteAllSweeps():
         vector<int> lookbacks = {10, 15, 20, 30, 40};
         for (int lb : lookbacks) {
             DowContext ctx(lb, ...);
             // run full sweep, write all positions to output/data.json
         }
         Also serialize "lookbackPeriod" to each Position's JSON

Python:  load_positions():
          features.append(float(pos.get("lookbackPeriod", 20)) / 40.0)  # normalise
         Feature vector grows by 1 feature
```

**Files to change:**
- `BackTrader/StrategyRunner/DowATRStrategy/DowATRStrategy.h`
- `NeuralNetwork/neural_network.py`

---

### 16. Add Short-Term Trend for Multi-Timeframe Analysis

**What to do:**
Add a third Dow Theory trend to `DowContext` using `lookBackPeriod / 2` (the short-term trend), alongside the existing primary trend and double trend. Serialize `shortTrendReady` and `shortTrend` (17 features) to the Position JSON.

**Why it improves results:**
Currently `DowContext` captures two timeframes: the primary trend and the double trend (2× lookback). Adding a half-period trend provides short-term momentum context. Multi-timeframe agreement is a much stronger signal than any single timeframe — when short, medium, and long-term trends all point the same direction, that's a high-conviction entry. Conflicts (short-term down, long-term up) signal choppy conditions where outcomes are more uncertain.

**Simple explanation:**
Imagine reading three sources of news: today's headlines, this week's summary, and this month's overview. When all three agree ("stocks falling everywhere"), that's more reliable than when they conflict. DowContext currently reads two news sources. Adding the short-term "daily news" lets the model detect when all three agree — which tends to produce more reliable trade outcomes than when they disagree.

**Workflow:**
```
C++:  DowContext has trend (period), doubleTrend (2×period)
         →  add shortTrendIdentifier(period / 2)
         →  compute shortTrend in updateContext()
         →  serialize shortTrendReady and shortTrend (same structure as trend)
         →  17 additional JSON fields

Python:  extract_context_features():
          features.extend(_trend_features(
              ctx.get("shortTrendReady", False),
              ctx.get("shortTrend", {})
          ))
         Feature vector grows by 17
```

**Files to change:**
- `BackTrader/TradingContext/DowContext/DowContext.h/.cpp`
- `NeuralNetwork/neural_network.py`

---

### 17. Add On-Balance Volume (OBV) Indicator

**What to do:**
Create `BackTrader/Indicators/OBV/OBV.h/.cpp` implementing On-Balance Volume — a cumulative sum that adds volume on up-days and subtracts it on down-days. Serialize `obvSlope`, `obvMomentum`, and `obvReady` to `DowContext`.

**Why it improves results:**
The current feature set has raw volume statistics (mean, std, slope) but no volume-price relationship indicator. OBV ties volume to price direction: sustained price rises on increasing OBV (volume confirming the move) are more likely to continue than price rises with declining OBV (volume diverging). This divergence signal between price trend and OBV trend is widely used in technical analysis and represents new information not captured by the existing 64 features.

**Simple explanation:**
OBV is like measuring crowd enthusiasm at a sports game. If the home team scores and the crowd erupts (high volume), the momentum is real and likely to continue. If they score but the crowd is quiet (low volume), the move might not last. OBV tracks whether price movements are backed by strong buyer/seller participation, which predicts whether a trend will continue or fade.

**Workflow:**
```
C++:  BackTrader/Indicators/OBV/OBV.h/.cpp
         obv_running += (close > prev_close) ? +volume : -volume
         Compute obvSlope using existing WindowStatistics infrastructure
         obvMomentum = obvSlope rate of change
      DowContext::getContextData():
         json["obvSlope"]     = obvSlope
         json["obvMomentum"]  = obvMomentum
         json["obvReady"]     = obvReady

Python:  extract_context_features():
          features.append(float(ctx.get("obvSlope", 0.0)))
          features.append(float(ctx.get("obvMomentum", 0.0)))
          features.append(1.0 if ctx.get("obvReady") else 0.0)
```

**Files to change:**
- New `BackTrader/Indicators/OBV/OBV.h`
- New `BackTrader/Indicators/OBV/OBV.cpp`
- `BackTrader/TradingContext/DowContext/DowContext.h/.cpp`
- `NeuralNetwork/neural_network.py`

---

## Tier 6 — Advanced Neural Network Techniques (Lower Priority)

### 18. Add L2 Weight Decay (AdamW)

**What to do:**
Replace `optim.Adam(lr=0.001)` with `optim.AdamW(lr=0.001, weight_decay=1e-4)` in `base.py`. AdamW applies L2 regularisation directly to weights, preventing any single weight from dominating predictions.

**Why it improves results:**
With 10k trades and the largest architectures having up to 800k parameters, models like `funnel_3L_1024` have far more parameters than training examples. Without regularisation, these models can memorise training data (overfit) rather than learning generalisable patterns. Weight decay penalises large weights, forcing the model toward simpler solutions that generalise better.

**Simple explanation:**
Weight decay is like telling the model "don't get obsessed with any single feature." Without it, the model might put enormous weight on one feature (e.g., MACD value) and largely ignore everything else. Even if this works during training, it fails when that one feature behaves differently in the test period. Weight decay keeps all features in balance, producing more robust predictions.

**Workflow:**
```
base.py train_model():
    optimizer = optim.AdamW(
        model.parameters(),
        lr=lr,
        weight_decay=1e-4    ← new parameter
    )
    # everything else unchanged
```

**Files to change:**
- `NeuralNetwork/architectures/base.py` only

---

### 19. Add Residual Connections for Deep Architectures

**What to do:**
For cylinder-family architectures (all hidden layers same width), add skip connections: `h_k = ReLU(BN(W_k · h_{k-1} + b_k)) + h_{k-1}`. The residual term `+ h_{k-1}` lets gradients flow directly to early layers. Implement as an optional `residual=True` flag in `GenericMLP`.

**Why it improves results:**
In deep networks (4–5 layers), gradients from the output layer become very small by the time they reach the first layer ("vanishing gradients"), making early layers learn slowly or not at all. Residual connections provide a gradient "highway" that bypasses intermediate layers. Microsoft's ResNet (2015) used this to successfully train 152-layer networks — without residuals, anything deeper than ~20 layers typically performed worse than shallower networks.

**Simple explanation:**
Imagine sending a message through 5 people, each of whom modifies it slightly before passing it on. By the time the original sender receives feedback about their message, it's been so changed that they don't know what to fix. Residual connections give the original sender a direct copy of the final output, so they can see exactly how their message contributed — making early layers much easier to train.

**Workflow:**
```
base.py GenericMLP with residual=True:
    for i, layer in enumerate(self.layers):
        h_new = layer(h)    ← Linear + BN + ReLU + Dropout
        if residual and h_new.shape == h.shape:
            h = h_new + h   ← skip connection (only when dimensions match)
        else:
            h = h_new

Only applies to cylinder configs where all hidden dims are equal.
Funnel/diamond have changing widths — skip connections require same dimensions.
```

**Files to change:**
- `NeuralNetwork/architectures/base.py` — add `residual` parameter to `GenericMLP`
- `NeuralNetwork/architectures/cylinder/cylinder.py` — add `residual: True` to config dicts

---

### 20. Learning Rate Warmup Schedule

**What to do:**
Add a linear warmup phase where the learning rate starts at `lr/10` and ramps up to `lr` over the first 5 epochs, then hands off to the existing `ReduceLROnPlateau` schedule. Implement using `torch.optim.lr_scheduler.SequentialLR`.

**Why it improves results:**
At the start of training, model weights are random (Xavier/He initialisation) and gradients are large and noisy. Starting with the full learning rate causes large, noisy weight updates that can push the model into a poor region of the loss landscape — one it never fully recovers from. Warmup prevents this by gently increasing the learning rate as weights stabilise, then switching to the plateau-based decay for fine-tuning.

**Simple explanation:**
Starting a cold car engine: you ease off the throttle until the engine warms up, then drive normally. If you floor it immediately in the cold, you risk damaging the engine. Learning rate warmup gives the model a gentle start — small adjustments while the initial random noise settles — before switching to normal training speed. This particularly helps large models (512–1024 wide) that are more sensitive to early updates.

**Workflow:**
```
base.py train_model():
    scheduler_warmup = LinearLR(
        optimizer,
        start_factor=0.1,     ← start at lr/10
        end_factor=1.0,       ← ramp to full lr
        total_iters=5         ← over 5 epochs
    )
    scheduler_plateau = ReduceLROnPlateau(optimizer, patience=5, factor=0.5)
    scheduler = SequentialLR(
        optimizer,
        schedulers=[scheduler_warmup, scheduler_plateau],
        milestones=[5]        ← switch after epoch 5
    )
    # replace current ReduceLROnPlateau with scheduler
```

**Files to change:**
- `NeuralNetwork/architectures/base.py` only

---

## Summary Table

| # | Strategy | Scope | Impact | Changes C++? |
|---|---|---|---|---|
| 1 | Temporal train/val/test split | Evaluation | Very High | No |
| 2 | XGBoost / LightGBM baseline | Model family | Very High | No |
| 3 | RSI indicator | Feature | High | Yes |
| 4 | ATR as feature | Feature | High | Yes |
| 5 | Bollinger Bands | Feature | High | Yes |
| 6 | Stochastic Oscillator | Feature | High | Yes |
| 7 | Enable EMA MACD | Feature | Medium-High | Yes |
| 8 | Entry date temporal features | Feature | Medium | No |
| 9 | Ticker identity encoding | Feature | Medium | Yes (minor) |
| 10 | Label profit threshold | Label | Medium | No |
| 11 | SHAP feature importance | Analysis | Medium | No |
| 12 | Time-aware K-fold CV | Evaluation | Medium | No |
| 13 | Threshold optimisation | Inference | Medium | No |
| 14 | Ensemble top-N models | Inference | Medium | No |
| 15 | More parameter sweeps | Data | Medium | Yes |
| 16 | Short-term trend (3rd timeframe) | Feature | Medium | Yes |
| 17 | OBV indicator | Feature | Low-Medium | Yes |
| 18 | AdamW weight decay | Training | Low | No |
| 19 | Residual connections | Architecture | Low | No |
| 20 | LR warmup schedule | Training | Low | No |

**Recommended starting sequence:** 1 → 2 → 8 → 10 → 3 → 7
(Fix evaluation first, add fast features, then implement C++ indicators.)
