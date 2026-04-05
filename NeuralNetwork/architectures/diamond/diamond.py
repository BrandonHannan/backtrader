"""
Diamond architecture configurations.

Design principle: compress first (bottleneck), expand wide in the middle,
then compress back down to 1 output logit.

         Input (64)
            ↓
      [small bottleneck]   ← forces compact representation, filters noise
            ↓
      [wide middle layer]  ← heavy feature interaction / non-linear mixing
            ↓
      [small bottleneck]   ← compresses back to decision-relevant signal
            ↓
         Output (1)

Why this helps for trading data: the 64 input features contain a lot of
redundancy (e.g., e4/e5 extremums are often absent zeros, many stats
overlap). The initial bottleneck forces the network to find a sparse,
compact representation before the wide middle layer explores non-linear
interactions between those compressed features.

17 configurations varying:
  - Bottleneck width : 8 / 16 / 32 / 64
  - Peak width       : 64 / 128 / 256
  - Depth            : 3 / 5 layers (symmetric diamonds)
  - Dropout          : 0.2 (tight bottlenecks) or 0.3 (wider)
"""


def get_configs() -> list:
    """Return the 17 diamond hyperparameter configurations."""
    return [
        # --- 3-layer diamonds (compress → expand → compress) ---
        {"name": "diamond_32_64_32",          "hidden_dims": [32, 64, 32],             "dropout": 0.3},
        {"name": "diamond_32_128_32",         "hidden_dims": [32, 128, 32],            "dropout": 0.3},
        {"name": "diamond_32_256_32",         "hidden_dims": [32, 256, 32],            "dropout": 0.3},
        {"name": "diamond_16_64_16",          "hidden_dims": [16, 64, 16],             "dropout": 0.2},
        {"name": "diamond_16_128_16",         "hidden_dims": [16, 128, 16],            "dropout": 0.2},
        {"name": "diamond_64_128_64",         "hidden_dims": [64, 128, 64],            "dropout": 0.3},
        {"name": "diamond_64_256_64",         "hidden_dims": [64, 256, 64],            "dropout": 0.3},

        # --- 4-layer asymmetric diamonds ---
        {"name": "diamond_32_128_64_32",      "hidden_dims": [32, 128, 64, 32],        "dropout": 0.3},
        {"name": "diamond_32_256_128_32",     "hidden_dims": [32, 256, 128, 32],       "dropout": 0.3},

        # --- 5-layer symmetric diamonds ---
        {"name": "diamond_16_64_128_64_16",   "hidden_dims": [16, 64, 128, 64, 16],   "dropout": 0.2},
        {"name": "diamond_32_64_128_64_32",   "hidden_dims": [32, 64, 128, 64, 32],   "dropout": 0.3},
        {"name": "diamond_16_32_64_32_16",    "hidden_dims": [16, 32, 64, 32, 16],    "dropout": 0.2},
        {"name": "diamond_64_128_256_128_64", "hidden_dims": [64, 128, 256, 128, 64], "dropout": 0.3},
        {"name": "diamond_32_64_256_64_32",   "hidden_dims": [32, 64, 256, 64, 32],   "dropout": 0.3},

        # --- Tight bottleneck (extreme compression) ---
        {"name": "diamond_8_64_8",            "hidden_dims": [8, 64, 8],               "dropout": 0.2},
        {"name": "diamond_8_128_8",           "hidden_dims": [8, 128, 8],              "dropout": 0.2},
        {"name": "diamond_8_256_8",           "hidden_dims": [8, 256, 8],              "dropout": 0.2},
    ]
