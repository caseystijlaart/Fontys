#include "RecommendationEngine.hpp"

#include <sstream>

namespace pof02 {

namespace {

float Clamp(const float value, const float minVal, const float maxVal) {
    return value < minVal ? minVal : (value > maxVal ? maxVal : value);
}

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

const char* BandToText(const PreferenceBand band) {
    switch (band) {
        case PreferenceBand::pLow:
            return "pLow";
        case PreferenceBand::pMid:
            return "pMid";
        case PreferenceBand::pHigh:
            return "pHigh";
        default:
            return "pMid";
    }
}

} // namespace

Recommendation RecommendationEngine::Build(const SensorSnapshot& snapshot, const FeatureVector& features, const MLResult& mlResult, const PlantRuleProfile& profile) const {
    Recommendation rec{};

    const bool soilAlreadyBelowPreference = IsBelowPreference(snapshot.soilMoisturePct, profile.soilMoistureThresholds, profile.preferences.soilMoisture);

    // Feature engineering outputs:
    // values[2] = moistureDelta per cycle, values[3] = slope per cycle.
    // Sampling cycle is 2 hours in this project.
    const float avgDryingPerCycle = (-features.values[2] - features.values[3]) * 0.5f;
    const float baseDryingPerHour = avgDryingPerCycle > 0.0f ? (avgDryingPerCycle / 2.0f) : 0.0f;

    // Lightweight ML regression model for evaporation pressure.
    // Inputs: normalized temperature, light, inverse humidity, and soil trend.
    const float tempNorm = Clamp((snapshot.temperatureC - 10.0f) / 25.0f, 0.0f, 2.0f);
    const float lightNorm = Clamp(snapshot.lightLevelPct / 100.0f, 0.0f, 1.5f);
    const float dryAirNorm = Clamp((60.0f - snapshot.humidityPct) / 60.0f, 0.0f, 1.5f);
    const float trendNorm = Clamp(avgDryingPerCycle / 4.0f, 0.0f, 2.0f);

    // Linear model (offline-tunable coefficients).
    const float evapScore = 0.55f +
                            0.28f * tempNorm +
                            0.24f * lightNorm +
                            0.22f * dryAirNorm +
                            0.30f * trendNorm;
    const float evapMultiplier = Clamp(evapScore, 0.35f, 2.10f);
    const float effectiveDryingPerHour = baseDryingPerHour * evapMultiplier;

    const float soilTarget = profile.soilMoistureThresholds.midMin;
    float predictedHoursToTarget = -1.0f;
    if (snapshot.soilMoisturePct > soilTarget && effectiveDryingPerHour > 0.01f)
    {
        predictedHoursToTarget = (snapshot.soilMoisturePct - soilTarget) / effectiveDryingPerHour;
    }

    const bool predictedDryWithinTwoDays = predictedHoursToTarget > 0.0f && predictedHoursToTarget <= 48.0f;
    rec.water = soilAlreadyBelowPreference || predictedDryWithinTwoDays || mlResult.risk == RiskClass::HIGH_STRESS;
    rec.reduceTemp = IsAbovePreference(snapshot.temperatureC, profile.temperatureThresholds, profile.preferences.temperature);
    rec.increaseHumidity = IsBelowPreference(snapshot.humidityPct, profile.humidityThresholds, profile.preferences.humidity);
    rec.increaseLight = IsBelowPreference(snapshot.lightLevelPct, profile.lightThresholds, profile.preferences.light);

    std::ostringstream oss;
    oss << "plant=" << profile.plantName
        << " prefs=[soil:" << BandToText(profile.preferences.soilMoisture)
        << ",temp:" << BandToText(profile.preferences.temperature)
        << ",humidity:" << BandToText(profile.preferences.humidity)
        << ",light:" << BandToText(profile.preferences.light)
        << "] risk=" << static_cast<int>(mlResult.risk)
        << " confidence=" << mlResult.confidence
        << " evapML=" << evapMultiplier
        << " dryRateHr=" << effectiveDryingPerHour
        << " hoursToSoilTarget=" << predictedHoursToTarget
        << " actions:["
        << (rec.water ? "water " : "")
        << (rec.reduceTemp ? "cool " : "")
        << (rec.increaseHumidity ? "humidify " : "")
        << (rec.increaseLight ? "light " : "")
        << "]";
    rec.summary = oss.str();

    return rec;
}

} // namespace pof02
