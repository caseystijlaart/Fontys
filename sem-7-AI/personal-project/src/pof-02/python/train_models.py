"""Train baseline models for POF-02 and export parameters.

Uses real datetime columns for time-aware features + chronological validation split.
"""

from pathlib import Path
import json

import numpy as np
import pandas as pd
from sklearn.linear_model import LogisticRegression
from sklearn.metrics import classification_report
from sklearn.neural_network import MLPClassifier
from sklearn.preprocessing import StandardScaler
from sklearn.tree import DecisionTreeClassifier

ROOT = Path(__file__).resolve().parent
DATA_FILE = ROOT / "data" / "pof02_synthetic.csv"
OUT_FILE = ROOT / "data" / "model_export.json"

BASE_FEATURES = [
    "soil_now",
    "moisture_mean",
    "moisture_delta",
    "moisture_slope",
    "dry_duration",
    "temp_now",
    "humidity_now",
    "light_now",
]

TIME_FEATURES = [
    "hour_sin",
    "hour_cos",
    "dow_sin",
    "dow_cos",
]

FEATURES = BASE_FEATURES + TIME_FEATURES

df = pd.read_csv(DATA_FILE)
df["timestamp"] = pd.to_datetime(df["timestamp_iso"], utc=True)

# Cyclical time encoding (actual date/time-aware features)
df["hour_sin"] = np.sin(2 * np.pi * df["hour_of_day"] / 24.0)
df["hour_cos"] = np.cos(2 * np.pi * df["hour_of_day"] / 24.0)
df["dow_sin"] = np.sin(2 * np.pi * df["day_of_week"] / 7.0)
df["dow_cos"] = np.cos(2 * np.pi * df["day_of_week"] / 7.0)

# Chronological split: oldest 80% train, newest 20% test
sorted_df = df.sort_values("timestamp_unix").reset_index(drop=True)
cut = int(len(sorted_df) * 0.8)
train_df = sorted_df.iloc[:cut]
test_df = sorted_df.iloc[cut:]

X_train, X_test = train_df[FEATURES], test_df[FEATURES]
y_train, y_test = train_df["risk"], test_df["risk"]

scaler = StandardScaler()
X_train_scaled = scaler.fit_transform(X_train)
X_test_scaled = scaler.transform(X_test)

log_reg = LogisticRegression(max_iter=500)
log_reg.fit(X_train_scaled, y_train)

mlp = MLPClassifier(hidden_layer_sizes=(12,), random_state=7, max_iter=600)
mlp.fit(X_train_scaled, y_train)

dtree = DecisionTreeClassifier(max_depth=4, random_state=7)
dtree.fit(X_train, y_train)

print("=== Logistic Regression ===")
print(classification_report(y_test, log_reg.predict(X_test_scaled), digits=3))
print("=== MLP ===")
print(classification_report(y_test, mlp.predict(X_test_scaled), digits=3))
print("=== Decision Tree ===")
print(classification_report(y_test, dtree.predict(X_test), digits=3))

export = {
    "feature_order": FEATURES,
    "base_feature_order": BASE_FEATURES,
    "time_feature_order": TIME_FEATURES,
    "scaler": {"mean": scaler.mean_.tolist(), "std": scaler.scale_.tolist()},
    "time_split": {
        "train_end_timestamp_unix": int(train_df["timestamp_unix"].iloc[-1]),
        "test_start_timestamp_unix": int(test_df["timestamp_unix"].iloc[0]),
    },
    "mlp": {
        "coefs": [c.tolist() for c in mlp.coefs_],
        "intercepts": [b.tolist() for b in mlp.intercepts_],
        "classes": mlp.classes_.tolist(),
    },
    "decision_tree": {
        "feature": dtree.tree_.feature.tolist(),
        "threshold": dtree.tree_.threshold.tolist(),
        "children_left": dtree.tree_.children_left.tolist(),
        "children_right": dtree.tree_.children_right.tolist(),
        "value": dtree.tree_.value.tolist(),
    },
}

OUT_FILE.write_text(json.dumps(export, indent=2), encoding="utf-8")
print(f"Exported model params to {OUT_FILE}")
