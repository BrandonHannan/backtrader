# Cylinder Architecture

## 1. Design Rationale

The cylinder architecture maintains a **constant width** throughout all hidden layers.
Every hidden layer has the same number of neurons. Only the final linear layer narrows
to 1 output logit.

```
Input (64) → [W neurons] → [W neurons] → [W neurons] → Output (1)
              same width    same width    same width
```

This is the "steady state" architecture: no compression, no expansion, no assumptions
about where in the network the useful signal forms.

**Why constant width?** The funnel commits to progressive compression — it assumes
the useful decision boundary is best found by narrowing. The diamond commits to
a bottleneck-first philosophy. The cylinder makes neither assumption. It says:
"the same amount of representational capacity is needed at each stage, and we'll
let depth (the number of layers) do the work."

This design is widely used in natural language processing (e.g., Transformer layers
all have the same hidden dimension) because it's robust: the information representation
doesn't shrink or explode, it just gets *refined* with each additional layer.

**What it tests:** Is depth the key variable? Does width matter more than depth?
Can simple flat representations (shallow cylinders) match deep ones on this data?

**Expected strength for trading data:** If profitable trades are characterised by
a complex pattern that builds across multiple feature interactions — e.g., "MACD
crossover *and* significant slope *and* trend confirmation *and* trendline active"
— then more layers (greater depth) gives the network more chances to combine
these signals incrementally, without the distortion of shrinking or expanding.

---

## 2. Shape Diagram

Three representative configurations shown as horizontal bars:

```
cylinder_1L_128         cylinder_3L_128             cylinder_5L_32
(1 hidden layer)        (3 hidden layers)            (5 hidden layers)

[======] 64 in          [======] 64 in               [======] 64 in
[============] 128      [============] 128            [==] 32
[=] 1 out               [============] 128            [==] 32
                        [============] 128            [==] 32
                        [=] 1 out                     [==] 32
                                                      [==] 32
                                                      [=] 1 out
```

All hidden bars are the same length within each configuration — that is the cylinder shape.

---

## 3. Mathematics (Plain English)

### Why depth changes the computation even with constant width

With a constant width W, each hidden layer computes:
```
h_k = ReLU(BN(W_k · h_{k-1} + b_k))
```

Even though W_k and h_{k-1} are the same size at every layer, each W_k is a *different*
learned matrix. So layer 2 is not repeating layer 1 — it is computing a new weighted
combination of the representations that layer 1 produced.

**Analogy:** Think of a series of meetings where 10 people always attend. In meeting 1
they share raw observations; in meeting 2 they synthesise what they learned in meeting 1;
in meeting 3 they refine further. The group size (width) never changes, but each meeting
produces a more refined understanding.

### Linear transform at each layer:
```
z_k = W_k · h_{k-1} + b_k
```
- W_k is (W × W) — a square matrix (same input and output width)
- This is the most parameter-efficient shape per layer because in × out is minimised
  relative to both funnels (which have large in) and diamonds (which have large out or in)

### BatchNorm, ReLU, Dropout — identical to Funnel/Diamond (see those docs §3)

### The gradient flow advantage of constant width

In very deep funnels, layers near the input have much larger weight matrices than
layers near the output. The gradients flowing back through the small output layers
can become very small before reaching the large input layers — the "vanishing gradient"
problem. In a cylinder, all weight matrices are the same size, so gradient magnitudes
stay more consistent across all layers. This makes very deep cylinders (e.g., 5 layers)
more stable to train than 5-layer funnels.

---

## 4. Configuration Table

| # | Name | hidden_dims | Dropout | Approx params |
|---|------|-------------|---------|---------------|
| 1 | cylinder_1L_64 | [64] | 0.3 | 4,289 |
| 2 | cylinder_2L_64 | [64, 64] | 0.3 | 8,449 |
| 3 | cylinder_3L_64 | [64, 64, 64] | 0.3 | 12,609 |
| 4 | cylinder_4L_64 | [64, 64, 64, 64] | 0.3 | 16,769 |
| 5 | cylinder_1L_128 | [128] | 0.3 | 8,449 |
| 6 | cylinder_2L_128 | [128, 128] | 0.3 | 24,961 |
| 7 | cylinder_3L_128 | [128, 128, 128] | 0.3 | 41,473 |
| 8 | cylinder_4L_128 | [128, 128, 128, 128] | 0.3 | 57,985 |
| 9 | cylinder_1L_256 | [256] | 0.3 | 16,897 |
| 10 | cylinder_2L_256 | [256, 256] | 0.3 | 82,945 |
| 11 | cylinder_3L_256 | [256, 256, 256] | 0.3 | 148,993 |
| 12 | cylinder_4L_256 | [256, 256, 256, 256] | 0.3 | 215,041 |
| 13 | cylinder_1L_512 | [512] | 0.3 | 33,793 |
| 14 | cylinder_2L_512 | [512, 512] | 0.3 | 296,961 |
| 15 | cylinder_3L_512 | [512, 512, 512] | 0.3 | 560,129 |
| 16 | cylinder_3L_32 | [32, 32, 32] | 0.3 | 3,233 |
| 17 | cylinder_5L_32 | [32, 32, 32, 32, 32] | 0.3 | 4,257 |

*Dropout is uniformly 0.3 — depth is the only variable being tested, so regularisation is held constant.*

---

## 5. Workflow Diagram — Full Traced Example

This traces one trade through `cylinder_3L_128` (hidden_dims = [128, 128, 128]).

### Input (after StandardScaler normalisation)
```
x_norm = [0.42, -1.10, 0.87, 0.05, -0.33, 1.21, 0.00, ...]   (64 numbers)
```

---

### Layer 1: Linear(64 → 128) — initial representation

**Weight slice W₁** (128 × 64 matrix — showing 2 neurons, 3 weights each):
```
W₁ = [[ 0.12, -0.08,  0.31, ...],   ← neuron 1: price mean + trend interaction
      [ 0.05,  0.77, -0.22, ...],   ← neuron 2: mostly volume std
      ...]
b₁ = [0.01, -0.03, ...]
```

**Compute z₁ = W₁ · x_norm + b₁:**
```
neuron_1: 0.12×0.42 + (-0.08)×(-1.10) + 0.31×0.87 + ... + 0.01 = 1.34
neuron_2: 0.05×0.42 + 0.77×(-1.10) + (-0.22)×0.87 + ... - 0.03 = -0.71
...
z₁ = [1.34, -0.71, 0.22, 1.05, ...]   (128 numbers)
```

**BatchNorm** (normalise across the 64-trade mini-batch):
```
μ₁ = 0.03  (mean of z₁ across the batch)
σ₁ = 1.12  (std of z₁ across the batch)
z₁_norm = (z₁ - 0.03) / 1.12 × γ + β  ≈  [1.17, -0.66, 0.17, 0.91, ...]
```

**ReLU** (silence negatives):
```
h₁ = max(0, z₁_norm) = [1.17, 0.00, 0.17, 0.91, ...]
                               ↑ neuron 2 was -0.66, now silent
```

**Dropout(0.3)** (random 30% zeroed):
```
h₁_dropped = [1.17, 0.00, 0.00, 0.91, ...]   ← additional random zeros
```

---

### Layer 2: Linear(128 → 128) — refinement pass 1

**Weight W₂ is 128 × 128** (a square matrix — same input and output size):
```
W₂ = [[...], [...], ...]    ← 128 neurons, each receiving all 128 outputs from Layer 1
```

This layer re-combines the 128 representations from Layer 1. Because W₂ ≠ W₁,
it computes genuinely different combinations — not a repetition of Layer 1.

```
z₂ = W₂ · h₁_dropped + b₂   (128 numbers)
```

After BatchNorm → ReLU → Dropout(0.3):
```
h₂ = [0.88, 0.00, 0.43, 0.00, ...]
```

---

### Layer 3: Linear(128 → 128) — refinement pass 2

Same structure as Layer 2, using its own unique weight matrix W₃.
Each additional layer gives the network one more opportunity to refine
which combinations of features best predict profitability.

```
h₃ = [0.52, 1.10, 0.00, 0.78, ...]   (128 numbers, after BN+ReLU+Dropout)
```

---

### Output: Linear(128 → 1)

**Weight W_out is (1 × 128)** — one row vector, one bias:
```
logit = W_out · h₃ + b_out
      = 0.21×0.52 + 0.14×1.10 + 0.00×0.00 + (-0.33)×0.78 + ... + b_out
      = 0.72

P(profitable) = sigmoid(0.72) = 1 / (1 + e^(-0.72)) = 0.67
```
Since 0.67 ≥ 0.5  →  **predicted: profitable**

---

### How Weights Change During Training (Backpropagation)

Suppose this trade is actually profitable (label = 1) and the model predicted 0.67.
The loss will be small (correct direction), but weights still update to push probability higher.

**Step 1 — Compute loss:**
```
L = BCEWithLogitsLoss(logit=0.72, label=1)
  = -log(σ(0.72))
  = -log(0.67)
  = 0.400
```

**Step 2 — Gradient of loss w.r.t. logit:**
```
∂L/∂logit = σ(logit) - label = 0.67 - 1.0 = -0.33
```
Negative gradient: the logit should increase (weights that produced it should grow slightly).

**Step 3 — Chain rule through 3 layers:**
```
δ_out  = ∂L/∂logit = -0.33

∂L/∂W_out = δ_out × h₃ᵀ                        (1 × 128 gradient)
∂L/∂h₃    = W_outᵀ × δ_out                     (128-vector)
∂L/∂W₃    = (∂L/∂h₃ ⊙ ReLU_mask_3) × h₂ᵀ      (128 × 128 gradient)
∂L/∂h₂    = W₃ᵀ × (∂L/∂h₃ ⊙ ReLU_mask_3)
∂L/∂W₂    = (∂L/∂h₂ ⊙ ReLU_mask_2) × h₁ᵀ
∂L/∂h₁    = W₂ᵀ × (∂L/∂h₂ ⊙ ReLU_mask_2)
∂L/∂W₁    = (∂L/∂h₁ ⊙ ReLU_mask_1) × x_normᵀ  (128 × 64 gradient)
```

**Key advantage of constant width:** all gradient tensors propagating backward have
size 128 at every layer. In a 3-layer funnel going 512→256→128, the gradient tensor
at layer 1 is 512-wide while at layer 3 it is 128-wide — a 4× size difference that
can cause instability. The cylinder avoids this entirely.

**Step 4 — Adam weight update (same formula as Funnel §5):**
```
For every weight w across all layers:
  m_new = 0.9 × m_old + 0.1 × ∂L/∂w         ← momentum
  v_new = 0.999 × v_old + 0.001 × (∂L/∂w)²  ← velocity
  w_new = w_old - lr × m̂ / (√v̂ + 1e-8)      ← adaptive step

Concrete example for one weight:
  w_old = 0.21,  ∂L/∂w = -0.04  (should increase)
  m = 0.9×0 + 0.1×(-0.04) = -0.004
  v = 0.999×0 + 0.001×0.0016 = 0.0000016
  w_new = 0.21 - 0.001 × (-0.004) / (√0.0000016 + 1e-8)
        = 0.21 - 0.001 × (-3.16)
        = 0.21 + 0.00316
        = 0.213      ← weight increased (correct direction)
```

After thousands of mini-batches, each of the 128×128 + 128×128 + 128×64 + 1×128
weights settles to values that best separate profitable from loss trades.

**ReduceLROnPlateau:** If the validation loss stops improving for 5 epochs, the
learning rate is halved (0.001 → 0.0005 → 0.00025 → ...). This lets the model
make coarser updates early in training and finer updates later, recovering from
saddle points that would stall a fixed-lr optimiser.

---

### ASCII Flowchart

```
Input x_norm (64 features)
        │
        ▼
┌─────────────────────────────────────────────────────────┐
│  Layer 1: Linear(64 → 128)   z = W₁·x + b₁             │
│  BatchNorm: z_norm = (z-μ)/σ × γ + β                   │
│  ReLU:      h = max(0, z_norm)                          │
│  Dropout(0.3): h[30%] = 0                              │
└─────────────────────────────────────────────────────────┘
        │  128 features (initial representation)
        ▼
┌─────────────────────────────────────────────────────────┐
│  Layer 2: Linear(128 → 128)  z = W₂·h + b₂   [W₂ ≠ W₁]│
│  BatchNorm → ReLU → Dropout(0.3)                        │
└─────────────────────────────────────────────────────────┘
        │  128 features (refined representation)
        ▼
┌─────────────────────────────────────────────────────────┐
│  Layer 3: Linear(128 → 128)  z = W₃·h + b₃   [W₃ ≠ W₂]│
│  BatchNorm → ReLU → Dropout(0.3)                        │
└─────────────────────────────────────────────────────────┘
        │  128 features (further refined)
        ▼
┌─────────────────────────────────────────────────────────┐
│  Output: Linear(128 → 1)     logit = W_out·h + b_out   │
└─────────────────────────────────────────────────────────┘
        │
        ▼
  BCEWithLogitsLoss(logit, label, pos_weight ≈ 1.49)
  [training: backward() → update all W₁, W₂, W₃, W_out via Adam]
  [if val_loss stalls 5 epochs: lr × 0.5 via ReduceLROnPlateau]

        │
        ▼ (inference only)
  P = sigmoid(logit)
  prediction = "profitable" if P ≥ 0.5 else "loss"
```
