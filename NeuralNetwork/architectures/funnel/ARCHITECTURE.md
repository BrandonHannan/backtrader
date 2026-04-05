# Funnel Architecture

## 1. Design Rationale

The funnel is the most intuitive neural network shape for classification problems.
It starts with a wide first layer (more neurons than the input) to give the network
plenty of room to learn different combinations of the raw features, then narrows
steadily toward the output. Each layer has fewer neurons than the one before it,
forcing the network to progressively compress information — keeping only what is
useful for predicting whether a trade will be profitable.

**Why this fits trading data:** The 64 input features include a lot of overlapping
information (e.g., price slope and MACD both track momentum; multiple extremum
points describe the same trend). The funnel lets the wide first layer learn many
different views of the data, then subsequent layers vote on which views actually
predict profitability.

**What it tests:** Does giving the network more initial capacity (wider first layer)
improve learning? Do deeper funnels (more compression stages) help or just overfit?

---

## 2. Shape Diagram

Three representative configurations shown as horizontal bars:

```
funnel_2L_128      funnel_3L_256          funnel_4L_512
(2 hidden layers)  (3 hidden layers)      (4 hidden layers)

[======] 64 in     [======] 64 in         [======] 64 in
[============] 128 [====================] 256   [========================================] 512
[======] 64        [============] 128            [====================] 256
[=] 1 out          [======] 64                   [============] 128
                   [=] 1 out                     [======] 64
                                                 [=] 1 out
```

Each `[===]` bar is proportional to the number of neurons in that layer.

---

## 3. Mathematics (Plain English)

### Forward pass through one hidden layer

Given input vector **x** (64 numbers), a single layer does four things:

**Step 1 — Linear transform:**
```
z = W · x + b
```
- **W** is a weight matrix (e.g., 128 × 64 for the first funnel layer)
- **b** is a bias vector (128 numbers, one per neuron)
- Each row of W is one neuron's "recipe" — how much attention to pay to each input
- Result **z** is 128 numbers, one activation signal per neuron

**Step 2 — Batch Normalisation:**
```
z_norm = (z - μ) / σ  ×  γ  +  β
```
- μ = mean of z across the current mini-batch of trades
- σ = standard deviation of z across the mini-batch
- γ, β = learned scale/shift parameters (the model adjusts these)
- **Why:** Without this, large values in z can make training unstable. Normalising
  keeps all activations in a similar range so every neuron learns at a similar speed.

**Step 3 — ReLU activation:**
```
h = max(0, z_norm)
```
- Any negative value becomes 0; positive values pass through unchanged.
- **Why:** This is the non-linearity. Without it, stacking linear layers is
  mathematically identical to just one linear layer — you'd get no benefit from depth.
  ReLU lets the network learn curves and thresholds, not just straight lines.

**Step 4 — Dropout (training only):**
- Randomly zeros out ~30% of neurons in h (or 40% for steep funnels).
- **Why:** Forces the network to not rely on any single neuron. Acts like training
  many slightly different networks simultaneously, which improves generalisation.
- At inference (test) time, all neurons are active and their outputs are scaled down
  by the dropout probability to match the training-time expected magnitude.

### Output layer
The final linear layer produces one number (the raw **logit**):
```
logit = W_out · h_last + b_out
```

The loss function (`BCEWithLogitsLoss`) converts this internally:
```
probability = σ(logit) = 1 / (1 + e^(-logit))
```
- logit > 0  →  probability > 0.5  →  predicted "profitable"
- logit < 0  →  probability < 0.5  →  predicted "loss"

---

## 4. Configuration Table

| # | Name | hidden_dims | Dropout | Approx params |
|---|------|-------------|---------|---------------|
| 1 | funnel_2L_128 | [128, 64] | 0.3 | 12,929 |
| 2 | funnel_3L_128 | [128, 64, 32] | 0.3 | 15,073 |
| 3 | funnel_4L_128 | [128, 64, 32, 16] | 0.3 | 15,633 |
| 4 | funnel_2L_256 | [256, 128] | 0.3 | 49,537 |
| 5 | funnel_3L_256 | [256, 128, 64] | 0.3 | 57,985 |
| 6 | funnel_4L_256 | [256, 128, 64, 32] | 0.3 | 60,129 |
| 7 | funnel_2L_256_steep | [256, 64] | 0.4 | 33,345 |
| 8 | funnel_2L_512 | [512, 256] | 0.3 | 165,889 |
| 9 | funnel_3L_512 | [512, 256, 128] | 0.3 | 199,041 |
| 10 | funnel_4L_512 | [512, 256, 128, 64] | 0.3 | 207,489 |
| 11 | funnel_2L_512_steep | [512, 128] | 0.4 | 99,201 |
| 12 | funnel_2L_512_vsteep | [512, 64] | 0.4 | 66,113 |
| 13 | funnel_3L_256_steep | [256, 128, 32] | 0.4 | 50,465 |
| 14 | funnel_3L_512_steep | [512, 256, 64] | 0.4 | 182,529 |
| 15 | funnel_2L_128_steep | [128, 32] | 0.4 | 12,449 |
| 16 | funnel_3L_1024 | [1024, 512, 256] | 0.3 | 789,249 |
| 17 | funnel_3L_1024_steep | [1024, 256, 64] | 0.4 | 395,585 |

*Param count ≈ Σ(in × out + out) for linear layers + 4 × width for each BatchNorm.*

---

## 5. Workflow Diagram — Full Traced Example

This traces one trade through `funnel_3L_256` (hidden_dims = [256, 128, 64]).

### The trade's input features (64 numbers)
```
x = [1.0,        ← positionType = LONG
     48234.5,    ← price mean
     1203.2,     ← price std
     ...         ← 62 more context features]
```
After StandardScaler normalisation (mean=0, std=1 per feature):
```
x_norm = [0.42, -1.10, 0.87, 0.05, -0.33, 1.21, ...]   (64 numbers, all roughly -3 to +3)
```

---

### Layer 1: Linear(64 → 256)

**Weights W₁** (256 × 64 matrix — only a 2×3 slice shown):
```
W₁ = [[ 0.12, -0.08,  0.31, ...],   ← neuron 1's weights
      [-0.22,  0.45, -0.17, ...],   ← neuron 2's weights
      ...                            ← 254 more neurons
     ]

b₁ = [0.01, -0.03, ...]              ← 256 biases
```

**Linear transform:**
```
z₁ = W₁ · x_norm + b₁
   = [ 0.12×0.42 + (-0.08)×(-1.10) + 0.31×0.87 + ... + 0.01,   ← neuron 1 → z = 1.34
       -0.22×0.42 + 0.45×(-1.10) + (-0.17)×0.87 + ... + (-0.03), ← neuron 2 → z = -0.89
       ... ]                                                         (256 numbers total)
```

**BatchNorm** (normalise z₁ across the batch, then scale/shift):
```
μ = mean(z₁ across all 64 trades in the batch)  e.g. 0.02
σ = std(z₁ across the batch)                    e.g. 1.15
z₁_norm = (z₁ - 0.02) / 1.15 × γ + β            (γ, β learned; start near 1 and 0)
        = [1.15, -0.79, ...]
```

**ReLU** (zero out negatives):
```
h₁ = max(0, z₁_norm) = [1.15,  0.0,  ...]
                               ↑ neuron 2 was negative (-0.79), now silenced
```

**Dropout** (training: randomly zero 30% of h₁):
```
h₁_dropped = [1.15, 0.0, 0.0, 0.93, ...]   ← some neurons randomly zeroed
```

---

### Layer 2: Linear(256 → 128)  →  same operations as Layer 1

### Layer 3: Linear(128 → 64)   →  same operations

### Output: Linear(64 → 1)
```
logit = W_out · h₃ + b_out = 1.23
```

**Sigmoid** (applied by loss function or at inference):
```
P(profitable) = 1 / (1 + e^(-1.23)) = 1 / (1 + 0.292) = 0.77
```
Since 0.77 ≥ 0.5  →  **predicted: profitable**

---

### How Weights Change During Training (Backpropagation)

Suppose the actual label for this trade is **0** (it was a loss trade).
The network predicted 0.77 (profitable) — that's wrong.

**Step 1 — Compute loss:**
```
L = BCEWithLogitsLoss(logit=1.23, label=0)
  = log(1 + e^(1.23))         ← cross-entropy for negative label
  = log(1 + 3.42)
  = log(4.42)
  = 1.487
```
Higher loss = more wrong. The goal is to make L as small as possible.

**Step 2 — Compute gradients (chain rule):**

The gradient ∂L/∂W tells us: "if I increase this weight slightly, how much does the loss increase?"

```
∂L/∂logit = σ(logit) - label = 0.77 - 0 = 0.77

Then chain backward through each layer:
∂L/∂W_out  =  ∂L/∂logit × h₃ᵀ           (output layer)
∂L/∂W₃     =  ∂L/∂h₃ × (ReLU mask) × h₂ᵀ
∂L/∂W₂     =  ∂L/∂h₂ × (ReLU mask) × h₁ᵀ
∂L/∂W₁     =  ∂L/∂h₁ × (ReLU mask) × x_normᵀ
```

The "ReLU mask" is 1 where h > 0 and 0 where h was zeroed — gradients only
flow back through neurons that were "on".

**Step 3 — Update weights (Adam optimiser):**

Vanilla gradient descent would be:
```
W_new = W_old - lr × ∂L/∂W

Example for one weight:  W_old = 0.12,  ∂L/∂W = 0.05,  lr = 0.001
→  W_new = 0.12 - 0.001 × 0.05 = 0.11995
```

**Adam goes further** — it adapts the effective learning rate per weight:
```
m = 0.9 × m_prev + 0.1 × ∂L/∂W        ← momentum: smoothed gradient history
v = 0.999 × v_prev + 0.001 × (∂L/∂W)² ← velocity: smoothed squared gradient
m̂ = m / (1 - 0.9^t)                    ← bias correction for early steps
v̂ = v / (1 - 0.999^t)

W_new = W_old - lr × m̂ / (√v̂ + 1e-8)
```

- Weights that have consistently large gradients get a **smaller** effective lr
  (they're already learning fast; Adam prevents overshooting).
- Weights that rarely update get a **larger** effective lr (Adam gives them
  more of a nudge to avoid getting stuck).

This process runs for every weight in the network, once per mini-batch of 64 trades,
across all training epochs. After hundreds of mini-batches, the weights settle into
values that minimise the loss on training data.

---

### ASCII Flowchart

```
Input x (64 features, normalised)
        │
        ▼
┌─────────────────────────────────────────┐
│  Linear(64 → 256):   z = W·x + b       │
│  BatchNorm:          z = (z-μ)/σ×γ+β   │
│  ReLU:               h = max(0, z)      │
│  Dropout(0.3):       h[30%] = 0        │
└─────────────────────────────────────────┘
        │
        ▼
┌─────────────────────────────────────────┐
│  Linear(256 → 128):  z = W·h + b       │
│  BatchNorm  →  ReLU  →  Dropout(0.3)   │
└─────────────────────────────────────────┘
        │
        ▼
┌─────────────────────────────────────────┐
│  Linear(128 → 64):   z = W·h + b       │
│  BatchNorm  →  ReLU  →  Dropout(0.3)   │
└─────────────────────────────────────────┘
        │
        ▼
┌─────────────────────────────────────────┐
│  Linear(64 → 1):     logit = W·h + b   │
└─────────────────────────────────────────┘
        │
        ▼
  BCEWithLogitsLoss(logit, true_label)
  [training: backward() → update W, b via Adam]

        │
        ▼ (inference only)
  P = sigmoid(logit)
  prediction = "profitable" if P ≥ 0.5 else "loss"
```
