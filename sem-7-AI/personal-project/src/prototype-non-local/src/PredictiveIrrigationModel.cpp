#include "PredictiveIrrigationModel.hpp"

#include <algorithm>

namespace pof02 {

namespace {

float ComputeSlope(const float* x, const float* y, const std::size_t n) {
    if (n < 2) {
        return 0.0f;
    }

    float meanX = 0.0f;
    float meanY = 0.0f;
    for (std::size_t i = 0; i < n; ++i) {
        meanX += x[i];
        meanY += y[i];
    }
    meanX /= static_cast<float>(n);
    meanY /= static_cast<float>(n);

    float cov = 0.0f;
    float var = 0.0f;
    for (std::size_t i = 0; i < n; ++i) {
        const float dx = x[i] - meanX;
        cov += dx * (y[i] - meanY);
        var += dx * dx;
    }

    return var <= 1e-4f ? 0.0f : cov / var;
}

} // namespace

void PredictiveIrrigationModel::Update(const SensorHistory& history) {
    const std::size_t n = history.Size();
    if (n < 4) {
        return;
    }

    float minutes[64]{};
    float soil[64]{};
    const std::size_t capN = std::min<std::size_t>(n, 64);
    const auto& latest = history.Latest();

    for (std::size_t i = 0; i < capN; ++i) {
        const auto& sample = history.Data().at(n - capN + i);
        minutes[i] = static_cast<float>(sample.unixTime - latest.unixTime) / 60.0f;
        soil[i] = sample.soilMoisturePct;
    }

    soilRatePerMinute_ = ClampRate(ComputeSlope(minutes, soil, capN));
    soilIntercept_ = latest.soilMoisturePct;
    trained_ = true;
}

float PredictiveIrrigationModel::PredictMinutesUntilWatering(const SensorSnapshot& snapshot, const PlantRuleProfile& profile) const {
    const float threshold = profile.soilMoistureThresholds.midMin;
    const float rate = trained_ ? soilRatePerMinute_ : -0.02f;
    return MinutesToCrossThreshold(snapshot.soilMoisturePct, threshold, rate);
}

float PredictiveIrrigationModel::ClampRate(float rate) {
    return std::clamp(rate, -0.5f, 0.05f);
}

float PredictiveIrrigationModel::MinutesToCrossThreshold(float currentValue, float threshold, float ratePerMinute) {
    if (currentValue <= threshold) {
        return 0.0f;
    }

    if (ratePerMinute >= 0.0f) {
        return 7.0f * 24.0f * 60.0f;
    }

    const float minutes = (threshold - currentValue) / ratePerMinute;
    return std::clamp(minutes, 0.0f, 7.0f * 24.0f * 60.0f);
}

} // namespace pof02
