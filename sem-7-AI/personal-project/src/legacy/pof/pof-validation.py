import pandas as pd
import numpy as np

# paste your trained model params
mean = np.array([25.04231369, 8.39165835, 600.06397933])
std = np.array([8.77016247, 5.48159131, 225.02376219])

W1 = np.array([
    [-1.66090395,  1.28863049,  1.23965745,  0.25400533, -1.21944919, -2.50832765, -1.63462578,  0.85975954],
    [-0.02208931, -0.01401896, -0.13590374,  0.57209713,  0.82869254, -0.05283282,  0.00269596, -0.86147397],
    [-0.05387287, -0.23540778, -0.04325007, -0.29359517, -0.43342775, -0.30334154, -0.05451270, -0.18861791]
])

b1 = np.array([1.13610458, 0.51206488, 0.17462122, 0.93053437,
               0.41859057, -1.08522127, 1.14938218, 0.41616960])

W2 = np.array([
    [-3.02247220,  1.30793735,  0.88963572],
    [ 0.77690098,  0.06565555, -1.09702521],
    [-0.30470337,  0.70747872, -0.61820458],
    [ 0.24103667, -1.19770835,  0.89838840],
    [-1.61411744,  0.86172138, -0.21602353],
    [-0.32693233,  1.73635854, -2.54433441],
    [-0.55514585,  0.82761545,  1.15932432],
    [ 0.31188915, -0.15159676,  1.16560239]
])

b2 = np.array([-0.58318004, -1.11053463, -0.14287843])

labels = {
    0: "Healthy",
    1: "High Stress",
    2: "Moderate Stress"
}

def relu(x):
    return np.maximum(0, x)

def softmax(x):
    e = np.exp(x - np.max(x, axis=1, keepdims=True))
    return e / e.sum(axis=1, keepdims=True)

df = pd.read_csv("esp32_log.csv")

X = df[["soil_moisture", "humidity_stress", "moisture_temp_interaction"]].values
Xn = (X - mean) / std

hidden = relu(Xn @ W1 + b1)
logits = hidden @ W2 + b2
probs = softmax(logits)

df["laptop_pred_idx"] = np.argmax(probs, axis=1)
df["laptop_pred"] = df["laptop_pred_idx"].map(labels)
df["laptop_confidence"] = np.max(probs, axis=1)

df["match"] = df["predicted_class"].str.strip() == df["laptop_pred"]

print(df[["predicted_class", "laptop_pred", "confidence", "laptop_confidence", "match"]].head())
print("Match rate:", df["match"].mean())

df.to_csv("esp32_vs_laptop_comparison.csv", index=False)