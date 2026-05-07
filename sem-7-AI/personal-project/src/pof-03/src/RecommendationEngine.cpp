#include "RecommendationEngine.hpp"

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

Recommendation RecommendationEngine::Build(const SensorSnapshot& snapshot, const MLResult& mlResult, const PlantRuleProfile& profile) const {
    Recommendation rec{};

    rec.water = IsBelowPreference(snapshot.soilMoisturePct, profile.soilMoistureThresholds, profile.preferences.soilMoisture)
                || mlResult.risk == RiskClass::HIGH_STRESS;
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
