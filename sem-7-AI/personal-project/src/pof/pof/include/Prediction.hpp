#pragma once

#include "SensorData.hpp"
#include "PlantConfiguration.hpp"
#include "PlantHealth.hpp"

class Prediction
{
public:
    /** @brief Initialize the prediction model
     * @return true if initialization was successful, false otherwise
     */
    bool Init();

    /** @brief Predict the plant health status based on sensor data and configuration
     * @param data The sensor data to be used for prediction
     * @param config The plant configuration thresholds to be used for classification
     * @return The predicted plant health status (HEALTHY, MODERATE_STRESS, or HIGH_STRESS)
     */
    PlantHealth Predict(const SensorData &data, const PlantConfiguration &config);

private:
    /** @brief Apply the ReLU activation function
     * @param x The input value
     * @return The output value after applying ReLU
     */
    float Relu(float x);

    /** @brief Get the index of the maximum value in an array
     * @param values The array of values to be evaluated
     * @param size The size of the array
     * @return The index of the maximum value in the array
     */
    int ArgMax(const float values[], int size);

    /** @brief Perform a forward pass through the neural network
     * @param input The input values
     * @param output The output values
     */
    void Forward(const float input[3], float output[3]);
};