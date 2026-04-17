#pragma once

#include <cstdint>

#include "LightSensor.hpp"
#include "MLLayer.hpp"
#include "RecommendationEngine.hpp"
#include "SensorHistory.hpp"
#include "SoilMoistureSensor.hpp"
#include "TempHumiditySensor.hpp"

namespace pof02 {

inline constexpr std::int64_t kDefaultDatasetStartUnixTime = 1772626200; // 2026-03-04 12:10:00 UTC

class MonitoringSystem {
public:
    MonitoringSystem(SoilMoistureSensor soilSensor,
                     TempHumiditySensor tempHumiditySensor,
                     LightSensor lightSensor,
                     MLLayer mlLayer,
                     RecommendationEngine recommendationEngine,
                     std::size_t historySize = 24,
                     std::int64_t startUnixTime = kDefaultDatasetStartUnixTime);

    bool Init();
    Recommendation RunCycle();
    MonitoringCycleResult RunCycleDetailed();

private:
    SoilMoistureSensor soilSensor_;
    TempHumiditySensor tempHumiditySensor_;
    LightSensor lightSensor_;
    MLLayer mlLayer_;
    RecommendationEngine recommendationEngine_;
    SensorHistory history_;
    std::int64_t startUnixTime_;
    unsigned long startMillis_;
};

} // namespace pof02
