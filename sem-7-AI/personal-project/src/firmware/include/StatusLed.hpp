/**
 * @file StatusLed.hpp
 * @brief Common-cathode RGB status LED reflecting the plant health risk class.
 * @version 1.1.0
 * @date 2026-06-12
 * @author C. Stijlaart
 * @copyright Copyright (c) 2026 C. Stijlaart. Released under the MIT License.
 */
#pragma once

#include "PlantTypes.hpp"

/**
 * @brief Drives a common-cathode RGB LED to indicate plant health at a glance.
 *
 * Common cathode: the shared pin goes to GND and each colour pin is driven HIGH
 * to light that channel. Colour mapping:
 *  - HEALTHY        → green
 *  - MODERATE_STRESS→ yellow (red + green)
 *  - HIGH_STRESS    → red
 */
class StatusLed
{
public:
    /**
     * @brief Constructs a StatusLed instance.
     * @param redPin GPIO connected to the red channel.
     * @param greenPin GPIO connected to the green channel.
     * @param bluePin GPIO connected to the blue channel.
     */
    StatusLed(int redPin, int greenPin, int bluePin);

    /**
     * @brief Configures the GPIO pins as outputs and turns the LED off.
     */
    void Begin() const;

    /**
     * @brief Updates the LED colour to match the given health risk class.
     * @param risk The plant health risk class.
     */
    void ShowRisk(RiskClass risk) const;

private:
    /**
     * @brief Writes the individual channel states (true = on) to the GPIO pins.
     */
    void Write(bool red, bool green, bool blue) const;

    int redPin_;
    int greenPin_;
    int bluePin_;
};
