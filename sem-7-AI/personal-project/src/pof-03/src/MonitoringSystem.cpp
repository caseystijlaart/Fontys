#include "MonitoringSystem.hpp"

#include <Arduino.h>

#include "FeatureEngineering.hpp"

namespace pof02 {

MonitoringSystem::MonitoringSystem(SoilMoistureSensor soilSensor,
                                   TempHumiditySensor tempHumiditySensor,
                                   LightSensor lightSensor,
                                   MLLayer mlLayer,
                                   RecommendationEngine recommendationEngine,
                                   PlantRuleProfile plantProfile,
                                   std::size_t historySize,
                                   std::int64_t startUnixTime)
    : soilSensor_(soilSensor),
      tempHumiditySensor_(tempHumiditySensor),
      lightSensor_(lightSensor),
      mlLayer_(mlLayer),
      recommendationEngine_(recommendationEngine),
      plantProfile_(plantProfile),
      history_(historySize),
      startUnixTime_(startUnixTime),
      startMillis_(0) {}

bool MonitoringSystem::Init() {
    startMillis_ = millis();
    return tempHumiditySensor_.Init();
}

Recommendation MonitoringSystem::RunCycle() {
    return RunCycleDetailed().recommendation;
}

MonitoringCycleResult MonitoringSystem::RunCycleDetailed() {
    const auto [temperature, humidity] = tempHumiditySensor_.Read();

    SensorSnapshot snapshot{};
    snapshot.soilMoisturePct = soilSensor_.ReadPercent();
    snapshot.temperatureC = temperature;
    snapshot.humidityPct = humidity;
    snapshot.lightLevelPct = lightSensor_.ReadLightLevelPct();

    const unsigned long elapsedMs = millis() - startMillis_;
    snapshot.unixTime = startUnixTime_ + static_cast<std::int64_t>(elapsedMs / 1000UL);

    history_.Add(snapshot);

    const FeatureVector features = FeatureEngineering::Build(history_);
    const MLResult mlResult = mlLayer_.Predict(features);
    const Recommendation recommendation = recommendationEngine_.Build(snapshot, mlResult, plantProfile_);

    return MonitoringCycleResult{snapshot, features, mlResult, recommendation};
}

void MonitoringSystem::SetPlantProfile(const PlantRuleProfile& plantProfile) {
    plantProfile_ = plantProfile;
}

const PlantRuleProfile& MonitoringSystem::GetPlantProfile() const {
    return plantProfile_;
}

} // namespace pof02
