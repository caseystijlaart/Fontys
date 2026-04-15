import pandas as pd
import numpy as np

from sklearn.model_selection import train_test_split
from sklearn.preprocessing import StandardScaler, LabelEncoder
from sklearn.metrics import accuracy_score, classification_report, confusion_matrix
from sklearn.linear_model import LogisticRegression
from sklearn.neural_network import MLPClassifier

df = pd.read_csv("sem-7-AI\personal-project\datasets\datasets\plant_health_data.csv")

# Ideal values for this plant
ideal_humidity = 60
ideal_soil_moisture = 35

# Engineered features
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

X = dataset[feature_columns].values
y = dataset[target_column].values

label_encoder = LabelEncoder()
y_encoded = label_encoder.fit_transform(y)

X_train, X_test, y_train, y_test = train_test_split(
    X,
    y_encoded,
    test_size=0.2,
    random_state=42,
    stratify=y_encoded
)

scaler = StandardScaler()
X_train_scaled = scaler.fit_transform(X_train)
X_test_scaled = scaler.transform(X_test)

log_model = LogisticRegression(max_iter=2000)
log_model.fit(X_train_scaled, y_train)
log_pred = log_model.predict(X_test_scaled)

print("=== Logistic Regression ===")
print("Accuracy:", accuracy_score(y_test, log_pred))
print(classification_report(y_test, log_pred, target_names=label_encoder.classes_))

mlp_model = MLPClassifier(
    hidden_layer_sizes=(8,),
    activation="relu",
    max_iter=2000,
    random_state=42
)
mlp_model.fit(X_train_scaled, y_train)
mlp_pred = mlp_model.predict(X_test_scaled)

print("=== Tiny MLP ===")
print("Accuracy:", accuracy_score(y_test, mlp_pred))
print(classification_report(y_test, mlp_pred, target_names=label_encoder.classes_))

print("\nScaler means:")
print(scaler.mean_)

print("\nScaler stds:")
print(scaler.scale_)

print("\nLabel mapping:")
for i, label in enumerate(label_encoder.classes_):
    print(i, "->", label)

print("\nLogistic Regression coefficients:")
print(log_model.coef_)

print("\nLogistic Regression intercept:")
print(log_model.intercept_)

print("\nMLP layer 1 weights:")
print(mlp_model.coefs_[0])

print("\nMLP layer 1 bias:")
print(mlp_model.intercepts_[0])

print("\nMLP output weights:")
print(mlp_model.coefs_[1])

print("\nMLP output bias:")
print(mlp_model.intercepts_[1])