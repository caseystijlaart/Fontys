#pragma once

#include "PlantTypes.hpp"

namespace pof02 {

class RecommendationEngine {
public:
    Recommendation Build(const SensorSnapshot& snapshot, const MLResult& mlResult) const;
};

} // namespace pof02
