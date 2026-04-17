#include <Arduino.h>

#include <ctime>

#include "MonitoringSystem.hpp"

using namespace pof02;

SoilMoistureSensor soil(34, 3200.0f, 1200.0f);
TempHumiditySensor dht(4, 22);
LightSensor light(35);

MLLayer ml(MLBackend::TINYML_TFLM);
RecommendationEngine recEngine;
MonitoringSystem monitoringSystem(soil, dht, light, ml, recEngine, 24);

const unsigned long INTERVAL = 10000;
unsigned long lastRun = 0;
bool wroteCsvHeader = false;

void PrintCsvHeader() {
    Serial.println(
        "timestamp_utc,soil_moisture_pct,temperature_c,humidity_pct,light_level_pct,"
        "risk_class,confidence,prob_healthy,prob_moderate_stress,prob_high_stress,"
        "action_water,action_reduce_temp,action_increase_humidity,action_increase_light,"
        "recommendation_summary,"
        "feature0,feature1,feature2,feature3,feature4,feature5,feature6,feature7,feature8,feature9,feature10,feature11");
}

void PrintTimestampUtc(std::int64_t unixTime) {
    const std::time_t ts = static_cast<std::time_t>(unixTime);
    std::tm timeInfo{};

#if defined(ESP32)
    gmtime_r(&ts, &timeInfo);
#else
    const std::tm* utc = std::gmtime(&ts);
    if (utc == nullptr) {
        Serial.print("1970-01-01 00:00");
        return;
    }
    timeInfo = *utc;
#endif

    char buffer[20];
    const std::size_t len = std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M", &timeInfo);
    if (len == 0) {
        Serial.print("1970-01-01 00:00");
        return;
    }

    Serial.print(buffer);
}

void PrintCsvRow(const MonitoringCycleResult& result) {
    const auto& snapshot = result.snapshot;
    const auto& mlResult = result.mlResult;
    const auto& rec = result.recommendation;
    const auto& features = result.features.values;

    PrintTimestampUtc(snapshot.unixTime);
    Serial.print(',');
    Serial.print(snapshot.soilMoisturePct, 3);
    Serial.print(',');
    Serial.print(snapshot.temperatureC, 3);
    Serial.print(',');
    Serial.print(snapshot.humidityPct, 3);
    Serial.print(',');
    Serial.print(snapshot.lightLevelPct, 3);
    Serial.print(',');
    Serial.print(static_cast<int>(mlResult.risk));
    Serial.print(',');
    Serial.print(mlResult.confidence, 6);
    Serial.print(',');
    Serial.print(mlResult.probabilities[0], 6);
    Serial.print(',');
    Serial.print(mlResult.probabilities[1], 6);
    Serial.print(',');
    Serial.print(mlResult.probabilities[2], 6);
    Serial.print(',');
    Serial.print(rec.water ? 1 : 0);
    Serial.print(',');
    Serial.print(rec.reduceTemp ? 1 : 0);
    Serial.print(',');
    Serial.print(rec.increaseHumidity ? 1 : 0);
    Serial.print(',');
    Serial.print(rec.increaseLight ? 1 : 0);
    Serial.print(',');

    for (char ch : rec.summary) {
        if (ch == ',') {
            Serial.print(';');
        } else if (ch == '"') {
            Serial.print('\'');
        } else {
            Serial.print(ch);
        }
    }

    for (std::size_t i = 0; i < features.size(); ++i) {
        Serial.print(',');
        Serial.print(features[i], 6);
    }
    Serial.println();
}

void setup() {
    Serial.begin(115200);

    if (!monitoringSystem.Init()) {
        Serial.println("Failed to initialize monitoring system");
        while (true) {
        }
    }

    Serial.println("Monitoring system initialized");
}

void loop() {
    unsigned long now = millis();

    if (now - lastRun >= INTERVAL) {
        lastRun = now;

        const auto result = monitoringSystem.RunCycleDetailed();

        if (!wroteCsvHeader) {
            PrintCsvHeader();
            wroteCsvHeader = true;
        }

        PrintCsvRow(result);
    }
}
