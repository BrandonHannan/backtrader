"""
Cylinder architecture configurations.

Design principle: all hidden layers have the same width, maintaining a
consistent representational capacity throughout the network. Only the
final linear layer narrows to 1 output logit.

         Input (64)
            ↓
      [width W layer]
            ↓
      [width W layer]   ← same width as above
            ↓
      [width W layer]   ← same width as above
            ↓
         Output (1)

Why this shape: unlike funnels (which commit early to compression) or
diamonds (which compress then expand), the cylinder treats every depth
level equally. This is ideal when you are not sure whether the useful
signal lives close to the input or requires many transformations to emerge.
The cylinder lets depth (not width) be the primary variable.

17 configurations varying:
  - Width  : 32 / 64 / 128 / 256 / 512
  - Depth  : 1 / 2 / 3 / 4 / 5 hidden layers
  - Dropout: 0.3 uniformly (depth is the only variable being tested)
"""


def get_configs() -> list:
    """Return the 17 cylinder hyperparameter configurations."""
    return [
        # --- Width 64 ---
        {"name": "cylinder_1L_64",  "hidden_dims": [64],                     "dropout": 0.3},
        {"name": "cylinder_2L_64",  "hidden_dims": [64, 64],                 "dropout": 0.3},
        {"name": "cylinder_3L_64",  "hidden_dims": [64, 64, 64],             "dropout": 0.3},
        {"name": "cylinder_4L_64",  "hidden_dims": [64, 64, 64, 64],         "dropout": 0.3},

        # --- Width 128 ---
        {"name": "cylinder_1L_128", "hidden_dims": [128],                    "dropout": 0.3},
        {"name": "cylinder_2L_128", "hidden_dims": [128, 128],               "dropout": 0.3},
        {"name": "cylinder_3L_128", "hidden_dims": [128, 128, 128],          "dropout": 0.3},
        {"name": "cylinder_4L_128", "hidden_dims": [128, 128, 128, 128],     "dropout": 0.3},

        # --- Width 256 ---
        {"name": "cylinder_1L_256", "hidden_dims": [256],                    "dropout": 0.3},
        {"name": "cylinder_2L_256", "hidden_dims": [256, 256],               "dropout": 0.3},
        {"name": "cylinder_3L_256", "hidden_dims": [256, 256, 256],          "dropout": 0.3},
        {"name": "cylinder_4L_256", "hidden_dims": [256, 256, 256, 256],     "dropout": 0.3},

        # --- Width 512 ---
        {"name": "cylinder_1L_512", "hidden_dims": [512],                    "dropout": 0.3},
        {"name": "cylinder_2L_512", "hidden_dims": [512, 512],               "dropout": 0.3},
        {"name": "cylinder_3L_512", "hidden_dims": [512, 512, 512],          "dropout": 0.3},

        # --- Width 32 (narrow, deep) ---
        {"name": "cylinder_3L_32",  "hidden_dims": [32, 32, 32],             "dropout": 0.3},
        {"name": "cylinder_5L_32",  "hidden_dims": [32, 32, 32, 32, 32],     "dropout": 0.3},
    ]
