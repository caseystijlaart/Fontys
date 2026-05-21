#include "Classification.hpp"

namespace pof02 {

Level Classification::Classify(float value, const ClassificationThresholds& thresholds) {
    if (value < thresholds.low) {
        return Level::kLow ;
    }
    if (value > thresholds.high) {
        return Level::kHigh;
    }
    return Level::kOk;
}

} // namespace pof02
