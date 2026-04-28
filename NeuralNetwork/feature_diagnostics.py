"""
Feature diagnostics: correlation matrix + permutation importance.

Reads:
  output/data.json
  NeuralNetwork/scaler.pkl       (saved by neural_network.py)
  NeuralNetwork/model.pt         (best model from grid search)
  NeuralNetwork/model_metadata.json

Writes:
  NeuralNetwork/feature_report.txt

Run:
  py -3.13 NeuralNetwork/feature_diagnostics.py
"""

import json
import os
import pickle
import sys

import numpy as np
import torch
from sklearn.metrics import roc_auc_score

sys.path.insert(0, os.path.dirname(__file__))
from neural_network import FEATURE_NAMES, load_positions, _winsorize_apply
from architectures.base import GenericMLP


CORRELATION_THRESHOLD = 0.95


def temporal_split(X, y, dates, train_frac=0.70, val_frac=0.15):
    """Match neural_network.py's temporal split exactly."""
    order = sorted(range(len(dates)), key=lambda i: dates[i])
    X, y = X[order], y[order]
    n = len(X)
    n_train = int(n * train_frac)
    n_val = int(n * (train_frac + val_frac))
    return (X[:n_train], y[:n_train],
            X[n_train:n_val], y[n_train:n_val],
            X[n_val:], y[n_val:])


def apply_scaler_bundle(X_train, X_val, X_test, bundle: dict):
    """Replay winsorize + scaler from the saved bundle."""
    lo, hi = bundle.get("winsor_lo"), bundle.get("winsor_hi")
    if lo is not None and hi is not None:
        X_train = _winsorize_apply(X_train, lo, hi)
        X_val   = _winsorize_apply(X_val,   lo, hi)
        X_test  = _winsorize_apply(X_test,  lo, hi)
    scaler = bundle["scaler"]
    return (scaler.transform(X_train),
            scaler.transform(X_val),
            scaler.transform(X_test))


def correlation_report(X_train_scaled, names, threshold=CORRELATION_THRESHOLD):
    """Compute Pearson correlation matrix; flag pairs above threshold.

    Constant-variance columns produce NaN in corrcoef — treat those as 0.
    """
    with np.errstate(invalid="ignore", divide="ignore"):
        corr = np.corrcoef(X_train_scaled, rowvar=False)
    corr = np.nan_to_num(corr, nan=0.0)
    np.fill_diagonal(corr, 0.0)

    pairs = []
    n = corr.shape[0]
    for i in range(n):
        for j in range(i + 1, n):
            if abs(corr[i, j]) >= threshold:
                pairs.append((abs(corr[i, j]), names[i], names[j], corr[i, j]))
    pairs.sort(key=lambda t: t[0], reverse=True)
    return corr, pairs


def permutation_importance(model, X_test, y_test, names, baseline_auc, n_repeats=3, rng=None):
    """Shuffle each column n_repeats times; record mean AUC drop."""
    rng = rng or np.random.default_rng(42)
    importances = []
    device = next(model.parameters()).device

    for col, name in enumerate(names):
        drops = []
        for _ in range(n_repeats):
            X_shuffled = X_test.copy()
            rng.shuffle(X_shuffled[:, col])
            with torch.no_grad():
                logits = model(torch.from_numpy(X_shuffled).float().to(device)).cpu().numpy().squeeze()
            try:
                auc = roc_auc_score(y_test, logits)
            except ValueError:
                auc = baseline_auc
            drops.append(baseline_auc - auc)
        importances.append((name, float(np.mean(drops)), float(np.std(drops))))

    importances.sort(key=lambda t: t[1], reverse=True)
    return importances


def load_model(model_path: str, metadata_path: str, n_features: int):
    """Reconstruct GenericMLP from saved metadata + state dict."""
    with open(metadata_path, "r") as f:
        meta = json.load(f)
    hidden_dims = meta["hidden_dims"]
    dropout = meta.get("dropout", 0.3)
    model = GenericMLP(input_dim=n_features, hidden_dims=hidden_dims, dropout=dropout)
    model.load_state_dict(torch.load(model_path, map_location="cpu"))
    model.eval()
    return model, meta


def main():
    here = os.path.dirname(__file__)
    data_path = os.path.abspath(os.path.join(here, "..", "output", "data.json"))
    scaler_path = os.path.join(here, "scaler.pkl")
    model_path = os.path.join(here, "model.pt")
    metadata_path = os.path.join(here, "model_metadata.json")
    report_path = os.path.join(here, "feature_report.txt")

    for required in (data_path, scaler_path, model_path, metadata_path):
        if not os.path.exists(required):
            print(f"ERROR: missing {required}. Run neural_network.py first.", file=sys.stderr)
            sys.exit(1)

    print("[diag]  Loading data...")
    X, y, dates = load_positions(data_path)
    X_train, y_train, X_val, y_val, X_test, y_test = temporal_split(X, y, dates)

    with open(scaler_path, "rb") as f:
        bundle = pickle.load(f)
    X_train_s, X_val_s, X_test_s = apply_scaler_bundle(X_train, X_val, X_test, bundle)

    if len(FEATURE_NAMES) != X.shape[1]:
        print(f"ERROR: FEATURE_NAMES has {len(FEATURE_NAMES)} entries, X has {X.shape[1]} columns.", file=sys.stderr)
        sys.exit(1)

    print("[diag]  Computing correlation matrix...")
    corr, high_pairs = correlation_report(X_train_s, FEATURE_NAMES)

    print(f"[diag]  Loading model from {model_path}...")
    model, meta = load_model(model_path, metadata_path, X.shape[1])

    print("[diag]  Computing baseline AUC...")
    with torch.no_grad():
        baseline_logits = model(torch.from_numpy(X_test_s).float()).cpu().numpy().squeeze()
    baseline_auc = roc_auc_score(y_test, baseline_logits)
    print(f"[diag]  Baseline AUC-ROC = {baseline_auc:.4f}")

    print("[diag]  Running permutation importance (3 shuffles per feature)...")
    importances = permutation_importance(model, X_test_s, y_test, FEATURE_NAMES, baseline_auc, n_repeats=3)

    print(f"[diag]  Writing report -> {report_path}")
    with open(report_path, "w") as f:
        f.write("FEATURE DIAGNOSTICS REPORT\n")
        f.write("=" * 70 + "\n\n")
        f.write(f"Best model: {meta['family']}/{meta['name']}\n")
        f.write(f"Baseline AUC-ROC on test: {baseline_auc:.4f}\n")
        f.write(f"Scaler: {bundle.get('scaler_kind', 'unknown')}, winsorize_pct={bundle.get('winsorize_pct', 0)}\n")
        f.write(f"Train samples: {len(X_train)}, Test samples: {len(X_test)}\n\n")

        f.write(f"--- HIGH-CORRELATION PAIRS (|r| >= {CORRELATION_THRESHOLD}) ---\n")
        if not high_pairs:
            f.write("(none — every feature carries independent signal at this threshold)\n")
        else:
            f.write(f"{'rank':>4}  {'r':>7}  {'feature_a':<28}  {'feature_b':<28}\n")
            for rank, (_, a, b, r) in enumerate(high_pairs[:50], start=1):
                f.write(f"{rank:>4}  {r:>+7.4f}  {a:<28}  {b:<28}\n")
        f.write("\n")

        f.write("--- PERMUTATION IMPORTANCE (sorted by AUC drop, desc) ---\n")
        f.write(f"{'rank':>4}  {'feature':<28}  {'mean_drop':>10}  {'std':>8}\n")
        for rank, (name, mean_drop, std_drop) in enumerate(importances, start=1):
            f.write(f"{rank:>4}  {name:<28}  {mean_drop:>+10.5f}  {std_drop:>8.5f}\n")
        f.write("\n")

        f.write("--- DROP CANDIDATES ---\n")
        f.write("Features with mean_drop <= 0 (shuffling them does not hurt AUC):\n")
        candidates = [(n, d, s) for n, d, s in importances if d <= 0.0]
        if not candidates:
            f.write("(none)\n")
        else:
            for name, mean_drop, std_drop in candidates:
                f.write(f"  {name:<28}  drop={mean_drop:+.5f}  std={std_drop:.5f}\n")

    print("[diag]  Done.")
    print()
    print(f"  High-correlation pairs (|r|>={CORRELATION_THRESHOLD}): {len(high_pairs)}")
    if high_pairs[:5]:
        print("  Top correlated:")
        for _, a, b, r in high_pairs[:5]:
            print(f"    {a:<28} <-> {b:<28}  r={r:+.4f}")
    print()
    print("  Top 5 most important features:")
    for name, mean_drop, std_drop in importances[:5]:
        print(f"    {name:<28}  drop={mean_drop:+.5f} (std={std_drop:.5f})")
    print()
    print("  Bottom 5 (drop candidates):")
    for name, mean_drop, std_drop in importances[-5:]:
        print(f"    {name:<28}  drop={mean_drop:+.5f} (std={std_drop:.5f})")


if __name__ == "__main__":
    main()
