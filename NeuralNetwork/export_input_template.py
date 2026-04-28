"""
Export a single MLP-input sample to a JSON template.

Loads one record from the training split, replays the full normalization
pipeline (extraction -> winsorize -> scale), and writes a JSON document that
shows exactly what GenericMLP.forward() receives, plus the transformation
parameters used.

Run:
    py -3.13 NeuralNetwork/export_input_template.py
    py -3.13 NeuralNetwork/export_input_template.py --index 5
"""

import argparse
import json
import os
import pickle
import sys

import numpy as np

sys.path.insert(0, os.path.dirname(__file__))
from neural_network import (
    FEATURE_NAMES,
    load_positions,
    _winsorize_apply,
)


TRAIN_FRAC = 0.70


def _fail(msg: str) -> None:
    print(f"[export_input_template] ERROR: {msg}", file=sys.stderr)
    sys.exit(1)


def _load_raw_positions(path: str) -> list:
    """
    Re-load data.json and apply the same unclosed-trade filter as load_positions(),
    yielding the source position dicts in the same order load_positions() emits rows.
    """
    with open(path, "r") as f:
        positions = json.load(f)

    kept = []
    for pos in positions:
        pnl = float(pos.get("pnl", 0.0))
        sell_date = pos.get("sellDate", "")
        if pnl == 0.0 and sell_date == "":
            continue
        kept.append(pos)
    return kept


def _temporal_sort_indices(dates: list) -> list:
    """Same sort the trainer uses: ascending purchaseDate (ISO strings sort lexicographically)."""
    return sorted(range(len(dates)), key=lambda i: dates[i])


def _to_native_list(arr) -> list:
    """numpy array -> plain Python list of floats (JSON-serialisable)."""
    return [float(x) for x in np.asarray(arr).ravel()]


def export_template(data_path: str, scaler_path: str, out_path: str, index: int) -> None:
    if not os.path.exists(data_path):
        _fail(f"data file not found: {data_path}")
    if not os.path.exists(scaler_path):
        _fail(
            f"scaler bundle not found: {scaler_path}\n"
            f"        Run `py -3.13 NeuralNetwork/neural_network.py --fast` first."
        )

    X, y, dates = load_positions(data_path)
    raw_positions = _load_raw_positions(data_path)
    if len(X) != len(raw_positions):
        _fail(
            f"row-count mismatch: load_positions() yielded {len(X)} rows but raw "
            f"filter yielded {len(raw_positions)}. Filter logic has drifted."
        )

    order = _temporal_sort_indices(dates)
    X_sorted = X[order]
    y_sorted = y[order]
    positions_sorted = [raw_positions[i] for i in order]

    n_train = int(len(X_sorted) * TRAIN_FRAC)
    if n_train == 0:
        _fail(f"training split is empty (only {len(X_sorted)} closed positions in dataset)")
    if not (0 <= index < n_train):
        _fail(f"--index {index} is out of range. Training split has {n_train} rows (0..{n_train - 1}).")

    raw_row = X_sorted[index].astype(np.float64)
    source_position = positions_sorted[index]
    label = int(y_sorted[index])

    with open(scaler_path, "rb") as f:
        bundle = pickle.load(f)

    scaler = bundle["scaler"]
    lo = bundle.get("winsor_lo")
    hi = bundle.get("winsor_hi")
    scaler_kind = bundle.get("scaler_kind", "standard")
    winsorize_pct = bundle.get("winsorize_pct", 0.0)

    if lo is not None and hi is not None:
        winsor_row = _winsorize_apply(raw_row, lo, hi)
    else:
        winsor_row = raw_row.copy()

    mlp_input = scaler.transform(winsor_row.reshape(1, -1))[0]

    if len(mlp_input) != len(FEATURE_NAMES):
        _fail(
            f"length mismatch: scaler produced {len(mlp_input)} features but "
            f"FEATURE_NAMES has {len(FEATURE_NAMES)}."
        )

    scaler_mean = getattr(scaler, "mean_", None)
    scaler_std = getattr(scaler, "scale_", None)

    values = []
    for i, name in enumerate(FEATURE_NAMES):
        values.append({
            "_not_ingested_by_mlp": ["index", "name", "raw", "winsorized"],
            "index": i,
            "name": name,
            "raw": float(raw_row[i]),
            "winsorized": float(winsor_row[i]),
            "value": float(mlp_input[i]),
        })

    document = {
        "metadata": {
            "_README": (
                "Top-level metadata about the source position and pipeline configuration. "
                "None of these fields are ingested by the MLP - they exist only so this "
                "template is reproducible."
            ),
            "source_data": data_path,
            "scaler_bundle": scaler_path,
            "scaler_kind": scaler_kind,
            "winsorize_pct": float(winsorize_pct) if winsorize_pct is not None else 0.0,
            "split": "train",
            "split_index": index,
            "feature_count": len(FEATURE_NAMES),
            "source_position": {
                "stockName": source_position.get("stockName", ""),
                "positionType": source_position.get("positionType", ""),
                "tradeType": source_position.get("tradeType", ""),
                "purchaseDate": source_position.get("purchaseDate", ""),
                "purchasePrice": float(source_position.get("purchasePrice", 0.0)),
                "sellDate": source_position.get("sellDate", ""),
                "sellPrice": float(source_position.get("sellPrice", 0.0)),
                "numShares": float(source_position.get("numShares", 0.0)),
                "pnl": float(source_position.get("pnl", 0.0)),
                "label": label,
            },
        },
        "transformation_parameters": {
            "_README": (
                "The exact arrays used to transform raw extracted features into the MLP "
                "input. Per-feature, ordered identically to FEATURE_NAMES."
            ),
            "winsor_lo":   _to_native_list(lo) if lo is not None else None,
            "winsor_hi":   _to_native_list(hi) if hi is not None else None,
            "scaler_mean": _to_native_list(scaler_mean) if scaler_mean is not None else None,
            "scaler_std":  _to_native_list(scaler_std)  if scaler_std  is not None else None,
        },
        "mlp_input": {
            "_README": (
                "This is exactly what GenericMLP.forward() receives. The MLP ingests ONLY "
                "the 'value' field of each entry as a flat float32 tensor of length "
                f"{len(FEATURE_NAMES)}. Every other field ('index', 'name', 'raw', "
                "'winsorized', '_not_ingested_by_mlp') is metadata included for human "
                "readability and is NOT part of the model input."
            ),
            "tensor_shape": [1, len(FEATURE_NAMES)],
            "tensor_dtype": "float32",
            "values": values,
            "flat_vector": _to_native_list(mlp_input),
        },
    }

    out_dir = os.path.dirname(out_path)
    if out_dir and not os.path.exists(out_dir):
        os.makedirs(out_dir, exist_ok=True)

    with open(out_path, "w") as f:
        json.dump(document, f, indent=2)

    print(f"[export_input_template] Wrote {out_path}")
    print(f"  source ticker      : {document['metadata']['source_position']['stockName']}")
    print(f"  purchaseDate       : {document['metadata']['source_position']['purchaseDate']}")
    print(f"  positionType       : {document['metadata']['source_position']['positionType']}")
    print(f"  pnl                : {document['metadata']['source_position']['pnl']:.4f}")
    print(f"  label              : {label} ({'profit' if label == 1 else 'loss'})")
    print(f"  training split row : {index} of {n_train}")


def main() -> None:
    repo_root = os.path.abspath(os.path.join(os.path.dirname(__file__), os.pardir))
    default_data = os.path.join(repo_root, "output", "data.json")
    default_scaler = os.path.join(os.path.dirname(__file__), "scaler.pkl")
    default_out = os.path.join(repo_root, "output", "data_ingestion_template.json")

    parser = argparse.ArgumentParser(description=__doc__.split("\n\n")[0])
    parser.add_argument("--data",   default=default_data,   help=f"Path to data.json (default: {default_data})")
    parser.add_argument("--scaler", default=default_scaler, help=f"Path to scaler.pkl (default: {default_scaler})")
    parser.add_argument("--out",    default=default_out,    help=f"Path to write template (default: {default_out})")
    parser.add_argument("--index",  type=int, default=0,    help="Row index within the training split (default: 0)")
    args = parser.parse_args()

    export_template(args.data, args.scaler, args.out, args.index)


if __name__ == "__main__":
    main()
