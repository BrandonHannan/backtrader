# Neural Network — Orchestrator & Architecture Guide

## Overview

This module trains a binary classifier that predicts whether a trade will be **profitable** (`pnl > 0`) given only the market conditions captured at the moment the trade was entered (the `entryContext` `DowContext` snapshot serialised by the C++ backtester).

The fundamental question it answers:

> *"Given the DowContext snapshot at trade entry — the trend state, trendlines, price/volume statistics, MACD, RSI, and ATR — would this trade have made money?"*

[neural_network.py](neural_network.py) is an **orchestrator**, not a single model. Each run:

1. Loads `output/data.json` and builds a 33-feature vector per trade
2. Splits temporally (oldest 70% train, next 15% val, newest 15% test)
3. Runs a 51-configuration grid search across three neural-network shape families — **Funnel**, **Diamond**, **Cylinder** (17 configs each, all built from `GenericMLP` in [architectures/base.py](architectures/base.py))
4. Trains two gradient-boosting baselines — **XGBoost** and **LightGBM** — via [tree_models.py](tree_models.py)
5. Sorts every entry by AUC-ROC (F1 tiebreaker), prints a leaderboard, and saves the best **neural** model's weights, the fitted scaler, and a JSON metadata file describing the winner

---

## Quick Start

### 1. Install dependencies

```bash
py -3.13 -m pip install -r NeuralNetwork/requirements.txt
```

This installs `torch`, `scikit-learn`, `numpy`, `xgboost`, `lightgbm`.

> **Windows note:** the bare `python` command resolves to the MSYS2 Python used by the C++ build tools, which has no pip and no packages. Always use `py -3.13` (the Windows Python Launcher).

### 2. Generate the training data

The C++ backtester must have been run to produce `output/data.json`. If it does not exist:

```bash
cd BackTrader
make run
```

This runs a base-case strategy sweep and writes every closed position — including the full DowContext snapshot at entry and exit — to `../output/data.json`.

### 3. Run the grid search

From the repo root:

```bash
py -3.13 NeuralNetwork/neural_network.py
```

For a quick development run (50 epochs, patience=7):

```bash
py -3.13 NeuralNetwork/neural_network.py --fast
```

Per-configuration progress is printed:

```
[01/51] funnel/funnel_2L_128              AUC=0.631  F1=0.583  Recall=0.541  Acc=0.618
[02/51] funnel/funnel_3L_128              AUC=0.644  F1=0.601  Recall=0.567  Acc=0.631
...
[51/51] cylinder/cylinder_5L_32           AUC=0.598  F1=0.552  Recall=0.489  Acc=0.601
[tree]  Training XGBoost (scale_pos_weight=1.4912) ...
[tree]  XGBoost done  AUC=0.6712  F1=0.6021  Recall=0.5832  Acc=0.6398
[tree]  Training LightGBM (class_weight balanced) ...
[tree]  LightGBM done AUC=0.6655  F1=0.5984  Recall=0.5810  Acc=0.6371
```

### 4. Reading the leaderboard

After every model is trained, three blocks are printed:

```
==========================================================================================
  ARCHITECTURE GRID SEARCH — TOP 10 OVERALL  (53 configs, sorted by AUC-ROC)
==========================================================================================
  Rank  Family    Config                          Acc     Prec    Recall  F1      AUC-ROC
  ----  --------  ------------------------------  ------  ------  ------  ------  -------
     1  cylinder  cylinder_3L_128                 0.6412  0.6120  0.5870  0.5993  0.6814
     2  xgboost   xgboost_d6_n500                 0.6398  0.6087  0.5832  0.5956  0.6712
  ...

  --- GRADIENT BOOSTING BASELINE ---
  ...

  --- TOP 5: FUNNEL ---
  ...
  --- TOP 5: DIAMOND ---
  ...
  --- TOP 5: CYLINDER ---
  ...
```

- **AUC-ROC** — primary ranking metric. 0.5 = random, 1.0 = perfect. Above 0.65 indicates genuine predictive signal.
- **Recall** — of all profitable trades, how many did the model catch?
- **F1** — best single number when classes are imbalanced (balances precision and recall).
- **Tree-model rows** appear in the overall top-10 alongside neural configs and again grouped in the dedicated baseline block.

### 5. Command-line options

```
py -3.13 NeuralNetwork/neural_network.py [OPTIONS]

  --data PATH       Path to data.json       (default: output/data.json)
  --epochs N        Max epochs per model    (default: 100)
  --batch-size N    Mini-batch size         (default: 64)
  --lr FLOAT        Adam learning rate      (default: 0.001)
  --patience N      Early-stopping patience (default: 15)
  --seed N          Random seed             (default: 42)
  --fast            Quick sweep: epochs=50, patience=7
```

### 6. Export a single MLP-input sample

After at least one run of `neural_network.py` (so `scaler.pkl` exists), you
can dump exactly what the MLP receives as a JSON template:

```bash
py -3.13 NeuralNetwork/export_input_template.py
```

Writes [../output/data_ingestion_template.json](../output/data_ingestion_template.json) — one
training-split row, fully extracted + winsorized + scaled, alongside the
transformation parameters used. See [Normalisation.md](Normalisation.md) for a
step-by-step explanation of every transformation in this file.

Options:

```
py -3.13 NeuralNetwork/export_input_template.py [OPTIONS]

  --data PATH       Path to data.json           (default: output/data.json)
  --scaler PATH     Path to scaler.pkl bundle   (default: NeuralNetwork/scaler.pkl)
  --out PATH        Where to write the template (default: output/data_ingestion_template.json)
  --index N         Row index within the training split (default: 0 → first training row)
```

### 7. File outputs

After a full run, three files are written:

| File | Contents |
|---|---|
| [scaler.pkl](scaler.pkl) | Fitted `StandardScaler` (pickle). Load with `pickle.load(open('NeuralNetwork/scaler.pkl', 'rb'))` to normalise new data for inference. |
| [model.pt](model.pt) | PyTorch state dict of the best-performing **neural** model (tree models are not saved). Load with `model = GenericMLP(input_dim=33, hidden_dims=...); model.load_state_dict(torch.load('NeuralNetwork/model.pt'))`. |
| [model_metadata.json](model_metadata.json) | Family, config name, `hidden_dims`, dropout, and the five test metrics of the winning neural model. |
| [../output/data_ingestion_template.json](../output/data_ingestion_template.json) | One fully-transformed training sample, exactly as `GenericMLP.forward()` receives it, plus the `winsor_lo`/`winsor_hi`/`scaler_mean`/`scaler_std` arrays as plain JSON. Generated by [export_input_template.py](export_input_template.py). |

---

## Data Pipeline

### Source

`output/data.json` is an array of JSON objects, one per closed position, written by `ExecuteAllSweeps()` in [BackTrader/StrategyRunner/DowATRStrategy/DowATRStrategy.h](../BackTrader/StrategyRunner/DowATRStrategy/DowATRStrategy.h) via `Position::toJson()`:

```json
{
  "stockName":        "DC=F",
  "positionType":     "LONG",
  "tradeType":        "LONG BREAKTHROUGH",
  "purchaseDate":     "2006-05-25",
  "sellDate":         "2006-06-16",
  "purchasePrice":    11.26,
  "sellPrice":        11.18,
  "numShares":        88.8,
  "originalStopLoss": 10.94,
  "pnl":              -7.10,
  "entryContext":     { ... DowContext snapshot ... },
  "exitContext":      { ... DowContext snapshot ... }
}
```

### Filtering

A position is **skipped** if `pnl == 0.0` AND `sellDate == ""` — these are trades the backtester opened but never closed before the data ended. Genuine breakeven closes (`pnl == 0.0` with a non-empty `sellDate`) are kept and labelled as non-profitable (`y = 0`).

### Label

```
y = 1  if pnl  > 0    (profitable)
y = 0  if pnl <= 0    (loss or breakeven)
```

### Temporal split (not stratified)

Positions are sorted by `purchaseDate` ascending and sliced sequentially:

```
oldest  ──────────────────────────────────────────────────────────  newest
[============== 70% train ============][== 15% val ==][== 15% test ==]
```

See [neural_network.py:313-330](neural_network.py#L313-L330). This deliberately replaces the stratified random split used previously. **Why temporal:**

- **Prevents future-regime leakage.** A random split lets 2024 trades inform a model that is then tested on 2010 trades, which is impossible in production. A temporal split forces the model to extrapolate to a market regime it has never seen — the only honest estimate of out-of-sample performance.
- The cost is that test-set class balance is not guaranteed. The orchestrator prints test-set balance after splitting so any large deviation is visible.

---

## Feature Vector (33 values, ATR/return-normalized)

### Why only the entry context?

Only `entryContext` is used as input. The `exitContext` is **deliberately excluded** because it contains information that would not have been available at the time the trade was opened — including it would create **data leakage** and inflate accuracy in a way that does not generalise to live trading. `tradeType` is also excluded: it is a human-readable label assigned by the developer and is derived from the same DowContext data the network already sees, so including it would be a redundant authored signal.

### Why stationarity transforms?

The 32-ticker commodity universe spans wildly different natural scales — GC=F trades around $2,000, ZC=F around $5, CL=F volume is ~500,000 contracts/day while LBS=F is ~500. After a global `StandardScaler`, raw features at those different scales become incomparable across tickers; the network spends parameters memorizing per-ticker baselines instead of learning the actual signal. Every feature is converted into a **scale-free, cross-ticker-comparable form**: prices become ATR-normalized distances from `purchasePrice`, slopes are converted to t-statistics (`slope/slopeSE`), volumes are log-mean plus spike ratios (`max/mean`, `min/mean`) plus a coefficient of variation. See [neural_network.py:`extract_context_features`](neural_network.py).

### Why MLP-friendly summary stats instead of per-extremum coordinates?

The earlier 77-feature layout passed five sequential trend extrema (`e1..e5`) as `[days, close_atr, isTrough]` triplets. Missing extrema were zero-padded. A decision tree can learn `if close_atr == 0 then ignore`; an MLP cannot — it interprets `close_atr == 0.0` as "the extremum's close exactly equaled the purchase price" and `days == 0` as "the extremum occurred today," which contradicts the missing-data semantics. The current layout replaces all five triplets with three summary statistics (`duration_days`, `range_atr`, `extrema_count`) that are well-defined regardless of how many extrema exist. Boolean flags (`*Ready`, `isActive`, `slopeSignificant`, `isTrough`) and collinear features (raw `slope` + `slopeSE` + `slopeRSQ` + `slopeSig`; raw `log_min`, `log_max`, `log_std`) were removed for the same reason — rigid 0/1 channels and high redundancy waste MLP capacity.

### Layout (33 values per trade)

| Offset | Group              | Count | Notes |
|-------:|--------------------|------:|-------|
|     0  | positionType (LONG=1, SHORT=0) | 1 | unchanged |
|   1–6  | priceStatistics    |   6   | ATR-normalized distances + CoV + t-statistic |
|  7–11  | volumeStatistics   |   5   | `log1p(mean)`, spike ratios, CoV, t-statistic |
|  12–13 | macd / signal      |   2   | divided by **atrValue** (was previously divided by mean price) |
|  14–16 | trend              |   3   | summary stats: duration, peak-to-trough range, valid-extremum count |
|  17–20 | trendLine          |   4   | slope/mean, ATR-normalized intercept, dateDifference, projected distance |
|  21–23 | doubleTrend        |   3   | same shape as trend, anchored to e1 of the doubleTrend |
|  24–27 | doubleTrendLine    |   4   | same shape as trendLine |
|  28–29 | rsi / doubleRsi    |   2   | already in [0, 100], unchanged |
|  30–32 | atr block          |   3   | atr/price, doubleAtr/price, **vol_ratio = atr/doubleAtr** |
| **Total** |                 | **33** | |

`FEATURE_NAMES` in [neural_network.py](neural_network.py) gives the exact name of each column for diagnostics.

### Price statistics block (6 values)

| Index | Field            | Transform |
|------:|------------------|-----------|
| 0 | price_mean_atr     | `(mean - purchasePrice) / atr` |
| 1 | price_std_atr      | `std / atr` |
| 2 | price_min_atr      | `(min - purchasePrice) / atr` |
| 3 | price_max_atr      | `(max - purchasePrice) / atr` |
| 4 | price_cov          | `std / mean` — coefficient of variation, scale-free relative dispersion |
| 5 | price_t            | `slope / slopeSE` — t-statistic of the regression slope, the single momentum-vs-noise feature that replaces the previous four (slope/mean, slopeSE/mean, slopeRSQ, slopeSig) |

### Volume statistics block (5 values)

| Index | Field             | Transform |
|------:|-------------------|-----------|
| 0 | vol_log_mean       | `log1p(mean)` — baseline magnitude |
| 1 | vol_max_over_mean  | `max / mean` — surface upside spikes |
| 2 | vol_min_over_mean  | `min / mean` — surface downside dry-ups |
| 3 | vol_cov            | `std / mean` — relative dispersion |
| 4 | vol_t              | `slope / slopeSE` — volume-trend t-statistic |

### MACD block (2 values)

| Index | Field        | Transform |
|------:|--------------|-----------|
| 0 | macd_atr       | `macd / atrValue` |
| 1 | signal_atr     | `signal / atrValue` |

ATR-normalization aligns the MACD scale with every other price-momentum feature in the vector (price min/max distances, trendline intercept distances, etc.) so a single first-layer weight can treat all of them uniformly. The `macdReady` flag has been removed.

### Trend block (3 values) — `trend` and `doubleTrend`

| Index | Field                | Meaning |
|------:|----------------------|---------|
| 0 | trend_duration_days    | `(purchaseDate - e1.date).days` — how long the identified trend has been forming |
| 1 | trend_range_atr        | `(max(close) - min(close)) / atr` across valid extrema, `0.0` if fewer than 2 |
| 2 | trend_extrema_count    | Number of valid (`index >= 0`, non-empty date) extrema; integer in `[0, 5]` |

Empty trends (no `e1`) report `[0.0, 0.0, 0.0]`. Note: in the current dataset every trade is opened only after exactly 3 swing points have been identified, so `trend_extrema_count` is constant at `3.0` — `StandardScaler` handles zero-variance features by setting their scale to `1.0`. Different parameter sweeps may exhibit variance.

### Trendline block (4 values) — `trendLine` and `doubleTrendLine`

| Index | Field              | Meaning |
|------:|--------------------|---------|
| 0 | tl_slope_ret         | `slope / priceStatistics.mean` (per-bar return) |
| 1 | tl_intercept_atr     | `(intercept - purchasePrice) / atr` |
| 2 | tl_dateDiff          | Number of calendar days spanned by the line |
| 3 | tl_proj_atr          | `(purchasePrice - (intercept + slope*dateDifference)) / atr` — distance from current price to the trendline's projected current value, in volatility units |

When the trendline is broken (`isActive == false`) or absent, all four values are zeroed. The previous `trendLineReady` and `isActive` flags have been removed.

### Normalisation

All 33 features pass through a configurable scaler:

```
x_clipped = clip(x, lower_pct_bound, upper_pct_bound)   # winsorize, default 1%–99%
x_scaled  = scaler.transform(x_clipped)                  # StandardScaler or RobustScaler
```

CLI flags:
- `--scaler {standard,robust}` — default `standard`. `robust` uses median/IQR for fat-tail resilience.
- `--winsorize <pct>` — default `1.0`. Clip each feature to `[pct, 100-pct]` percentile bounds computed from the train split. Set to `0` to disable.

The scaler **and** the winsorize bounds are fitted **exclusively on the training split**, then both are applied to val and test. The bundle `{scaler, winsor_lo, winsor_hi, scaler_kind, winsorize_pct}` is persisted to [scaler.pkl](scaler.pkl) so inference replays both steps deterministically.

After the Phase A stationarity transforms, the pre-scaling feature ranges have collapsed from a 9-order-of-magnitude spread to roughly 4 orders of magnitude — enough that the choice of scaler matters less than it used to, but RobustScaler is still a sensible default for financial data because of fat-tailed return distributions.

### Feature diagnostics

[feature_diagnostics.py](feature_diagnostics.py) loads the trained model + scaler bundle and writes [feature_report.txt](feature_report.txt) containing:

- **Pearson correlation matrix**, with all pairs at `|r| ≥ 0.95` flagged for redundancy review
- **Permutation importance** — for each feature, shuffle its column in the test set 3× and record the mean AUC-ROC drop. Negative-or-zero drops mark drop candidates
- A concise drop-list of features that did not move the needle

Run after `neural_network.py` has trained at least once: `py -3.13 NeuralNetwork/feature_diagnostics.py`.

---

## Orchestrator Flow

The full pipeline lives in `main()` at [neural_network.py:273-435](neural_network.py#L273-L435):

```
1. Parse CLI args (--epochs, --patience, --lr, --fast, ...)
2. load_positions(data.json)              ← filter + extract 33 features per trade (ATR-normalized)
3. Sort by purchaseDate; slice 70 / 15 / 15
4. normalise(X_train, X_val, X_test)      ← fit StandardScaler on train, persist
5. pos_weight = num_neg_train / num_pos_train       (~1.49)
6. Build train_loader, val_loader (batch=64)
7. for cfg in funnel.get_configs() + diamond.get_configs() + cylinder.get_configs():
       model = GenericMLP(input_dim=33, hidden_dims=cfg["hidden_dims"], dropout=cfg["dropout"])
       train_model(model, train_loader, val_loader, epochs, patience, lr, pos_weight)
       evaluate(model, X_test, y_test)    ← Accuracy / Precision / Recall / F1 / AUC-ROC
       results.append({...})
8. tree_models.run_tree_models(...)       ← XGBoost + LightGBM, same splits, same pos_weight
9. results.sort(key=AUC-ROC, F1)          ← merged ranking of NN + tree
10. print_leaderboard(...)                ← top 10 overall + tree block + per-family top 5
11. Save best NN model.state_dict() -> model.pt
12. Save winner metadata -> model_metadata.json
```

Every NN config shares the same `train_model` runtime in [architectures/base.py:96-179](architectures/base.py#L96-L179) — only `hidden_dims` and `dropout` change.

---

## Architecture Families

All three families instantiate the same [`GenericMLP`](architectures/base.py#L45-L74) class — they differ only in `hidden_dims`. The shape determines what kind of representation the network is forced to learn.

### Funnel — [architectures/funnel/](architectures/funnel/)

- **Shape intuition:** monotonically narrowing — start wider than the input (33) and step down (e.g. `[1024, 256, 64]`).
- **Config space:** 17 configs spanning initial widths 128 / 256 / 512 / 1024 and depths 2–4. Dropout 0.3 (standard) or 0.4 (steep / wide).
- **Learning hypothesis:** **progressive distillation.** Each successive layer compresses the representation, forcing earlier wide layers to expose every weak combination of features and forcing later narrow layers to commit to the few combinations that actually predict profit. A useful representation is whatever survives the squeeze.
- **Best when:** the discriminative signal is the *aggregate* of many small effects rather than a single high-level latent factor.
- **Deep dive:** [architectures/funnel/ARCHITECTURE.md](architectures/funnel/ARCHITECTURE.md) — full configuration table, mathematics, worked numeric trace, and backprop diagram.

### Diamond — [architectures/diamond/](architectures/diamond/)

- **Shape intuition:** compress → expand → compress (e.g. `[32, 256, 128, 32]`). The first layer drops to a tight bottleneck; the middle widens out; the final hidden layer narrows again.
- **Config space:** 17 configs with bottleneck widths 8 / 16 / 32 / 64, peak widths 64 / 128 / 256, and depths 3–5. Dropout 0.2 for tight bottlenecks (≤16), 0.3 elsewhere.
- **Learning hypothesis:** **bottleneck regularisation.** A useful feature must "pay for its bandwidth" — the bottleneck cannot encode every input, so it learns a low-rank summary of the 33 features. The wide middle layer then has space to combine those compressed factors non-linearly before the final compression projects to a decision.
- **Best when:** the raw feature vector is noisy or redundant (which it is — many DowContext fields are zero when trends/trendlines are absent), so pre-filtering pays off.
- **Deep dive:** [architectures/diamond/ARCHITECTURE.md](architectures/diamond/ARCHITECTURE.md).

### Cylinder — [architectures/cylinder/](architectures/cylinder/)

- **Shape intuition:** every hidden layer is the same width (e.g. `[128, 128, 128]`).
- **Config space:** 17 configs covering widths 32 / 64 / 128 / 256 / 512 and depths 1–5. Dropout uniformly 0.3 — depth is the only experimental variable.
- **Learning hypothesis:** **iterative refinement.** With no compression imposed, each layer can refine the representation produced by the previous one rather than reshape it. Constant width makes the Jacobians between layers square, which keeps gradient magnitudes more stable as depth grows — useful for testing whether *depth alone* helps.
- **Best when:** the right representation is reached by repeated small adjustments rather than by re-projecting into a different dimensionality.
- **Deep dive:** [architectures/cylinder/ARCHITECTURE.md](architectures/cylinder/ARCHITECTURE.md).

---

## How Each Architecture Learns

### Shared scaffolding (identical across all 51 NN configs)

Each network is a `GenericMLP` with the same per-layer pattern:

```
For each hidden width w in hidden_dims:
    Linear(in → w)        ← learns weight matrix W and bias b
    BatchNorm1d(w)        ← per-batch normalisation, two learnable rescale params
    ReLU                  ← element-wise max(0, x); introduces non-linearity
    Dropout(p)            ← randomly zeros p fraction of activations during training

Final:
    Linear(in → 1)        ← raw logit (no Sigmoid in the model)
```

Source: [architectures/base.py:58-74](architectures/base.py#L58-L74).

**Forward pass per layer:** `z = x · Wᵀ + b`, then BatchNorm centres `z` to zero mean / unit variance over the mini-batch and rescales with learned `γ, β`, then ReLU clips negatives, then Dropout zeroes a random fraction of outputs.

**Loss:** `BCEWithLogitsLoss(pos_weight=num_neg/num_pos)` ([base.py:119](architectures/base.py#L119)). It applies the sigmoid internally with a numerically stable formulation (`log(1 + exp(-|x|))`) so the model can output unbounded raw logits without `log(0)` blow-ups. The `pos_weight ≈ 1.49` correction penalises a missed profitable trade ~49% more than a missed loss, matching the actual class ratio (~60% loss / ~40% profit).

**Backward pass:** PyTorch autograd walks the chain rule. For each layer `k`, the gradient of the loss with respect to its weights is `∂L/∂W_k = δ_k · a_{k-1}ᵀ`, where `δ_k` is the back-propagated error signal. `loss.backward()` ([base.py:137](architectures/base.py#L137)) populates every parameter's `.grad`, and `optimizer.step()` ([base.py:138](architectures/base.py#L138)) applies the Adam update.

**Optimizer:** Adam (lr=1e-3) — maintains a per-parameter adaptive learning rate using running averages of the first and second moments of the gradient. More stable than vanilla SGD on tabular data with mixed feature scales.

**Scheduler:** `ReduceLROnPlateau(mode="min", patience=5, factor=0.5)` ([base.py:121-123](architectures/base.py#L121-L123)). When validation loss does not improve for 5 epochs, the learning rate is halved — letting the model take coarse steps early and fine steps late.

**Early stopping:** training stops when val loss has not improved for `patience` epochs (default 15; `--fast` lowers it to 7). The model is restored to the weights from its best validation epoch ([base.py:177-178](architectures/base.py#L177-L178)). With scheduler patience=5 and early-stopping patience=15, the LR scheduler gets up to three halving attempts before training is killed.

### How the shapes differ in what they learn

The scaffolding is identical — what changes is the geometry of the gradient flow:

- **Funnel (wide → narrow).** Early wide layers receive distributed gradient signal — many neurons share the responsibility for any one feature combination, so each individual weight gets a small update. The narrowing forces a *concentration* of error: by the last hidden layer, only a few neurons exist, so each one receives a strong, targeted gradient. The net effect: early layers freely explore every weak interaction; late layers commit hard to the few that matter. Loss landscape is forgiving early, sharp late.

- **Diamond (compress → expand → compress).** The bottleneck layer is the gradient bottleneck too — every gradient flowing back to the input layer must pass through it. This *forces feature competition*: a feature that does not survive the bottleneck cannot influence the output, so the bottleneck weights become an implicit feature selector. The wide middle layer sees only the surviving signals and has plenty of capacity to combine them non-linearly. The final compression chooses the decision-relevant combinations. This is why diamond is most useful when the input is noisy/redundant: the bottleneck learns "what to pay attention to" before the wide layer learns "how to combine it."

- **Cylinder (constant width).** Because every layer has the same width, the per-layer Jacobian is square, and the gradient magnitude is more stable from layer to layer. Deep cylinders behave like an iterative refinement loop — each layer learns a small correction to the previous representation rather than a different projection of it. The risk is that a too-narrow constant width leaves no room for the network to disentangle features at all (configs with width=32 or 1 layer are intentionally included as floor cases); the reward is that depth can be increased without the gradient instability that would plague a deep funnel or deep diamond.

For per-shape worked numeric examples and step-by-step backprop walkthroughs, see each family's `ARCHITECTURE.md`.

---

## Gradient-Boosting Baselines

In addition to the 51 neural configs, [tree_models.py](tree_models.py) trains two strong tabular baselines on the **same splits** with the **same `pos_weight`**:

| Model | Key hyperparameters | Source |
|-------|---------------------|--------|
| **XGBoost** (`xgboost_d6_n500`) | `n_estimators=500, max_depth=6, learning_rate=0.05, scale_pos_weight=pos_weight (~1.49), subsample=0.8, colsample_bytree=0.8, early_stopping_rounds=20` on val | [tree_models.py:57-69](tree_models.py#L57-L69) |
| **LightGBM** (`lightgbm_l63_n500`) | `n_estimators=500, num_leaves=63, learning_rate=0.05, class_weight="balanced", subsample=0.8, colsample_bytree=0.8`, val eval set | [tree_models.py:85-99](tree_models.py#L85-L99) |

Their results are merged into the leaderboard ([neural_network.py:393-399](neural_network.py#L393-L399)) and printed in a dedicated **GRADIENT BOOSTING BASELINE** block. **Why include them:** gradient-boosted trees are notoriously strong on tabular data — they set a floor that any neural config has to clear to be worth its added complexity. If the best NN cannot beat XGBoost on AUC-ROC, the right move is usually to ship the tree model. The orchestrator still saves the best **neural** config to `model.pt` (tree results have `model=None`, so the save logic at [neural_network.py:410](neural_network.py#L410) skips them).

---

## Training Procedure

### Loss: `BCEWithLogitsLoss` with `pos_weight`

```
pos_weight = num_neg_train / num_pos_train   ≈ 1.49
loss       = BCEWithLogitsLoss(pos_weight=pos_weight)(logit, y)
```

Combines a sigmoid and binary cross-entropy in a single numerically stable formula. With `pos_weight = 1.49`, the loss for a missed profitable trade (`y=1` predicted as 0) is multiplied by 1.49 — directly compensating for the ~60/40 class imbalance. The earlier `BCELoss` without weighting drove the model to predict "loss" almost everywhere (Recall ≈ 0.006, AUC-ROC ≈ 0.49 — near random).

### Why raw logits (no Sigmoid in the model)

`BCEWithLogitsLoss` applies sigmoid internally using `log(1 + exp(-|x|))`, which is stable for very confident inputs. Putting an explicit `Sigmoid()` before `BCELoss` produces values close to 0 or 1, and `log(near_0)` blows up. Sigmoid is applied only at inference: `torch.sigmoid(logits)` → threshold at 0.5.

### Optimizer: Adam (lr=1e-3)

Per-parameter adaptive learning rate using the running first and second moments of the gradient. Less sensitive to learning-rate choice than SGD, handles sparse gradients (many DowContext flags are 0 when trends/trendlines are absent), and is the standard starting point for tabular MLPs.

### LR scheduler: `ReduceLROnPlateau(patience=5, factor=0.5)`

Halves the learning rate when validation loss stops improving for 5 consecutive epochs. Lets the model take large steps early to find a good basin and small steps late to converge inside it.

### Early stopping

| Mode    | Early-stopping patience | Scheduler patience | Halving attempts before stop |
|---------|------------------------:|-------------------:|-----------------------------:|
| default |                      15 |                  5 |                            3 |
| `--fast`|                       7 |                  5 |                            1 |

Best-validation weights are restored when training ends.

### Mini-batches

Updates run once per mini-batch of 64. Gradient noise from small batches helps escape local minima; many updates per epoch (~74 for ~4,700 train samples) makes early learning fast; batch size ≥ 32 is enough for stable BatchNorm statistics.

---

## Interpreting the Output

After every model is trained, the leaderboard prints accuracy, precision, recall, F1, and AUC-ROC for the test split.

| Metric | Interpretation |
|---|---|
| **Accuracy > 0.55** | Better than majority-class guessing; the model has found *some* signal |
| **AUC-ROC > 0.65** | Meaningful separation of profitable from loss trades regardless of threshold |
| **F1 > 0.60** | Reasonable balance — catching profitable trades without too many false positives |
| **Precision vs Recall** | Higher precision = fewer false alarms; higher recall = fewer missed winners. Trading-cost economics decide which matters more |

### What the model cannot tell you

- It predicts whether a trade *configuration* is historically associated with profit. It does **not** guarantee future performance.
- Market regimes change. A model trained on 2006–2024 may behave differently in a new regime — this is exactly why the test split is the **most recent** 15% of trades, not a random sample.
- It does not account for correlation between trades opened on the same ticker or in overlapping time windows.

---

## Directory Map

```
NeuralNetwork/
├── neural_network.py            ← orchestrator (run this)
├── tree_models.py               ← XGBoost + LightGBM baselines
├── requirements.txt
├── scaler.pkl                   ← (generated) fitted StandardScaler
├── model.pt                     ← (generated) best NN state dict
├── model_metadata.json          ← (generated) winner family/config/metrics
├── ARCHITECTURE.md              ← this file
└── architectures/
    ├── base.py                  ← GenericMLP, train_model, evaluate, make_loader, TrainResult
    ├── funnel/
    │   ├── funnel.py            ← 17 configs (monotonically narrowing)
    │   └── ARCHITECTURE.md      ← design rationale, math, worked example, backprop
    ├── diamond/
    │   ├── diamond.py           ← 17 configs (compress → expand → compress)
    │   └── ARCHITECTURE.md
    └── cylinder/
        ├── cylinder.py          ← 17 configs (constant width)
        └── ARCHITECTURE.md
```
