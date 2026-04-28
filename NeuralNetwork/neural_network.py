"""
Multi-architecture neural network orchestrator for trade profitability prediction.

Runs a hyperparameter grid search across 51 configurations spanning three
architecture families (Funnel, Diamond, Cylinder), prints a ranked leaderboard,
and saves the best model.

Input : output/data.json  (array of closed Position objects from the C++ backtester)
Output: binary classification — 1 (profitable, pnl > 0) / 0 (loss)

Feature vector (77 values per position, ATR/return-normalized for cross-ticker stationarity):
  Position-level  : positionType                                          (1)
  Entry context   : price stats (ATR-distance + CoV), volume stats       (76)
                    (log + CoV), MACD/mean, trend (days-since-e1 +
                    ATR-distance closes), trendLine (slope/mean +
                    intercept-distance + projected-distance), RSI,
                    ATR/price + volatility regime ratio

Run:
    py -3.13 NeuralNetwork/neural_network.py
    py -3.13 NeuralNetwork/neural_network.py --data path/to/data.json
    py -3.13 NeuralNetwork/neural_network.py --fast   # quick 50-epoch sweep
"""

import argparse
import datetime as dt
import json
import os
import sys
import pickle

import numpy as np
from sklearn.preprocessing import RobustScaler, StandardScaler

import torch

# Architecture config providers
from architectures.funnel.funnel       import get_configs as funnel_configs
from architectures.diamond.diamond     import get_configs as diamond_configs
from architectures.cylinder.cylinder   import get_configs as cylinder_configs

# Shared runtime
from architectures.base import GenericMLP, make_loader, train_model, evaluate, TrainResult

# Gradient boosting baseline
from tree_models import run_tree_models


# ---------------------------------------------------------------------------
# Feature extraction
# ---------------------------------------------------------------------------

TREND_TYPE_MAP = {"UPTREND": 1.0, "NONE": 0.0, "DOWNTREND": -1.0}
EPS = 1e-9

FEATURE_NAMES = (
    ["positionType"]
    + [f"price_{n}" for n in ("mean_atr", "std_atr", "min_atr", "max_atr",
                              "slope_ret", "slopeSE_ret", "slopeRSQ", "slopeSig", "cov")]
    + [f"vol_{n}"   for n in ("log_mean", "log_std", "log_min", "log_max",
                              "slope_frac", "slopeSE_frac", "slopeRSQ", "slopeSig", "cov")]
    + ["macd_ret", "signal_ret", "macdReady"]
    + ["trend_ready", "trend_type"]
    + [f"trend_e{i}_{f}" for i in range(1, 6) for f in ("days", "close_atr", "trough")]
    + ["tl_ready", "tl_active", "tl_slope_ret", "tl_intercept_atr", "tl_dateDiff", "tl_proj_atr"]
    + ["dtrend_ready", "dtrend_type"]
    + [f"dtrend_e{i}_{f}" for i in range(1, 6) for f in ("days", "close_atr", "trough")]
    + ["dtl_ready", "dtl_active", "dtl_slope_ret", "dtl_intercept_atr", "dtl_dateDiff", "dtl_proj_atr"]
    + ["rsi", "rsiReady", "doubleRsi", "doubleRsiReady"]
    + ["atr_ret", "atrReady", "doubleAtr_ret", "doubleAtrReady", "vol_ratio"]
)
assert len(FEATURE_NAMES) == 77, f"FEATURE_NAMES length {len(FEATURE_NAMES)} != 77"


def _safe_div(num: float, den: float) -> float:
    """Divide with a magnitude guard. Returns 0.0 when denominator is too small."""
    den_f = float(den)
    if abs(den_f) <= EPS:
        return 0.0
    return float(num) / den_f


def _days_diff(later: str, earlier: str) -> float:
    """Days between two ISO-format date strings. Returns 0.0 on parse failure."""
    if not later or not earlier:
        return 0.0
    try:
        a = dt.date.fromisoformat(later)
        b = dt.date.fromisoformat(earlier)
        return float((a - b).days)
    except (ValueError, TypeError):
        return 0.0


def _price_stats_features(stats: dict, purchase_price: float, atr_value: float) -> list:
    """
    Price statistics, normalized for cross-ticker comparability. 9 features:
      [mean_atr_dist, std_atr, min_atr_dist, max_atr_dist,
       slope_return, slopeSE_return, slopeRSQ, slopeSignificant, price_cov]
    """
    mean = float(stats.get("mean", 0.0))
    std = float(stats.get("std", 0.0))
    pmin = float(stats.get("min", 0.0))
    pmax = float(stats.get("max", 0.0))
    slope = float(stats.get("slope", 0.0))
    slope_se = float(stats.get("slopeSE", 0.0))
    return [
        _safe_div(mean - purchase_price, atr_value),
        _safe_div(std, atr_value),
        _safe_div(pmin - purchase_price, atr_value),
        _safe_div(pmax - purchase_price, atr_value),
        _safe_div(slope, mean),
        _safe_div(slope_se, mean),
        float(stats.get("slopeRSQ", 0.0)),
        1.0 if stats.get("slopeSignificant", False) else 0.0,
        _safe_div(std, mean),
    ]


def _volume_stats_features(stats: dict) -> list:
    """
    Volume statistics, log-transformed plus CoV-derived. 9 features:
      [log_mean, log_std, log_min, log_max,
       slope_frac, slopeSE_frac, slopeRSQ, slopeSignificant, vol_cov]
    """
    mean = float(stats.get("mean", 0.0))
    std = float(stats.get("std", 0.0))
    vmin = float(stats.get("min", 0.0))
    vmax = float(stats.get("max", 0.0))
    slope = float(stats.get("slope", 0.0))
    slope_se = float(stats.get("slopeSE", 0.0))
    return [
        float(np.log1p(max(mean, 0.0))),
        float(np.log1p(max(std, 0.0))),
        float(np.log1p(max(vmin, 0.0))),
        float(np.log1p(max(vmax, 0.0))),
        _safe_div(slope, mean),
        _safe_div(slope_se, mean),
        float(stats.get("slopeRSQ", 0.0)),
        1.0 if stats.get("slopeSignificant", False) else 0.0,
        _safe_div(std, mean),
    ]


def _extremum_features(ex: dict, anchor_date: str, purchase_price: float, atr_value: float) -> list:
    """
    3 features from a single extremum:
      [days_since_anchor, close_atr_dist, isTrough]

    Missing extremums (index == -1 or empty date) are zeroed out across all
    three features so the network sees a neutral signal instead of a
    spurious -purchasePrice/atrValue distance from a fictitious zero-close.
    """
    if ex.get("index", -1) == -1 or not ex.get("date", ""):
        return [0.0, 0.0, 0.0]
    return [
        _days_diff(ex.get("date", ""), anchor_date),
        _safe_div(float(ex.get("close", 0.0)) - purchase_price, atr_value),
        1.0 if ex.get("isTrough", False) else 0.0,
    ]


def _trend_features(ready_flag: bool, trend: dict, purchase_price: float, atr_value: float) -> list:
    """
    17 features from a trend block. e1.date is the duration anchor.
    Layout: [trendReady, trendType, e1×3, e2×3, e3×3, e4×3, e5×3]
    """
    anchor_date = trend.get("e1", {}).get("date", "")
    features = [
        1.0 if ready_flag else 0.0,
        TREND_TYPE_MAP.get(trend.get("type", "NONE"), 0.0),
    ]
    for key in ("e1", "e2", "e3", "e4", "e5"):
        features.extend(_extremum_features(trend.get(key, {}), anchor_date, purchase_price, atr_value))
    return features  # 2 + 5×3 = 17


def _trendline_features(ready_flag: bool, tl: dict, purchase_price: float,
                       atr_value: float, mean_price: float) -> list:
    """
    6 features from a trendline block:
      [trendLineReady, isActive, slope_return, intercept_atr_dist,
       dateDifference, projected_atr_dist]

    When the trendline is not ready, the structural fields default to zero in
    the JSON. Zero out the derived distances explicitly so a missing trendline
    reports a neutral signal rather than -purchasePrice/atrValue.
    """
    is_active = bool(tl.get("isActive", False))
    if not ready_flag or not is_active:
        return [
            1.0 if ready_flag else 0.0,
            1.0 if is_active else 0.0,
            0.0, 0.0, 0.0, 0.0,
        ]
    slope = float(tl.get("slope", 0.0))
    intercept = float(tl.get("intercept", 0.0))
    date_diff = float(tl.get("dateDifference", 0.0))
    projected = intercept + slope * date_diff
    return [
        1.0,
        1.0,
        _safe_div(slope, mean_price),
        _safe_div(intercept - purchase_price, atr_value),
        date_diff,
        _safe_div(purchase_price - projected, atr_value),
    ]


def extract_context_features(ctx: dict, purchase_price: float) -> list:
    """
    Flatten one DowContext JSON object into 76 stationary, ATR/return-normalized floats.

    Block layout:
      priceStatistics   (9)   ATR-distance + price CoV
      volumeStatistics  (9)   log-transform + volume CoV
      macd block        (3)   macd/mean, signal/mean, macdReady
      trend             (17)  days-since-e1 + ATR-distance closes
      trendLine         (6)   slope/mean, ATR-distance intercept, projected distance
      doubleTrend       (17)
      doubleTrendLine   (6)
      rsi / doubleRsi   (4)   already in [0, 100], unchanged
      atr block         (5)   atr/price, doubleAtr/price, ready flags, volatility_ratio
    """
    atr_value = float(ctx.get("atrValue", 0.0))
    double_atr_value = float(ctx.get("doubleAtrValue", 0.0))
    price_stats = ctx.get("priceStatistics", {})
    mean_price = float(price_stats.get("mean", 0.0))

    features = []
    features.extend(_price_stats_features(price_stats, purchase_price, atr_value))
    features.extend(_volume_stats_features(ctx.get("volumeStatistics", {})))
    features.append(_safe_div(float(ctx.get("macd", 0.0)), mean_price))
    features.append(_safe_div(float(ctx.get("signal", 0.0)), mean_price))
    features.append(1.0 if ctx.get("macdReady", False) else 0.0)
    features.extend(_trend_features(ctx.get("trendReady", False), ctx.get("trend", {}),
                                    purchase_price, atr_value))
    features.extend(_trendline_features(ctx.get("trendLineReady", False), ctx.get("trendLine", {}),
                                        purchase_price, atr_value, mean_price))
    features.extend(_trend_features(ctx.get("doubleTrendReady", False), ctx.get("doubleTrend", {}),
                                    purchase_price, double_atr_value))
    features.extend(_trendline_features(ctx.get("doubleTrendLineReady", False), ctx.get("doubleTrendLine", {}),
                                        purchase_price, double_atr_value, mean_price))
    features.append(float(ctx.get("rsiValue", 50.0)))
    features.append(1.0 if ctx.get("rsiReady", False) else 0.0)
    features.append(float(ctx.get("doubleRsiValue", 50.0)))
    features.append(1.0 if ctx.get("doubleRsiReady", False) else 0.0)
    features.append(_safe_div(atr_value, purchase_price))
    features.append(1.0 if ctx.get("atrReady", False) else 0.0)
    features.append(_safe_div(double_atr_value, purchase_price))
    features.append(1.0 if ctx.get("doubleAtrReady", False) else 0.0)
    features.append(_safe_div(atr_value, double_atr_value))
    return features  # 9+9+3+17+6+17+6+4+5 = 76


# ---------------------------------------------------------------------------
# Data loading
# ---------------------------------------------------------------------------

def load_positions(path: str):
    """
    Load data.json and return (X, y, dates) arrays.

    Filters out positions where pnl == 0.0 AND sellDate is empty
    (unclosed / still-open trades).

    Feature vector per position (77 total):
      [positionType, *entryContext×76]

    Label: 1 if pnl > 0 else 0
    dates: list of purchaseDate strings (ISO format) for temporal splitting
    """
    with open(path, "r") as f:
        positions = json.load(f)

    X_rows, y_rows, dates = [], [], []
    skipped = 0

    for pos in positions:
        pnl = float(pos.get("pnl", 0.0))
        sell_date = pos.get("sellDate", "")

        if pnl == 0.0 and sell_date == "":
            skipped += 1
            continue

        position_type = 1.0 if pos.get("positionType", "LONG") == "LONG" else 0.0
        purchase_price = float(pos.get("purchasePrice", 0.0))
        ctx_features = extract_context_features(pos.get("entryContext", {}), purchase_price)

        X_rows.append([position_type] + ctx_features)
        y_rows.append(1 if pnl > 0.0 else 0)
        dates.append(pos.get("purchaseDate", ""))

    if skipped:
        print(f"[data]  Skipped {skipped} unclosed positions.")

    return np.array(X_rows, dtype=np.float32), np.array(y_rows, dtype=np.float32), dates


# ---------------------------------------------------------------------------
# Normalisation
# ---------------------------------------------------------------------------

SCALER_FACTORIES = {
    "standard": StandardScaler,
    "robust":   RobustScaler,
}


def _winsorize_fit(X_train, lower_pct: float, upper_pct: float):
    """Compute per-column clip bounds from the training split."""
    lo = np.percentile(X_train, lower_pct, axis=0)
    hi = np.percentile(X_train, upper_pct, axis=0)
    return lo, hi


def _winsorize_apply(X, lo, hi):
    return np.clip(X, lo, hi)


def normalise(X_train, X_val, X_test, scaler_path: str,
              scaler_kind: str = "standard",
              winsorize_pct: float = 1.0):
    """
    Fit scaler on train; transform all splits. Save scaler bundle.

    scaler_kind: "standard" or "robust" — selects StandardScaler vs RobustScaler.
    winsorize_pct: clip each feature to [pct, 100-pct] percentile bounds computed
        on the train split, applied to all splits before scaling. 0 disables it.

    The saved pickle bundles {scaler, lo, hi, kind} so inference can replay both
    the clipping and the scaling deterministically.
    """
    if scaler_kind not in SCALER_FACTORIES:
        raise ValueError(f"Unknown scaler_kind={scaler_kind!r}. "
                         f"Expected one of {list(SCALER_FACTORIES)}")

    lo = hi = None
    if winsorize_pct > 0:
        lo, hi = _winsorize_fit(X_train, winsorize_pct, 100.0 - winsorize_pct)
        X_train = _winsorize_apply(X_train, lo, hi)
        X_val   = _winsorize_apply(X_val,   lo, hi)
        X_test  = _winsorize_apply(X_test,  lo, hi)
        print(f"[norm]  Winsorized at [{winsorize_pct}%, {100.0 - winsorize_pct}%]")

    scaler = SCALER_FACTORIES[scaler_kind]()
    X_train = scaler.fit_transform(X_train)
    X_val   = scaler.transform(X_val)
    X_test  = scaler.transform(X_test)

    with open(scaler_path, "wb") as f:
        pickle.dump({
            "scaler": scaler,
            "winsor_lo": lo,
            "winsor_hi": hi,
            "scaler_kind": scaler_kind,
            "winsorize_pct": winsorize_pct,
        }, f)
    print(f"[norm]  Scaler ({scaler_kind}) saved -> {scaler_path}")
    return X_train, X_val, X_test


# ---------------------------------------------------------------------------
# Leaderboard printing
# ---------------------------------------------------------------------------

def _row(rank: int, family: str, name: str, r: TrainResult) -> str:
    return (
        f"  {rank:>4}  {family:<8}  {name:<30}  "
        f"{r.accuracy:.4f}  {r.precision:.4f}  "
        f"{r.recall:.4f}  {r.f1:.4f}  {r.auc_roc:.4f}"
    )


def _header() -> str:
    return (
        f"  {'Rank':>4}  {'Family':<8}  {'Config':<30}  "
        f"{'Acc':<6}  {'Prec':<6}  {'Recall':<6}  {'F1':<6}  {'AUC-ROC'}"
    )


def _divider(width: int = 88) -> str:
    return "  " + "-" * width


def print_leaderboard(results: list, n_total: int):
    """Print top-10 overall, tree model summary, and top-5 per NN family."""
    W = 90

    print()
    print("=" * W)
    print(f"  ARCHITECTURE GRID SEARCH — TOP 10 OVERALL  ({n_total} configs, sorted by AUC-ROC)")
    print("=" * W)
    print(_header())
    print(_divider())

    for i, entry in enumerate(results[:10], start=1):
        print(_row(i, entry["family"], entry["cfg"]["name"], entry["result"]))

    # Tree model summary (always shown in full, regardless of overall rank)
    tree_families = ("xgboost", "lightgbm")
    tree_entries = [e for e in results if e["family"] in tree_families]
    if tree_entries:
        print()
        print("  --- GRADIENT BOOSTING BASELINE ---")
        print(_header())
        print(_divider())
        for entry in tree_entries:
            overall_rank = results.index(entry) + 1
            print(_row(overall_rank, entry["family"], entry["cfg"]["name"], entry["result"]))

    # Per-family top 5 (NN families only)
    for family in ("funnel", "diamond", "cylinder"):
        family_results = [e for e in results if e["family"] == family]
        print()
        print(f"  --- TOP 5: {family.upper()} ---")
        print(_header())
        print(_divider())
        for i, entry in enumerate(family_results[:5], start=1):
            overall_rank = results.index(entry) + 1
            print(_row(overall_rank, entry["family"], entry["cfg"]["name"], entry["result"]))

    print()
    print("=" * W)
    best = results[0]
    print(
        f"  Best overall: {best['family']}/{best['cfg']['name']}  "
        f"AUC-ROC={best['result'].auc_roc:.4f}"
    )
    print("=" * W)


# ---------------------------------------------------------------------------
# Main
# ---------------------------------------------------------------------------

def main():
    parser = argparse.ArgumentParser(
        description="Multi-architecture grid search for trade profitability classifier"
    )
    parser.add_argument(
        "--data",
        default=os.path.join(os.path.dirname(__file__), "..", "output", "data.json"),
        help="Path to data.json (default: output/data.json relative to repo root)",
    )
    parser.add_argument("--epochs",      type=int,   default=100)
    parser.add_argument("--batch-size",  type=int,   default=64)
    parser.add_argument("--lr",          type=float, default=1e-3)
    parser.add_argument("--patience",    type=int,   default=15)
    parser.add_argument("--seed",        type=int,   default=42)
    parser.add_argument(
        "--fast",
        action="store_true",
        help="Quick sweep: epochs=50, patience=7 (for development iteration)",
    )
    parser.add_argument(
        "--scaler",
        choices=list(SCALER_FACTORIES.keys()),
        default="standard",
        help="Feature scaler: 'standard' (mean/std) or 'robust' (median/IQR)",
    )
    parser.add_argument(
        "--winsorize",
        type=float,
        default=1.0,
        help="Per-feature clip percentile (0 disables). Default 1.0 = clip to [1%%, 99%%]",
    )
    args = parser.parse_args()

    if args.fast:
        args.epochs   = 50
        args.patience = 7

    torch.manual_seed(args.seed)
    np.random.seed(args.seed)

    data_path = os.path.abspath(args.data)
    if not os.path.exists(data_path):
        print(f"ERROR: data.json not found at {data_path}", file=sys.stderr)
        sys.exit(1)

    # ------------------------------------------------------------------
    # Data — load, split, normalise (done once for all architectures)
    # ------------------------------------------------------------------
    print(f"[data]  Loading {data_path}")
    X, y, dates = load_positions(data_path)
    print(f"[data]  Feature matrix: {X.shape}  (expected N×77)")

    # Temporal split — sort by purchaseDate ascending, slice sequentially.
    # Prevents data leakage from future market regimes into training.
    order = sorted(range(len(dates)), key=lambda i: dates[i])
    X, y = X[order], y[order]
    dates_sorted = [dates[i] for i in order]

    n = len(X)
    n_train = int(n * 0.70)
    n_val   = int(n * 0.85)   # 70%–85% = val (15%), 85%–100% = test (15%)

    X_train, y_train = X[:n_train],      y[:n_train]
    X_val,   y_val   = X[n_train:n_val], y[n_train:n_val]
    X_test,  y_test  = X[n_val:],        y[n_val:]

    print(f"[data]  Temporal split -> train={len(X_train)}  val={len(X_val)}  test={len(X_test)}")
    print(f"[data]  Train dates: {dates_sorted[0]} to {dates_sorted[n_train - 1]}")
    print(f"[data]  Val   dates: {dates_sorted[n_train]} to {dates_sorted[n_val - 1]}")
    print(f"[data]  Test  dates: {dates_sorted[n_val]} to {dates_sorted[-1]}")

    # Class balance in test set
    n_profitable = int(y_test.sum())
    n_loss = len(y_test) - n_profitable
    print(f"[data]  Test class balance: {n_profitable} profitable / {n_loss} loss")

    scaler_path = os.path.join(os.path.dirname(__file__), "scaler.pkl")
    X_train, X_val, X_test = normalise(
        X_train, X_val, X_test, scaler_path,
        scaler_kind=args.scaler,
        winsorize_pct=args.winsorize,
    )

    # Class-imbalance correction: penalise missing profitable trades proportionally
    num_pos = float(y_train.sum())
    num_neg = float(len(y_train) - num_pos)
    pos_weight = torch.tensor([num_neg / num_pos], dtype=torch.float32)
    print(f"[train] pos_weight={pos_weight.item():.4f}  (num_neg/num_pos = {num_neg:.0f}/{num_pos:.0f})")

    train_loader = make_loader(X_train, y_train, args.batch_size, shuffle=True)
    val_loader   = make_loader(X_val,   y_val,   args.batch_size, shuffle=False)

    # ------------------------------------------------------------------
    # Grid search — collect all 51 configs and run them
    # ------------------------------------------------------------------
    all_configs = []
    for family_name, cfg_fn in [
        ("funnel",   funnel_configs),
        ("diamond",  diamond_configs),
        ("cylinder", cylinder_configs),
    ]:
        for cfg in cfg_fn():
            all_configs.append({"family": family_name, **cfg})

    n_total = len(all_configs)
    print(f"\n[sweep] Running {n_total} configurations  "
          f"(epochs={args.epochs}  patience={args.patience}  lr={args.lr})\n")

    results = []
    for i, cfg in enumerate(all_configs, start=1):
        label = f"{cfg['family']}/{cfg['name']}"
        print(f"[{i:02d}/{n_total}] {label:<42} ", end="", flush=True)

        model = GenericMLP(
            input_dim=X_train.shape[1],
            hidden_dims=cfg["hidden_dims"],
            dropout=cfg["dropout"],
        )
        model = train_model(
            model, train_loader, val_loader,
            epochs=args.epochs,
            patience=args.patience,
            lr=args.lr,
            pos_weight=pos_weight,
            verbose=False,
        )
        result = evaluate(model, X_test, y_test)
        results.append({"cfg": cfg, "family": cfg["family"], "result": result, "model": model})
        print(
            f"AUC={result.auc_roc:.4f}  F1={result.f1:.4f}  "
            f"Recall={result.recall:.4f}  Acc={result.accuracy:.4f}"
        )

    # ------------------------------------------------------------------
    # Gradient boosting baseline (XGBoost + LightGBM)
    # ------------------------------------------------------------------
    print("\n[tree]  Running gradient boosting baseline models...")
    tree_results = run_tree_models(
        X_train, y_train, X_val, y_val, X_test, y_test,
        pos_weight=float(pos_weight.item()),
    )
    results.extend(tree_results)
    n_total += len(tree_results)

    # ------------------------------------------------------------------
    # Sort and display leaderboard
    # ------------------------------------------------------------------
    results.sort(key=lambda e: (e["result"].auc_roc, e["result"].f1), reverse=True)
    print_leaderboard(results, n_total)

    # ------------------------------------------------------------------
    # Save best NN model (tree models have model=None so skip them)
    # ------------------------------------------------------------------
    best = next(e for e in results if e["model"] is not None)
    nn_dir = os.path.dirname(__file__)

    model_path = os.path.join(nn_dir, "model.pt")
    torch.save(best["model"].state_dict(), model_path)

    metadata_path = os.path.join(nn_dir, "model_metadata.json")
    with open(metadata_path, "w") as f:
        json.dump(
            {
                "family":      best["family"],
                "name":        best["cfg"]["name"],
                "hidden_dims": best["cfg"]["hidden_dims"],
                "dropout":     best["cfg"]["dropout"],
                "auc_roc":     best["result"].auc_roc,
                "f1":          best["result"].f1,
                "accuracy":    best["result"].accuracy,
                "precision":   best["result"].precision,
                "recall":      best["result"].recall,
            },
            f, indent=2,
        )

    print(f"\n[save]  Best model  -> {model_path}")
    print(f"[save]  Metadata    -> {metadata_path}")
    print(f"[save]  Scaler      -> {scaler_path}")


if __name__ == "__main__":
    main()
