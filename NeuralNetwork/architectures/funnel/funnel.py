"""
Funnel architecture configurations.

Design principle: each hidden layer is narrower than the one before it,
forcing the network to progressively compress information. The network starts
wider than the input (to expand representational capacity) then funnels down
to 1 output logit.

Analogy: a funnel narrows water flow — the network narrows information flow,
forcing it to keep only what matters for the final prediction.

17 configurations varying:
  - Initial width  : 128 / 256 / 512 / 1024
  - Depth          : 2 / 3 / 4 hidden layers
  - Compression    : gradual (halving each layer) vs steep (large single drop)
  - Dropout        : 0.3 (standard) or 0.4 (wider/steeper funnels that risk overfitting)
"""


def get_configs() -> list:
    """Return the 17 funnel hyperparameter configurations."""
    return [
        # --- 128-wide entry, gradual compression ---
        {"name": "funnel_2L_128",        "hidden_dims": [128, 64],           "dropout": 0.3},
        {"name": "funnel_3L_128",        "hidden_dims": [128, 64, 32],       "dropout": 0.3},
        {"name": "funnel_4L_128",        "hidden_dims": [128, 64, 32, 16],   "dropout": 0.3},

        # --- 256-wide entry, gradual compression ---
        {"name": "funnel_2L_256",        "hidden_dims": [256, 128],          "dropout": 0.3},
        {"name": "funnel_3L_256",        "hidden_dims": [256, 128, 64],      "dropout": 0.3},
        {"name": "funnel_4L_256",        "hidden_dims": [256, 128, 64, 32],  "dropout": 0.3},

        # --- 256-wide entry, steep compression ---
        {"name": "funnel_2L_256_steep",  "hidden_dims": [256, 64],           "dropout": 0.4},

        # --- 512-wide entry, gradual compression ---
        {"name": "funnel_2L_512",        "hidden_dims": [512, 256],          "dropout": 0.3},
        {"name": "funnel_3L_512",        "hidden_dims": [512, 256, 128],     "dropout": 0.3},
        {"name": "funnel_4L_512",        "hidden_dims": [512, 256, 128, 64], "dropout": 0.3},

        # --- 512-wide entry, steep compression ---
        {"name": "funnel_2L_512_steep",  "hidden_dims": [512, 128],          "dropout": 0.4},
        {"name": "funnel_2L_512_vsteep", "hidden_dims": [512, 64],           "dropout": 0.4},
        {"name": "funnel_3L_256_steep",  "hidden_dims": [256, 128, 32],      "dropout": 0.4},
        {"name": "funnel_3L_512_steep",  "hidden_dims": [512, 256, 64],      "dropout": 0.4},
        {"name": "funnel_2L_128_steep",  "hidden_dims": [128, 32],           "dropout": 0.4},

        # --- 1024-wide entry (maximum capacity) ---
        {"name": "funnel_3L_1024",       "hidden_dims": [1024, 512, 256],    "dropout": 0.3},
        {"name": "funnel_3L_1024_steep", "hidden_dims": [1024, 256, 64],     "dropout": 0.4},
    ]
