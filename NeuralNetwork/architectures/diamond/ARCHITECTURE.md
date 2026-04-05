# Diamond Architecture

## 1. Design Rationale

The diamond architecture is built on a counter-intuitive idea: **compress first, then expand**.

Most networks start wide and narrow. The diamond does the opposite — it begins with a
tight bottleneck that forces the network to find a compact, noise-free summary of the 64
input features. Then it widens dramatically in the middle to explore complex non-linear
interactions between those compressed features. Finally it narrows again to reach the
single output logit.

```
Input (64) → [small] → [WIDE MIDDLE] → [small] → Output (1)
```

**Why compress first for trading data?**
The 64 input features contain significant noise and redundancy:
- e4 and e5 extremum points are often absent (zero-filled) — they contribute nothing
- price and volume statistics overlap (both track momentum via slope)
- macdReady/trendReady flags are binary, creating a mixed scale problem

A bottleneck at the start forces the network to immediately discard noise and keep only
the most informative signal. The wide middle then has a clean, compact representation to
work with when building the decision boundary.

**The diamond hypothesis:** If profitable trades differ from loss trades primarily through
non-obvious *combinations* of features (e.g., uptrend + significant slope + MACD crossover
together, not individually), then a diamond is well-suited — the wide middle has capacity
to model complex interaction effects between the bottleneck's compressed features.

**What it tests:** Does pre-compression help? How tight should the bottleneck be (8 vs 64)?
How wide should the middle expansion be (64 vs 256)?

---

## 2. Shape Diagram

Three representative configurations shown as horizontal bars:

```
diamond_16_64_16        diamond_32_256_32          diamond_64_128_256_128_64
(3 hidden layers)       (3 hidden layers)           (5 hidden layers, symmetric)

[======] 64 in          [======] 64 in              [======] 64 in
[=] 16                  [==] 32                     [======] 64
[=====] 64              [====================] 256  [============] 128
[=] 16                  [==] 32                     [====================] 256
[=] 1 out               [=] 1 out                   [============] 128
                                                     [======] 64
                                                     [=] 1 out
```

---

## 3. Mathematics (Plain English)

### The bottleneck layer — what compression actually means

When a linear layer compresses from 64 inputs to 16 neurons, the weight matrix W is
16 × 64. Each of the 16 neurons must summarise the entire 64-dimensional input in
one number. With only 16 neurons available, the network cannot memorise all 64 inputs
separately — it must find 16 directions through the 64-dimensional space that capture
the most variance. This is analogous to PCA (principal component analysis) but learned
end-to-end rather than computed separately.

**Step 1 — Bottleneck linear transform:**
```
W_bottleneck is (16 × 64)

z = W · x_norm + b     →    z is a 16-number summary of the 64-feature trade

Example (one neuron's calculation):
  neuron_1 = 0.31×x₁ + (-0.12)×x₂ + 0.07×x₃ + ... + b₁
           = 0.31×0.42 + 0.12×1.10 + 0.07×0.87 + ... + 0.01
           = 0.54    ← this neuron picks up on a combination of price mean, std, slope
```

**BatchNorm, ReLU, Dropout:** same as Funnel (see Funnel ARCHITECTURE.md §3).

### The wide middle layer — feature interaction

After compression to 16 numbers, the wide middle layer expands back to e.g. 128 neurons.
Each of those 128 neurons receives all 16 bottleneck features and computes a different
weighted combination:

```
W_middle is (128 × 16)

h_middle_neuron_1  = 0.8×h_bottleneck_1 + 0.3×h_bottleneck_2 + ... ← large interaction
h_middle_neuron_57 = -0.2×h_bottleneck_1 + 0.9×h_bottleneck_8 + ... ← different interaction
...
```

Because the bottleneck already cleaned the signal, these 128 interactions are between
compressed, meaningful features rather than noisy raw values.

### Output layer, loss, and inference — identical to Funnel (see §3 there)

---

## 4. Configuration Table

| # | Name | hidden_dims | Dropout | Approx params |
|---|------|-------------|---------|---------------|
| 1 | diamond_32_64_32 | [32, 64, 32] | 0.3 | 4,897 |
| 2 | diamond_32_128_32 | [32, 128, 32] | 0.3 | 8,993 |
| 3 | diamond_32_256_32 | [32, 256, 32] | 0.3 | 17,185 |
| 4 | diamond_16_64_16 | [16, 64, 16] | 0.2 | 2,657 |
| 5 | diamond_16_128_16 | [16, 128, 16] | 0.2 | 4,753 |
| 6 | diamond_64_128_64 | [64, 128, 64] | 0.3 | 21,441 |
| 7 | diamond_64_256_64 | [64, 256, 64] | 0.3 | 42,177 |
| 8 | diamond_32_128_64_32 | [32, 128, 64, 32] | 0.3 | 17,249 |
| 9 | diamond_32_256_128_32 | [32, 256, 128, 32] | 0.3 | 49,441 |
| 10 | diamond_16_64_128_64_16 | [16, 64, 128, 64, 16] | 0.2 | 27,361 |
| 11 | diamond_8_64_8 | [8, 64, 8] | 0.2 | 1,609 |
| 12 | diamond_8_128_8 | [8, 128, 8] | 0.2 | 2,697 |
| 13 | diamond_8_256_8 | [8, 256, 8] | 0.2 | 4,873 |
| 14 | diamond_32_64_128_64_32 | [32, 64, 128, 64, 32] | 0.3 | 25,921 |
| 15 | diamond_16_32_64_32_16 | [16, 32, 64, 32, 16] | 0.2 | 7,249 |
| 16 | diamond_64_128_256_128_64 | [64, 128, 256, 128, 64] | 0.3 | 122,177 |
| 17 | diamond_32_64_256_64_32 | [32, 64, 256, 64, 32] | 0.3 | 51,361 |

*Dropout is 0.2 for tight bottlenecks (width ≤ 16) — less capacity means less need for regularisation.*

---

## 5. Workflow Diagram — Full Traced Example

This traces one trade through `diamond_16_128_16` (hidden_dims = [16, 128, 16]).

### Input (after StandardScaler normalisation)
```
x_norm = [0.42, -1.10, 0.87, 0.05, -0.33, 1.21, 0.00, 0.00, ...]   (64 numbers)
                                                       ↑↑ e4, e5 extremums often zero
```

---

### Layer 1: Linear(64 → 16)  — the bottleneck

**Weight slice W₁** (showing 2 of the 16 neurons, each looking at 3 of 64 inputs):
```
W₁ = [[ 0.31, -0.12,  0.07, ... ],   ← neuron 1: price mean + std + slope combo
      [-0.05,  0.88, -0.03, ... ],   ← neuron 2: mostly price std
      ...                             ← 14 more summary neurons
     ]
b₁ = [0.01, -0.02, ...]
```

**Linear transform:**
```
z₁ = W₁ · x_norm + b₁
   = [0.31×0.42 + (-0.12)×(-1.10) + 0.07×0.87 + ... + 0.01,   → 0.54
      (-0.05)×0.42 + 0.88×(-1.10) + (-0.03)×0.87 + ... - 0.02, → -1.03
      ...]
```
Result: 16 numbers that summarise the entire 64-feature trade.

**BatchNorm → ReLU → Dropout(0.2):**
```
h₁ = [0.54_normed → ReLU → 0.61,   ← kept
       (-1.03)_normed → ReLU → 0.0, ← silenced (was negative)
       ...]                           (16 numbers, some zeroed)
```

---

### Layer 2: Linear(16 → 128)  — the wide expansion

**Weight slice W₂** (3 of 128 neurons, each combining all 16 bottleneck outputs):
```
W₂ = [[ 0.8, 0.3, -0.1, 0.5, ...],   ← neuron 1: uptrend + slope interaction?
      [-0.4, 0.9,  0.7, -0.2, ...],   ← neuron 57: a different interaction pattern
      [ 0.1, -0.6, 0.4, 0.8, ...],   ← neuron 89: yet another combination
      ...]
```

**Linear transform:**
```
z₂ = W₂ · h₁ + b₂
   = 128 numbers, each a weighted sum of the 16 compressed features
```

The 128 neurons each learn a **different cross-feature interaction** from the
compressed representation. This is the heart of the diamond — heavy interaction
exploration on a clean, compressed input.

**BatchNorm → ReLU → Dropout(0.2):**
```
h₂ = 128 numbers (some zeroed by ReLU and Dropout)
```

---

### Layer 3: Linear(128 → 16)  — compress back

```
h₃ = W₃ · h₂ + b₃  →  BatchNorm → ReLU → Dropout(0.2)
h₃ = 16 numbers (the most decision-relevant interactions from layer 2)
```

---

### Output: Linear(16 → 1)
```
logit = W_out · h₃ + b_out = -0.45
P(profitable) = sigmoid(-0.45) = 1 / (1 + e^0.45) = 0.39
```
Since 0.39 < 0.5  →  **predicted: loss**

---

### How Weights Change During Training (Backpropagation)

Suppose this trade's true label is **1** (profitable). The network predicted 0.39 — wrong.

**Step 1 — Compute loss:**
```
L = BCEWithLogitsLoss(logit=-0.45, label=1)
  = log(1 + e^0.45)            ← cross-entropy for positive label
  = log(1 + 1.568)
  = log(2.568)
  = 0.943
```

**Step 2 — Gradient of loss with respect to logit:**
```
∂L/∂logit = σ(logit) - label = 0.39 - 1.0 = -0.61
```
Negative gradient means: "increase the logit to reduce loss." The weights that
contributed positively to the logit should be increased; those that pushed it
negative should be decreased.

**Step 3 — Chain rule through all layers (backpropagation):**
```
∂L/∂W_out  =  -0.61 × h₃ᵀ
∂L/∂W₃     =  (W_outᵀ × -0.61) ⊙ ReLU_mask_3 × h₂ᵀ
∂L/∂W₂     =  (W₃ᵀ × δ₃) ⊙ ReLU_mask_2 × h₁ᵀ
∂L/∂W₁     =  (W₂ᵀ × δ₂) ⊙ ReLU_mask_1 × x_normᵀ
```
Where ⊙ = element-wise multiply, and the ReLU mask is 1 where h > 0 and 0 elsewhere
(gradients cannot flow back through silenced neurons).

**Step 4 — Adam update (identical to Funnel §5):**
```
For each weight w:
  m = 0.9 × m_prev + 0.1 × ∂L/∂w       ← smooth gradient
  v = 0.999 × v_prev + 0.001 × (∂L/∂w)² ← smooth squared gradient
  w_new = w_old - lr × m̂ / (√v̂ + 1e-8)
```

**pos_weight effect:** Because the dataset has more loss trades than profitable ones,
the loss function is weighted:
```
L = pos_weight × label × log(σ(logit)) + (1 - label) × log(1 - σ(logit))
```
When label=1 (profitable), the loss is multiplied by pos_weight ≈ 1.49.
This means gradient signals from profitable trades are ~49% larger than from
loss trades, so the network is proportionally more penalised for missing a
profitable trade than for incorrectly predicting one.

---

### ASCII Flowchart

```
Input x_norm (64 features)
        │
        ▼
┌─────────────────────────────────────────────┐
│  BOTTLENECK  Linear(64 → 16)                │
│  64 noisy features → 16 compact summaries   │
│  BatchNorm → ReLU → Dropout(0.2)            │
└─────────────────────────────────────────────┘
        │   16 compressed features
        ▼
┌─────────────────────────────────────────────┐
│  EXPANSION   Linear(16 → 128)               │
│  128 neurons explore cross-feature combos   │
│  BatchNorm → ReLU → Dropout(0.2)            │
└─────────────────────────────────────────────┘
        │   128 interaction features
        ▼
┌─────────────────────────────────────────────┐
│  COMPRESSION Linear(128 → 16)               │
│  Distil interactions to decision signal     │
│  BatchNorm → ReLU → Dropout(0.2)            │
└─────────────────────────────────────────────┘
        │   16 decision-relevant features
        ▼
┌─────────────────────────────────────────────┐
│  Linear(16 → 1):  logit = W·h + b           │
└─────────────────────────────────────────────┘
        │
        ▼
  BCEWithLogitsLoss(logit, label, pos_weight)
  [training: backward() → update all W, b via Adam]

        │
        ▼ (inference only)
  P = sigmoid(logit)
  prediction = "profitable" if P ≥ 0.5 else "loss"
```
