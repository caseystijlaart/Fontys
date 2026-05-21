#pragma once

#include "PlantTypes.hpp"

namespace pof02 {

class RecommendationEngine {
public:
    Recommendation Build(const SensorSnapshot& snapshot,
                         const FeatureVector& features,
                         const MLResult& mlResult,
                         const PlantRuleProfile& profile) const;
};

} // namespace pof02
