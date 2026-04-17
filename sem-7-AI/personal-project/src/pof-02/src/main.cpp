#include <chrono>
#include <iostream>
#include <thread>

#include "MonitoringSystem.hpp"

int main() {
    using namespace pof02;

    SoilMoistureSensor soil(/*pin=*/34, /*adcDry=*/3200.0f, /*adcWet=*/1200.0f);
    TempHumiditySensor dht(/*pin=*/4, /*dhtType=*/22);
    LightSensor light(/*pin=*/35);

    MLLayer ml(MLBackend::TINYML_TFLM);
    RecommendationEngine recEngine;
    MonitoringSystem system(soil, dht, light, ml, recEngine, 24);

    if (!system.Init()) {
        std::cerr << "Failed to initialize monitoring system\n";
        return 1;
    }

#ifdef ARDUINO
    while (true) {
        auto rec = system.RunCycle();
        Serial.println(rec.summary.c_str());
        delay(10000);
    }
#else
    for (int i = 0; i < 6; ++i) {
        auto rec = system.RunCycle();
        std::cout << "Cycle " << i + 1 << ": " << rec.summary << '\n';
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
#endif

    return 0;
}
