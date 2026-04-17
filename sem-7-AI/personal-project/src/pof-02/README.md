# POF-02 — Real-time Plant Monitoring (Revised POC)

This folder contains a revised proof of concept based on `src/pof`, now prepared to use **existing TinyML ecosystems on ESP32** instead of hand-written ML math.

## Goal
Build a real-time monitoring pipeline:

`Sensors -> Preprocessing -> Classification -> Temporal Analysis -> ML Layer -> Recommendation`

## C++ modules
- Sensor layer: moisture, temp/humidity, light from a standard LDR + 10k resistor (0 = dark) with calibration + averaging
- Classification layer: `kLow  / kOk / kHigh` per environmental factor
- Stress layer: per-factor and combined stress
- Temporal core: circular history buffer with mean, delta, slope, dry-duration proxies
- Feature engineering: compact feature vector
- ML layer: pluggable backend (`RULE_BASED`, `TINYML_TFLM`, `EDGE_IMPULSE`)
- Recommendation engine: actionable outputs (`water`, `reduceTemp`, `increaseHumidity`, `increaseLight`)

## TinyML-ready backend strategy
The ML abstraction lives in `MLLayer` and supports swapping backends:
1. `RULE_BASED` (local/dev fallback, deterministic)
2. `TINYML_TFLM` (TensorFlow Lite Micro / EloquentTinyML adapter point)
3. `EDGE_IMPULSE` (Edge Impulse C++ SDK adapter point)

This lets you train using external platforms and only map outputs to `MLResult` on-device.

## ESP32 integration suggestions
### Option A — Edge Impulse (recommended for anomaly + classification)
- Train impulse with the same engineered features.
- Export Arduino/C++ library.
- Enable `POF02_ENABLE_EDGE_IMPULSE` and call `run_classifier()` inside `EdgeImpulseBackend`.

### Option B — TensorFlow Lite Micro / EloquentTinyML
- Train tiny model (MLP/tree equivalent) offline.
- Export `.tflite` and convert to C array.
- Enable `POF02_ENABLE_TINYML_TFLM` and call inference inside `TinyMLTFLMBackend`.

## Time-aware dataset note
Synthetic training data now uses real timestamps (e.g. `2026-03-04 10:20:00`) with unix epoch and cyclical time features (`hour_sin/cos`, `dow_sin/cos`) so training is date/time-aware instead of simple step ids.

## Build and upload (Arduino/PlatformIO)
```bash
platformio run
platformio run -t upload
platformio device monitor -b 115200
```

## Python workflow
```bash
python3 python/generate_training_data.py
python3 python/train_models.py
python3 python/validate_models.py
python3 python/export_for_tinyml.py
```


## CSV logging on your laptop
`main.cpp` now streams a CSV header + full data rows over Serial every cycle (human-readable UTC timestamp like `2026-03-04 12:10`, raw sensors including LDR light level %, ML risk, class probabilities, confidence, recommendation booleans, summary, and engineered features).

To persist that stream directly to a CSV file on your laptop:

```bash
python3 -m pip install pyserial
python3 python/log_serial_to_csv.py --port /dev/ttyUSB0 --baud 115200 --output ~/pof02-runs/run1.csv
```

Notes:
- The firmware uses a configurable start timestamp (`2026-03-04 12:10 UTC` by default) and advances it with elapsed runtime, so CSV output aligns with date-time based datasets.
- On Windows, use `--port COM4` (or your active COM port).
- The script only writes valid CSV lines from the device and keeps informational boot logs in terminal output.
