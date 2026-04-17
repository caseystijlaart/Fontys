#include "RecommendationEngine.hpp"

#include <sstream>

namespace pof02 {

Recommendation RecommendationEngine::Build(const SensorSnapshot& snapshot, const MLResult& mlResult) const {
    Recommendation rec{};

    rec.water = snapshot.soilMoisturePct < 35.0f || mlResult.risk == RiskClass::HIGH_STRESS;
    rec.reduceTemp = snapshot.temperatureC > 27.0f;
    rec.increaseHumidity = snapshot.humidityPct < 55.0f;
    rec.increaseLight = snapshot.lightLux < 250.0f;

    std::ostringstream oss;
    oss << "risk=" << static_cast<int>(mlResult.risk)
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
