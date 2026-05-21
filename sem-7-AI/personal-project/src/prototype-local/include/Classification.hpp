#pragma once

#include "PlantTypes.hpp"

namespace pof02 {

struct ClassificationThresholds {
    float low;
    float high;
};

class Classification {
public:
    static Level Classify(float value, const ClassificationThresholds& thresholds);
};

} // namespace pof02
