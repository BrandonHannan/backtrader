# Neural Network — Architecture & Guide

## Overview

This neural network predicts whether a trade will be **profitable** (`pnl > 0`) given only the market conditions that existed **at the moment the trade was entered**. It is a binary classifier trained on closed positions produced by the C++ backtester.

The fundamental question it answers:

> *"Given the DowContext snapshot at trade entry — the trend state, trendlines, price statistics, and MACD — would this trade have made money?"*

`neural_network.py` is an **orchestrator**: it runs a 51-configuration grid search across three architecture families (Funnel, Diamond, Cylinder), prints a ranked leaderboard, and saves the best model automatically.

---

## Quick Start

### 1. Install dependencies

```bash
py -3.13 -m pip install -r NeuralNetwork/requirements.txt
```

This installs: `torch`, `scikit-learn`, `numpy`.

> **Windows note:** The bare `python` command resolves to the MSYS2 Python used by the C++ build tools, which has no pip and no packages. Always use `py -3.13` (the Windows Python Launcher) to run this script.

### 2. Generate the training data

The C++ backtester must have already been run to produce `output/data.json`. If it doesn't exist:

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

Progress is printed for each of the 51 configurations:

```
[01/51] funnel/funnel_2L_128              AUC=0.631  F1=0.583  Recall=0.541  Acc=0.618
[02/51] funnel/funnel_3L_128              AUC=0.644  F1=0.601  Recall=0.567  Acc=0.631
...
[51/51] cylinder/cylinder_5L_32           AUC=0.598  F1=0.552  Recall=0.489  Acc=0.601
```

### 4. Reading the leaderboard

After all 51 runs complete, a ranked table is printed:

```
==========================================================================================
  ARCHITECTURE GRID SEARCH — TOP 10 OVERALL  (51 configs, sorted by AUC-ROC)
==========================================================================================
  Rank  Family    Config                          Acc     Prec    Recall  F1      AUC-ROC
  ----  --------  ------------------------------  ------  ------  ------  ------  -------
     1  cylinder  cylinder_3L_128                 0.6412  0.6120  0.5870  0.5993  0.6814
     2  funnel    funnel_3L_256                   0.6341  0.6012  0.5712  0.5858  0.6729
  ...

  --- TOP 5: FUNNEL ---
  ...

  --- TOP 5: DIAMOND ---
  ...

  --- TOP 5: CYLINDER ---
  ...
```

- **AUC-ROC** — primary ranking metric. 0.5 = random, 1.0 = perfect. Values above 0.65 indicate genuine predictive signal.
- **Recall** — critical for this use case: of all profitable trades, how many did the model catch?
- **F1** — balances precision and recall; the most useful single number when classes are imbalanced.

### 5. Command-line options

```
py -3.13 NeuralNetwork/neural_network.py [OPTIONS]

  --data PATH       Path to data.json      (default: output/data.json)
  --epochs N        Max epochs per model   (default: 100)
  --batch-size N    Mini-batch size        (default: 64)
  --lr FLOAT        Adam learning rate     (default: 0.001)
  --patience N      Early stopping patience (default: 15)
  --seed N          Random seed            (default: 42)
  --fast            Quick sweep: epochs=50, patience=7
```

### 6. File outputs

After a full run, three files are written:

- `NeuralNetwork/scaler.pkl` — the fitted StandardScaler (for inference)
- `NeuralNetwork/model.pt` — best model's PyTorch state dict
- `NeuralNetwork/model_metadata.json` — family, config name, hidden_dims, dropout, and test metrics of the best model

---

## Architecture Families

### Directory structure

```
NeuralNetwork/
├── neural_network.py          ← orchestrator (run this)
├── architectures/
│   ├── base.py                ← GenericMLP, train_model, evaluate, make_loader
│   ├── funnel/
│   │   ├── funnel.py          ← 17 configs (monotonically narrowing)
│   │   └── ARCHITECTURE.md   ← detailed design docs + worked example
│   ├── diamond/
│   │   ├── diamond.py         ← 17 configs (compress → expand → compress)
│   │   └── ARCHITECTURE.md
│   └── cylinder/
│       ├── cylinder.py        ← 17 configs (constant width)
│       └── ARCHITECTURE.md
```

### Funnel — `architectures/funnel/`
Starts wider than the input (64) and narrows steadily. Tests initial widths of 128/256/512/1024 with 2–4 hidden layers. Best when the useful decision signal emerges naturally from compression. See [architectures/funnel/ARCHITECTURE.md](architectures/funnel/ARCHITECTURE.md).

### Diamond — `architectures/diamond/`
Compresses to a tight bottleneck first, then expands wide in the middle, then compresses back. Bottlenecks range from 8–64 neurons; peaks from 64–256. Best when the features are noisy/redundant and useful interactions only emerge after pre-filtering. See [architectures/diamond/ARCHITECTURE.md](architectures/diamond/ARCHITECTURE.md).

### Cylinder — `architectures/cylinder/`
All hidden layers are the same width. Tests widths of 32/64/128/256/512 with 1–5 layers. Makes no assumption about where the useful signal forms — lets depth be the primary variable. Best when patterns require multi-step refinement without compression. See [architectures/cylinder/ARCHITECTURE.md](architectures/cylinder/ARCHITECTURE.md).

---

## Key Design Decisions (vs. original single-model approach)

### 1. Class imbalance correction (`pos_weight`)
The original model used `BCELoss` which treats both classes equally. With ~40% profitable / ~60% loss trades, this caused the model to mostly predict "loss" (near-random AUC-ROC ≈ 0.49).

The new approach computes:
```
pos_weight = num_loss_training_samples / num_profitable_training_samples  ≈ 1.49
```
And passes it to `BCEWithLogitsLoss`. This penalises missing a profitable trade ~49% more than incorrectly predicting one — matching the actual class ratio.

### 2. Raw logit output (no Sigmoid in model)
The model now outputs a raw logit. `BCEWithLogitsLoss` applies sigmoid internally using a numerically stable formula. Sigmoid is only applied at inference time. This prevents the numerical instability that occurs when the model outputs a very confident sigmoid probability (near 0 or 1) and then BCE takes `log(near_0)`.

### 3. Learning rate scheduler
`ReduceLROnPlateau(patience=5, factor=0.5)` halves the learning rate when validation loss stops improving. This lets the model make coarser updates early and finer updates later, recovering from stall points.

### 4. Early stopping patience increased to 15
The original patience was 10. With the LR scheduler needing 5 epochs to react and then the model needing time to recover, patience=15 gives the scheduler room to work before training is stopped.

---

## Data Pipeline

### Source

`output/data.json` is an array of JSON objects, one per closed position. Each object was written by `ExecuteAllSweeps()` in `BackTrader/StrategyRunner/DowATRStrategy/DowATRStrategy.h` and serialised via `Position::toJson()`.

A position contains:

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

Positions are skipped if `pnl == 0.0` **and** `sellDate` is empty — these represent trades the backtester opened but never closed before the data ended. Trades where `pnl == 0.0` but `sellDate` is set (genuine breakeven trades) are kept and labelled as non-profitable (`y = 0`).

### Label

```
y = 1  if pnl > 0  (profitable)
y = 0  if pnl <= 0 (loss or breakeven)
```

---

## Feature Engineering

### Why only entry context?

Only `entryContext` is used as input. The `exitContext` is intentionally excluded because it contains information that would not have been available at the time the trade decision was made. Using it would create **data leakage** — the model would be learning from the future, producing inflated accuracy that would not generalise to live trading.

### Feature vector layout (64 values)

Each position is reduced to a single flat array of 64 floating-point numbers:

```
Index   Group                   Count   Description
-----   -----                   -----   -----------
0       positionType            1       LONG=1.0 / SHORT=0.0
1–8     priceStatistics         8       See below
9–16    volumeStatistics        8       See below
17      macd                    1       MACD line value
18      signal                  1       MACD signal line value
19      macdReady               1       1.0 if MACD has enough history, else 0.0
20      trendReady              1       1.0 if primary trend is confirmed
21      trend.type              1       UPTREND=1.0 / NONE=0.0 / DOWNTREND=-1.0
22–24   trend.e1                3       [index, close, isTrough]
25–27   trend.e2                3       [index, close, isTrough]
28–30   trend.e3                3       [index, close, isTrough]
31–33   trend.e4                3       [index, close, isTrough]  (0.0 if absent)
34–36   trend.e5                3       [index, close, isTrough]  (0.0 if absent)
37–41   trendLine               5       See below
42      doubleTrendReady        1       1.0 if double trend is confirmed
43      doubleTrend.type        1       UPTREND=1.0 / NONE=0.0 / DOWNTREND=-1.0
44–58   doubleTrend.e1–e5       15      Same layout as trend extremums
59–63   doubleTrendLine         5       See below
```

**Statistics block (8 values)** — applies to both `priceStatistics` and `volumeStatistics`:

| Index (within block) | Field | Meaning |
|---|---|---|
| 0 | mean | Rolling average of close / volume |
| 1 | std | Rolling standard deviation |
| 2 | min | Rolling minimum |
| 3 | max | Rolling maximum |
| 4 | slope | Linear regression slope over the lookback window |
| 5 | slopeSE | Standard error of the slope |
| 6 | slopeRSQ | R² of the linear fit |
| 7 | slopeSignificant | 1.0 if slope is statistically significant, else 0.0 |

**Trendline block (5 values)** — applies to both `trendLine` and `doubleTrendLine`:

| Index (within block) | Field | Meaning |
|---|---|---|
| 0 | trendLineReady | 1.0 if trendline has been established |
| 1 | isActive | 1.0 if price has not broken the line |
| 2 | slope | Rise per bar of the trendline |
| 3 | intercept | Price at the current bar's x-position |
| 4 | dateDifference | Number of calendar days spanned by the line |

**Extremum block (3 values per extremum, 5 extremums per trend)**:

| Index (within block) | Field | Meaning |
|---|---|---|
| 0 | index | Bar index in the data window (-1 = not yet set) |
| 1 | close | Closing price at this swing point |
| 2 | isTrough | 1.0 = swing low, 0.0 = swing high |

### Why `tradeType` is excluded

`tradeType` is a human-readable string (e.g. `"LONG BREAKTHROUGH"`) assigned by the developer for identification. It encodes the developer's own classification of the trade, which is derived from the same DowContext data already present in the feature vector. Including it would introduce a redundant signal that is semantically authored, not independently measured, and could cause the network to overfit to label strings rather than learning from market structure.

### Normalisation

All 64 features are passed through a `StandardScaler` before training:

```
x_scaled = (x - mean) / std
```

The scaler is fitted exclusively on the **training split** and then applied to the validation and test splits. This is critical — if the scaler saw the test data, the test set would no longer be a true holdout and accuracy numbers would be optimistic.

**Why StandardScaler over MinMaxScaler?**

The features in this dataset include values with very different natural ranges: extremum indices (0–2000+), prices (0.5–500+), slopes (near-zero), R² values (0–1), and binary flags (0 or 1). MinMax scaling compresses everything to [0, 1] but is highly sensitive to outliers — a single anomalous price spike would compress all other values into a tiny band. StandardScaler is more robust because it centres data around zero and scales by spread, so outliers have less destructive effect on the majority of values. It also works better with ReLU activations, which learn best from inputs centred near zero.

---

## Model Architecture

### The chosen model: Multi-Layer Perceptron (MLP)

```
Input layer      64 neurons    (one per normalised feature)
        |
  Linear(64→128)               Learnable weight matrix W₁ (64×128) + bias b₁
  BatchNorm1d(128)              Normalises activations within each mini-batch
  ReLU                         Non-linearity: max(0, x)
  Dropout(p=0.3)               Randomly zeroes 30% of neurons during training
        |
  Linear(128→64)               Learnable weight matrix W₂ (128×64) + bias b₂
  BatchNorm1d(64)
  ReLU
  Dropout(p=0.3)
        |
  Linear(64→1)                 Learnable weight matrix W₃ (64×1) + bias b₃
  Sigmoid                      Squashes output to probability in [0, 1]
        |
Output           1 value       P(trade is profitable)
```

A prediction of ≥ 0.5 is classified as profitable (`y = 1`); < 0.5 is classified as a loss (`y = 0`).

### Why an MLP?

An MLP was selected over the alternatives below because it is the best match for this specific data and problem:

**Vs. Logistic Regression**
Logistic regression fits a single linear boundary through the 66-dimensional feature space. Market profitability is unlikely to be linearly separable — for example, a trendline slope that predicts profit in an uptrend may predict loss in a downtrend. The MLP learns non-linear combinations of features through its hidden layers, capturing these conditional relationships. The two hidden layers give it enough capacity without being overparameterised.

**Vs. Random Forest / Gradient Boosting (e.g. XGBoost)**
Tree-based ensembles are strong competitors for tabular data and would likely perform comparably here. An MLP was chosen because:
1. The feature space contains meaningful *continuous* relationships (e.g. trendline slope + R² together convey strength of trend). Trees split features independently and must implicitly reconstruct these relationships through many splits. Linear layers learn weighted combinations directly.
2. It is a natural foundation for later extension — for instance, processing the entry context with an LSTM if the context is ever provided as a time series rather than a summary snapshot.
3. Consistency with the existing Python tooling in this repo (no new non-Python dependency).

**Vs. Convolutional Networks (CNN)**
CNNs learn spatial patterns in grid-structured data (images, sequences). The 66-element feature vector here has no spatial topology — adjacent features are not necessarily related. A CNN would impose a structure that does not exist.

**Vs. Recurrent Networks (LSTM / GRU)**
LSTMs process sequences where order matters. The current input to this network is a single fixed-length snapshot, not a time series. An LSTM would be appropriate if the model received the raw OHLCV bars over the lookback window directly, but that would require a different data format from the C++ engine. If the data format is extended in the future, migrating to an LSTM encoder + MLP head would be a natural upgrade.

### Layer-by-layer explanation

**Linear layers**
Each `nn.Linear(in, out)` learns a matrix of weights and a bias vector. During the forward pass it computes `output = input @ W.T + b`. The weights are updated during backpropagation to minimise the loss. The widths (128, 64) follow a funnel pattern — each layer compresses the representation, forcing the network to distil the most predictive information.

**Batch Normalisation**
After each linear layer, `BatchNorm1d` normalises the activations across the mini-batch: it subtracts the batch mean and divides by the batch standard deviation, then applies two learnable rescaling parameters (gamma, beta). This solves the **internal covariate shift** problem — as weights update during training, the distribution of activations seen by the next layer keeps shifting, which slows convergence. BatchNorm re-centres activations at each layer, allowing higher learning rates and more stable training. It also adds a mild regularisation effect.

**ReLU activation**
`ReLU(x) = max(0, x)` introduces non-linearity. Without it, stacking multiple linear layers is equivalent to a single linear layer — no additional expressiveness is gained. ReLU was chosen over sigmoid or tanh for hidden layers because it does not saturate for positive inputs, so gradients flow cleanly during backpropagation (the **vanishing gradient** problem that plagues deep sigmoid networks is not present). ReLU is also computationally trivial.

**Dropout**
During training, `Dropout(p=0.3)` randomly sets 30% of neuron outputs to zero on each forward pass. This forces the network to learn redundant representations — no single neuron can be relied upon — which dramatically reduces overfitting. During evaluation (`model.eval()`), dropout is disabled and all neurons contribute. With only ~6,700 training samples, overfitting is a real risk; dropout is the primary guard against it.

**Sigmoid output**
The final `nn.Sigmoid()` maps the raw output (any real number) to the range [0, 1], which is interpreted as the probability the trade is profitable. This is required for `BCELoss` to be mathematically valid.

---

## Training Procedure

### Loss function: Binary Cross-Entropy (BCELoss)

```
BCE = -[y * log(p) + (1 - y) * log(1 - p)]
```

Where `y` is the true label (0 or 1) and `p` is the predicted probability. When the model is confident and correct (p close to 1 when y=1, or p close to 0 when y=0), the loss is near zero. When the model is confident and wrong, the loss is large (log of a near-zero number). This asymmetric penalty drives the model toward calibrated probabilities, not just correct rankings.

**Why not MSE?** Mean squared error treats the output as a continuous value and penalises distance. For binary classification the output is a probability, and BCE is the theoretically correct loss derived from maximum likelihood estimation under a Bernoulli distribution.

### Optimiser: Adam

Adam (Adaptive Moment Estimation) maintains a per-parameter learning rate, scaled by the running average of past gradients (first moment) and squared gradients (second moment). Compared to vanilla SGD:

- Converges faster because it adapts the learning rate to each parameter
- Less sensitive to the initial learning rate choice
- Handles sparse gradients gracefully (relevant for features like `macdReady` that are 0 for many samples)

The default learning rate of `0.001` is the standard starting point for Adam on tabular data.

### Data splits

```
Full dataset  (N positions)
      |
      +-- 70% Training set    — used to update weights
      |
      +-- 15% Validation set  — used to monitor generalisation during training
      |
      +-- 15% Test set        — held out entirely; used only for final evaluation
```

Splitting is **stratified** — each split has the same proportion of profitable / non-profitable trades as the full dataset. Without stratification, a random split could put most of the profitable trades in training and leave the test set unrepresentative.

The test set is never seen by the model or the scaler until the final `evaluate()` call. This makes the reported metrics an honest estimate of how well the model would perform on future unseen trades.

### Early stopping

```python
if val_loss < best_val_loss:
    save best weights
    reset patience counter
else:
    increment patience counter
    if patience counter >= 10:
        stop training, restore best weights
```

Training stops if the validation loss has not improved for 10 consecutive epochs. The model is then restored to the weights from its best validation epoch. This prevents overfitting without requiring manual tuning of the epoch count — the network trains for exactly as long as it is learning something useful.

### Mini-batches

Weights are updated once per mini-batch of 64 samples, not once per epoch. This means:
- Gradients are estimated from 64 samples, introducing noise that helps escape local minima
- Parameters update many times per epoch (~74 updates per epoch for 4,700 training samples), so learning is fast
- BatchNorm requires a reasonable batch size to compute stable statistics; 64 is sufficient

---

## Interpreting the Output

After training completes, the script prints:

```
--- Test Results ---
  Accuracy  : 0.XXXX
  Precision : 0.XXXX
  Recall    : 0.XXXX
  F1        : 0.XXXX
  AUC-ROC   : 0.XXXX

  Class balance: NNN profitable / NNN loss  (NNN total)
```

### What to look for

| Metric | Interpretation |
|---|---|
| **Accuracy > 0.55** | Better than guessing; the model has found a signal |
| **AUC-ROC > 0.65** | Meaningful separation of profitable vs loss trades |
| **F1 > 0.60** | Reasonable balance of catching profitable trades without too many false positives |
| **Precision vs Recall tradeoff** | High precision = fewer false alarms; high recall = fewer missed winners. Depending on trading costs, one may matter more than the other |

### What the model cannot tell you

- The model predicts whether a trade configuration is historically associated with profit. It does not guarantee future performance.
- Market regimes change. A model trained on 2006–2024 data may behave differently in a new regime.
- The model does not account for correlation between trades on the same ticker or time period.

---

## File Outputs

| File | Contents |
|---|---|
| `NeuralNetwork/scaler.pkl` | Fitted `StandardScaler` serialised with pickle. Load with `pickle.load(open('NeuralNetwork/scaler.pkl', 'rb'))` to normalise new data for inference. |
| `NeuralNetwork/model.pt` | PyTorch state dict (weights only). Load with `model = TradeMLP(64); model.load_state_dict(torch.load('NeuralNetwork/model.pt'))`. |
