#pragma once

#include <cstddef>

#include "PlantTypes.hpp"
#include "SensorHistory.hpp"

namespace pof02 {

class PredictiveIrrigationModel {
public:
    void Update(const SensorHistory& history);

    float PredictMinutesUntilWatering(const SensorSnapshot& snapshot, const PlantRuleProfile& profile) const;

private:
    bool trained_ = false;
    float soilIntercept_ = 100.0f;
    float soilRatePerMinute_ = -0.3f;

    static float ClampRate(float rate);
    static float MinutesToCrossThreshold(float currentValue, float threshold, float ratePerMinute);
};

} // namespace pof02
