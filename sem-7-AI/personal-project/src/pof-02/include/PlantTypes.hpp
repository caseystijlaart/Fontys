#pragma once

#include <array>
#include <cstdint>
#include <string>

namespace pof02 {

enum class Level : uint8_t {
    kLow  = 0,
    kOk = 1,
    kHigh = 2,
};

enum class RiskClass : uint8_t {
    HEALTHY = 0,
    MODERATE_STRESS = 1,
    HIGH_STRESS = 2,
};

struct SensorSnapshot {
    float soilMoisturePct = 0.0f;
    float temperatureC = 0.0f;
    float humidityPct = 0.0f;
    float lightLevelPct = 0.0f;
    std::int64_t unixTime = 0;
};

struct FeatureVector {
    static constexpr std::size_t kCount = 12;
    std::array<float, kCount> values{};
};

struct MLResult {
    RiskClass risk = RiskClass::MODERATE_STRESS;
    std::array<float, 3> probabilities{0.0f, 0.0f, 0.0f};
    float confidence = 0.0f;
};

struct Recommendation {
    bool water = false;
    bool reduceTemp = false;
    bool increaseHumidity = false;
    bool increaseLight = false;
    std::string summary;
};

struct MonitoringCycleResult {
    SensorSnapshot snapshot;
    FeatureVector features;
    MLResult mlResult;
    Recommendation recommendation;
};

} // namespace pof02
