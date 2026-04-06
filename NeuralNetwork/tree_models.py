"""
Gradient boosting baseline models (XGBoost and LightGBM).

Trains one XGBClassifier and one LGBMClassifier on the same splits produced
by neural_network.py, using the same class-imbalance correction factor.
Returns results as TrainResult dataclasses so they can be merged directly
into the neural network leaderboard.
"""

import numpy as np
from xgboost import XGBClassifier
from lightgbm import LGBMClassifier
from sklearn.metrics import (
    accuracy_score,
    precision_score,
    recall_score,
    f1_score,
    roc_auc_score,
)

from architectures.base import TrainResult


def _evaluate(model, X_test: np.ndarray, y_test: np.ndarray) -> TrainResult:
    """Compute the standard five metrics for a fitted sklearn-API classifier."""
    probs = model.predict_proba(X_test)[:, 1]
    preds = (probs >= 0.5).astype(int)
    return TrainResult(
        accuracy=float(accuracy_score(y_test, preds)),
        precision=float(precision_score(y_test, preds, zero_division=0)),
        recall=float(recall_score(y_test, preds, zero_division=0)),
        f1=float(f1_score(y_test, preds, zero_division=0)),
        auc_roc=float(roc_auc_score(y_test, probs)),
    )


def run_tree_models(
    X_train: np.ndarray,
    y_train: np.ndarray,
    X_val: np.ndarray,
    y_val: np.ndarray,
    X_test: np.ndarray,
    y_test: np.ndarray,
    pos_weight: float,
) -> list[dict]:
    """
    Train XGBoost and LightGBM and return results in the same dict format
    used by the neural network sweep:
        {"cfg": {...}, "family": str, "result": TrainResult, "model": None}

    pos_weight: num_neg / num_pos (same value used for BCEWithLogitsLoss).
    """
    results = []

    # XGBoost — use eval_set for early stopping on val split
    print(f"[tree]  Training XGBoost (scale_pos_weight={pos_weight:.4f}) ...", flush=True)
    xgb = XGBClassifier(
        n_estimators=500,
        max_depth=6,
        learning_rate=0.05,
        scale_pos_weight=pos_weight,
        subsample=0.8,
        colsample_bytree=0.8,
        eval_metric="logloss",
        early_stopping_rounds=20,
        verbosity=0,
        random_state=42,
    )
    xgb.fit(X_train, y_train, eval_set=[(X_val, y_val)], verbose=False)
    xgb_result = _evaluate(xgb, X_test, y_test)
    print(
        f"[tree]  XGBoost done  "
        f"AUC={xgb_result.auc_roc:.4f}  F1={xgb_result.f1:.4f}  "
        f"Recall={xgb_result.recall:.4f}  Acc={xgb_result.accuracy:.4f}"
    )
    results.append({
        "cfg":    {"name": "xgboost_d6_n500", "hidden_dims": [], "dropout": 0.0},
        "family": "xgboost",
        "result": xgb_result,
        "model":  None,
    })

    # LightGBM — use val split for early stopping
    print(f"[tree]  Training LightGBM (class_weight balanced) ...", flush=True)
    lgbm = LGBMClassifier(
        n_estimators=500,
        num_leaves=63,
        learning_rate=0.05,
        class_weight="balanced",
        subsample=0.8,
        colsample_bytree=0.8,
        random_state=42,
        verbosity=-1,
    )
    lgbm.fit(
        X_train, y_train,
        eval_set=[(X_val, y_val)],
        callbacks=[],
    )
    lgbm_result = _evaluate(lgbm, X_test, y_test)
    print(
        f"[tree]  LightGBM done "
        f"AUC={lgbm_result.auc_roc:.4f}  F1={lgbm_result.f1:.4f}  "
        f"Recall={lgbm_result.recall:.4f}  Acc={lgbm_result.accuracy:.4f}"
    )
    results.append({
        "cfg":    {"name": "lightgbm_l63_n500", "hidden_dims": [], "dropout": 0.0},
        "family": "lightgbm",
        "result": lgbm_result,
        "model":  None,
    })

    return results
