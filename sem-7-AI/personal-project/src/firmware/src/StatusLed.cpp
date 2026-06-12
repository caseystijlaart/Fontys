/**
 * @file StatusLed.cpp
 * @brief Implementation of StatusLed. Common-cathode RGB status LED driver.
 * @version 1.1.0
 * @date 2026-06-12
 * @author C. Stijlaart
 * @copyright Copyright (c) 2026 C. Stijlaart. Released under the MIT License.
 */
#include "StatusLed.hpp"

#include <Arduino.h>

StatusLed::StatusLed(int redPin, int greenPin, int bluePin)
    : redPin_(redPin), greenPin_(greenPin), bluePin_(bluePin) {}

void StatusLed::Begin() const
{
    pinMode(redPin_, OUTPUT);
    pinMode(greenPin_, OUTPUT);
    pinMode(bluePin_, OUTPUT);
    Write(false, false, false);
}

void StatusLed::ShowRisk(RiskClass risk) const
{
    switch (risk)
    {
    case RiskClass::HEALTHY:
        Write(false, true, false); // green
        break;
    case RiskClass::MODERATE_STRESS:
        Write(true, true, false); // yellow
        break;
    case RiskClass::HIGH_STRESS:
        Write(true, false, false); // red
        break;
    }
}

void StatusLed::Write(bool red, bool green, bool blue) const
{
    // Common cathode: HIGH lights the channel.
    digitalWrite(redPin_, red ? HIGH : LOW);
    digitalWrite(greenPin_, green ? HIGH : LOW);
    digitalWrite(bluePin_, blue ? HIGH : LOW);
}
