#include "Classification.hpp"

namespace pof02 {

Level Classification::Classify(float value, const ClassificationThresholds& thresholds) {
    if (value < thresholds.low) {
        return Level::LOW;
    }
    if (value > thresholds.high) {
        return Level::HIGH;
    }
    return Level::OK;
}

} // namespace pof02
