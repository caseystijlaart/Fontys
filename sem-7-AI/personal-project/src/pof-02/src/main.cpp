#include <Arduino.h>

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

        auto rec = monitoringSystem.RunCycle();
        Serial.println(rec.summary.c_str());
    }
}