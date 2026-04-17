#include "MonitoringSystem.hpp"

#include <ctime>

#include "FeatureEngineering.hpp"

namespace pof02 {

MonitoringSystem::MonitoringSystem(SoilMoistureSensor soilSensor,
                                   TempHumiditySensor tempHumiditySensor,
                                   LightSensor lightSensor,
                                   MLLayer mlLayer,
                                   RecommendationEngine recommendationEngine,
                                   std::size_t historySize)
    : soilSensor_(soilSensor),
      tempHumiditySensor_(tempHumiditySensor),
      lightSensor_(lightSensor),
      mlLayer_(mlLayer),
      recommendationEngine_(recommendationEngine),
      history_(historySize) {}

bool MonitoringSystem::Init() { return tempHumiditySensor_.Init(); }

Recommendation MonitoringSystem::RunCycle() {
    const auto [temperature, humidity] = tempHumiditySensor_.Read();

    SensorSnapshot snapshot{};
    snapshot.soilMoisturePct = soilSensor_.ReadPercent();
    snapshot.temperatureC = temperature;
    snapshot.humidityPct = humidity;
    snapshot.lightLux = lightSensor_.ReadLux();
    snapshot.unixTime = static_cast<std::int64_t>(std::time(nullptr));

    history_.Add(snapshot);

    const FeatureVector features = FeatureEngineering::Build(history_);
    const MLResult mlResult = mlLayer_.Predict(features);

    return recommendationEngine_.Build(snapshot, mlResult);
}

} // namespace pof02
