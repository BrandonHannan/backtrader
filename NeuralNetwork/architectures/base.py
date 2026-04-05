"""
Shared runtime for all architecture families.

Provides:
  - GenericMLP  : shape-agnostic feedforward network (raw logit output)
  - make_loader : wraps numpy arrays in a DataLoader
  - train_model : Adam + BCEWithLogitsLoss + LR scheduler + early stopping
  - evaluate    : returns a TrainResult dataclass (does not print)
  - TrainResult : dataclass holding the 5 test metrics
"""

from dataclasses import dataclass
from typing import Optional

import numpy as np
import torch
import torch.nn as nn
from torch.utils.data import DataLoader, TensorDataset
from sklearn.metrics import (
    accuracy_score,
    precision_score,
    recall_score,
    f1_score,
    roc_auc_score,
)


# ---------------------------------------------------------------------------
# Result container
# ---------------------------------------------------------------------------

@dataclass
class TrainResult:
    accuracy: float
    precision: float
    recall: float
    f1: float
    auc_roc: float


# ---------------------------------------------------------------------------
# Model
# ---------------------------------------------------------------------------

class GenericMLP(nn.Module):
    """
    Shape-agnostic multi-layer perceptron for binary classification.

    The output is a raw logit (no Sigmoid).  Use BCEWithLogitsLoss during
    training and torch.sigmoid() at inference time.

    Args:
        input_dim   : number of input features (64 for trade context)
        hidden_dims : list of hidden layer widths, e.g. [256, 128, 64]
        dropout     : dropout probability applied after each hidden layer
    """

    def __init__(self, input_dim: int, hidden_dims: list, dropout: float = 0.3):
        super().__init__()
        layers = []
        in_size = input_dim
        for out_size in hidden_dims:
            layers += [
                nn.Linear(in_size, out_size),
                nn.BatchNorm1d(out_size),
                nn.ReLU(),
                nn.Dropout(dropout),
            ]
            in_size = out_size
        layers.append(nn.Linear(in_size, 1))
        self.net = nn.Sequential(*layers)

    def forward(self, x: torch.Tensor) -> torch.Tensor:
        return self.net(x).squeeze(1)  # shape: (batch,)


# ---------------------------------------------------------------------------
# DataLoader factory
# ---------------------------------------------------------------------------

def make_loader(
    X: np.ndarray,
    y: np.ndarray,
    batch_size: int,
    shuffle: bool,
) -> DataLoader:
    """Wrap numpy arrays into a PyTorch DataLoader."""
    dataset = TensorDataset(torch.from_numpy(X), torch.from_numpy(y))
    return DataLoader(dataset, batch_size=batch_size, shuffle=shuffle)


# ---------------------------------------------------------------------------
# Training
# ---------------------------------------------------------------------------

def train_model(
    model: GenericMLP,
    train_loader: DataLoader,
    val_loader: DataLoader,
    epochs: int,
    patience: int,
    lr: float,
    pos_weight: Optional[torch.Tensor] = None,
    verbose: bool = False,
) -> GenericMLP:
    """
    Train model with Adam + BCEWithLogitsLoss + ReduceLROnPlateau + early stopping.

    Args:
        pos_weight : class-imbalance correction tensor, shape (1,).
                     Compute as: tensor([num_negative / num_positive]).
                     Penalises false negatives (missed profitable trades) proportionally.
        verbose    : print one line per epoch. Default False because the orchestrator
                     runs 51 models and per-epoch output would be overwhelming.

    Returns:
        Model with weights restored to the best validation-loss checkpoint.
    """
    criterion = nn.BCEWithLogitsLoss(pos_weight=pos_weight)
    optimizer = torch.optim.Adam(model.parameters(), lr=lr)
    scheduler = torch.optim.lr_scheduler.ReduceLROnPlateau(
        optimizer, mode="min", patience=5, factor=0.5
    )

    best_val_loss = float("inf")
    epochs_no_improve = 0
    best_state = None

    for epoch in range(1, epochs + 1):
        # Train
        model.train()
        total_loss = 0.0
        for X_batch, y_batch in train_loader:
            optimizer.zero_grad()
            logits = model(X_batch)
            loss = criterion(logits, y_batch)
            loss.backward()
            optimizer.step()
            total_loss += loss.item() * len(X_batch)
        train_loss = total_loss / len(train_loader.dataset)

        # Validate
        model.eval()
        val_loss = 0.0
        correct = 0
        with torch.no_grad():
            for X_batch, y_batch in val_loader:
                logits = model(X_batch)
                val_loss += criterion(logits, y_batch).item() * len(X_batch)
                probs = torch.sigmoid(logits)
                correct += ((probs >= 0.5).float() == y_batch).sum().item()
        val_loss /= len(val_loader.dataset)
        val_acc = correct / len(val_loader.dataset)

        scheduler.step(val_loss)

        if verbose:
            print(
                f"  Epoch {epoch:3d}/{epochs}  "
                f"train_loss={train_loss:.4f}  "
                f"val_loss={val_loss:.4f}  "
                f"val_acc={val_acc:.4f}"
            )

        # Early stopping
        if val_loss < best_val_loss:
            best_val_loss = val_loss
            epochs_no_improve = 0
            best_state = {k: v.clone() for k, v in model.state_dict().items()}
        else:
            epochs_no_improve += 1
            if epochs_no_improve >= patience:
                if verbose:
                    print(f"  Early stopping at epoch {epoch} (no improvement for {patience} epochs).")
                break

    if best_state is not None:
        model.load_state_dict(best_state)
    return model


# ---------------------------------------------------------------------------
# Evaluation
# ---------------------------------------------------------------------------

def evaluate(
    model: GenericMLP,
    X_test: np.ndarray,
    y_test: np.ndarray,
) -> TrainResult:
    """
    Run model on X_test and return a TrainResult.

    Applies sigmoid to raw logits before thresholding at 0.5.
    Does NOT print anything — the orchestrator owns all output formatting.
    """
    model.eval()
    with torch.no_grad():
        logits = model(torch.from_numpy(X_test))
        probs = torch.sigmoid(logits).numpy()

    preds = (probs >= 0.5).astype(int)
    labels = y_test.astype(int)

    return TrainResult(
        accuracy=float(accuracy_score(labels, preds)),
        precision=float(precision_score(labels, preds, zero_division=0)),
        recall=float(recall_score(labels, preds, zero_division=0)),
        f1=float(f1_score(labels, preds, zero_division=0)),
        auc_roc=float(roc_auc_score(labels, probs)),
    )
