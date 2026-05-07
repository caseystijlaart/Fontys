import pandas as pd
import numpy as np
from pathlib import Path
import matplotlib.pyplot as plt
import shap

from sklearn.model_selection import train_test_split
from sklearn.preprocessing import StandardScaler, LabelEncoder
from sklearn.metrics import accuracy_score, classification_report, confusion_matrix
from sklearn.linear_model import LogisticRegression
from sklearn.neural_network import MLPClassifier
from sklearn.inspection import permutation_importance

# -----------------------------
# Paths
# -----------------------------
ROOT = Path(__file__).resolve().parent
OUT_DIR = ROOT / "data"
PLOT_DIR = OUT_DIR / "plots"

OUT_DIR.mkdir(exist_ok=True)
PLOT_DIR.mkdir(exist_ok=True)

# -----------------------------
# Load data
# -----------------------------
df = pd.read_csv("sem-7-AI\\personal-project\\datasets\\datasets\\plant_health_data.csv")

# -----------------------------
# Feature engineering (PoC-01)
# -----------------------------
ideal_humidity = 60
ideal_soil_moisture = 35

df["humidity_stress"] = (df["Humidity"] - ideal_humidity).abs()
df["moisture_stress"] = (df["Soil_Moisture"] - ideal_soil_moisture).abs()
df["moisture_temp_interaction"] = df["Soil_Moisture"] * df["Ambient_Temperature"]

feature_columns = [
    "Soil_Moisture",
    "humidity_stress",
    "moisture_stress",
    "moisture_temp_interaction"
]

target_column = "Plant_Health_Status"

dataset = df[feature_columns + [target_column]].dropna().copy()

X = dataset[feature_columns]
y = dataset[target_column]

# -----------------------------
# Encoding
# -----------------------------
label_encoder = LabelEncoder()
y_encoded = label_encoder.fit_transform(y)

X_train, X_test, y_train, y_test = train_test_split(
    X,
    y_encoded,
    test_size=0.2,
    random_state=42,
    stratify=y_encoded
)

# -----------------------------
# Scaling
# -----------------------------
scaler = StandardScaler()
X_train_scaled = scaler.fit_transform(X_train)
X_test_scaled = scaler.transform(X_test)

# -----------------------------
# Models
# -----------------------------
log_model = LogisticRegression(max_iter=2000)
log_model.fit(X_train_scaled, y_train)
log_pred = log_model.predict(X_test_scaled)

mlp_model = MLPClassifier(
    hidden_layer_sizes=(8,),
    activation="relu",
    max_iter=2000,
    random_state=42
)
mlp_model.fit(X_train_scaled, y_train)
mlp_pred = mlp_model.predict(X_test_scaled)

# -----------------------------
# Evaluation
# -----------------------------
print("=== Logistic Regression ===")
print("Accuracy:", accuracy_score(y_test, log_pred))
print(classification_report(y_test, log_pred, target_names=label_encoder.classes_))

print("=== Tiny MLP ===")
print("Accuracy:", accuracy_score(y_test, mlp_pred))
print(classification_report(y_test, mlp_pred, target_names=label_encoder.classes_))

# -----------------------------
# Save predictions
# -----------------------------
results_df = pd.DataFrame({
    "true": y_test,
    "log_pred": log_pred,
    "mlp_pred": mlp_pred
})

results_file = OUT_DIR / "poc1_predictions.csv"
results_df.to_csv(results_file, index=False)
print(f"Saved predictions to {results_file}")

# -----------------------------
# Confusion matrix
# -----------------------------
cm = confusion_matrix(y_test, mlp_pred)
cm_df = pd.DataFrame(cm)

cm_file = OUT_DIR / "confusion_matrix.csv"
cm_df.to_csv(cm_file)
print(f"Saved confusion matrix to {cm_file}")

# -----------------------------
# Permutation importance (MLP)
# -----------------------------
perm = permutation_importance(
    mlp_model,
    X_test_scaled,
    y_test,
    n_repeats=10,
    random_state=42,
    scoring="accuracy"
)

perm_df = pd.DataFrame({
    "feature": feature_columns,
    "importance_mean": perm.importances_mean,
    "importance_std": perm.importances_std
}).sort_values("importance_mean", ascending=False)

perm_file = OUT_DIR / "permutation_importance.csv"
perm_df.to_csv(perm_file, index=False)

plt.figure()
plt.barh(perm_df["feature"], perm_df["importance_mean"])
plt.gca().invert_yaxis()
plt.title("Permutation Importance - PoC-01")
plt.tight_layout()
plt.savefig(PLOT_DIR / "perm_importance.png", dpi=200)
plt.close()

# -----------------------------
# SHAP (MLP)
# -----------------------------
# Sample for performance
X_sample = pd.DataFrame(X_test_scaled, columns=feature_columns).sample(100, random_state=42)

explainer = shap.Explainer(mlp_model.predict_proba, X_sample)
shap_values = explainer(X_sample)

# Summary plot
plt.figure()
shap.summary_plot(shap_values[..., 0], X_sample, show=False)
plt.tight_layout()
plt.savefig(PLOT_DIR / "shap_summary_class0.png", dpi=200)
plt.close()

# Mean absolute SHAP
mean_abs_shap = np.abs(shap_values.values[:, :, 0]).mean(axis=0)

shap_df = pd.DataFrame({
    "feature": feature_columns,
    "mean_abs_shap": mean_abs_shap
}).sort_values("mean_abs_shap", ascending=False)

shap_file = OUT_DIR / "shap_importance_class0.csv"
shap_df.to_csv(shap_file, index=False)

plt.figure()
plt.barh(shap_df["feature"], shap_df["mean_abs_shap"])
plt.gca().invert_yaxis()
plt.title("SHAP Importance - PoC-01 (Class 0)")
plt.tight_layout()
plt.savefig(PLOT_DIR / "shap_bar_class0.png", dpi=200)
plt.close()

print("Saved SHAP outputs.")