#pragma once

#include <Arduino.h>

/** @brief Struct to hold plant configuration thresholds
 * This struct encapsulates the various thresholds used for classifying plant health.
 * It includes thresholds for soil moisture, temperature, humidity, and light.
 */
struct PlantConfig
{
    float moistureThreshold = 60.0f;
    float temperatureThreshold = 25.0f;
    float humidityThreshold = 60.0f;
    float lightThreshold = 245.0f;
};

class PlantConfiguration
{
public:
    /** @brief Default constructor */
    PlantConfiguration();

    /** @brief Parameterized constructor
     * @param moistureThreshold The soil moisture threshold for plant health classification
     * @param temperatureThreshold The temperature threshold for plant health classification
     * @param humidityThreshold The humidity threshold for plant health classification
     * @param lightThreshold The light threshold for plant health classification
     */
    PlantConfiguration(float moistureThreshold, float temperatureThreshold, float humidityThreshold, float lightThreshold);

    /** @brief Get the plant configuration
     * @return const reference to the PlantConfig struct
     * This allows read-only access to the configuration values while keeping the internal struct encapsulated.
     */
    const PlantConfig &getConfig() const;

    /** @brief Set the soil moisture threshold
     * @param threshold The new soil moisture threshold (0-100)
     * @return true if the threshold was set successfully, false if the value is out of range
     */
    bool SetMoistureThreshold(float threshold);

    /** @brief Set the temperature threshold
     * @param threshold The new temperature threshold (0-100)
     * @return true if the threshold was set successfully, false if the value is out of range
     */
    bool SetTemperatureThreshold(float threshold);

    /** @brief Set the humidity threshold
     * @param threshold The new humidity threshold (0-100)
     * @return true if the threshold was set successfully, false if the value is out of range
     */
    bool SetHumidityThreshold(float threshold);

    /** @brief Set the light threshold
     * @param threshold The new light threshold (0-100)
     * @return true if the threshold was set successfully, false if the value is out of range
     */
    bool SetLightThreshold(float threshold);

    /** @brief Get the humidity threshold
     * @return The current humidity threshold value
     */
    float GetHumidityThreshold() const;

    /** @brief Get the soil moisture threshold
     * @return The current soil moisture threshold value
     */
    float GetMoistureThreshold() const;

    /** @brief Get the temperature threshold
     * @return The current temperature threshold value
     */
    float GetTemperatureThreshold() const;

    /** @brief Get the light threshold
     * @return The current light threshold value
     */
    float GetLightThreshold() const;

private:
    /** @brief The internal plant configuration struct */
    PlantConfig config;
};
