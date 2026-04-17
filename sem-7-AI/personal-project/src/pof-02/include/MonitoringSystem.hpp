#pragma once

#include "LightSensor.hpp"
#include "MLLayer.hpp"
#include "RecommendationEngine.hpp"
#include "SensorHistory.hpp"
#include "SoilMoistureSensor.hpp"
#include "TempHumiditySensor.hpp"

namespace pof02 {

class MonitoringSystem {
public:
    MonitoringSystem(SoilMoistureSensor soilSensor,
                     TempHumiditySensor tempHumiditySensor,
                     LightSensor lightSensor,
                     MLLayer mlLayer,
                     RecommendationEngine recommendationEngine,
                     std::size_t historySize = 24);

    bool Init();
    Recommendation RunCycle();

private:
    SoilMoistureSensor soilSensor_;
    TempHumiditySensor tempHumiditySensor_;
    LightSensor lightSensor_;
    MLLayer mlLayer_;
    RecommendationEngine recommendationEngine_;
    SensorHistory history_;
};

} // namespace pof02
