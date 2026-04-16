#pragma once

/** @brief Enum to represent plant health status
 * This enum defines the possible health states of a plant based on sensor data and configuration thresholds.
 * It includes three states: HEALTHY, MODERATE_STRESS, and HIGH_STRESS.
 */
enum class PlantHealth
{
    HEALTHY,
    MODERATE_STRESS,
    HIGH_STRESS
};