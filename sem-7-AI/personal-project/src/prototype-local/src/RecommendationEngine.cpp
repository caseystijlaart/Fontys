#include "RecommendationEngine.hpp"

#include <algorithm>
#include <sstream>

namespace pof02 {

namespace {

bool IsBelowPreference(const float value, const MetricThresholds& thresholds, const PreferenceBand preference) {
    switch (preference) {
        case PreferenceBand::pLow:
            return value > thresholds.lowMax;
        case PreferenceBand::pMid:
            return value < thresholds.midMin;
        case PreferenceBand::pHigh:
            return value < thresholds.highMin;
        default:
            return false;
    }
}

bool IsAbovePreference(const float value, const MetricThresholds& thresholds, const PreferenceBand preference) {
    switch (preference) {
        case PreferenceBand::pLow:
            return false;
        case PreferenceBand::pMid:
            return value > thresholds.midMax;
        case PreferenceBand::pHigh:
            return false;
        default:
            return false;
    }
}

} // namespace

Recommendation RecommendationEngine::Build(const SensorSnapshot& snapshot,
                                           const MLResult& mlResult,
                                           const PlantRuleProfile& profile,
                                           const float predictedMinutesToWater) const {
    Recommendation rec{};

    rec.predictedMinutesToWater = predictedMinutesToWater;
    rec.water = predictedMinutesToWater <= 30.0f
                || IsBelowPreference(snapshot.soilMoisturePct, profile.soilMoistureThresholds, profile.preferences.soilMoisture)
                || mlResult.risk == RiskClass::HIGH_STRESS;
    rec.reduceTemp = IsAbovePreference(snapshot.temperatureC, profile.temperatureThresholds, profile.preferences.temperature);
    rec.increaseLight = IsBelowPreference(snapshot.lightLevelPct, profile.lightThresholds, profile.preferences.light);

    rec.baselineCycleMinutes = 30.0f;
    rec.optimizedCycleMinutes = std::clamp(predictedMinutesToWater * 0.5f, 15.0f, 240.0f);
    rec.estimatedPowerSavingPct = (1.0f - (rec.baselineCycleMinutes / rec.optimizedCycleMinutes)) * 100.0f;
    rec.estimatedPowerSavingPct = std::clamp(rec.estimatedPowerSavingPct, 0.0f, 95.0f);

    std::ostringstream oss;
    oss << "predWaterMin=" << rec.predictedMinutesToWater
        << " nextCycleMin=" << rec.optimizedCycleMinutes
        << " powerSavingPct=" << rec.estimatedPowerSavingPct
        << " actions:["
        << (rec.water ? "water " : "")
        << (rec.reduceTemp ? "cool " : "")
        << (rec.increaseLight ? "light " : "")
        << "]";
    rec.summary = oss.str();

    return rec;
}

} // namespace pof02
