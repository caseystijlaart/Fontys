#include <math.h>
#include "Prediction.hpp"

bool Prediction::Init()
{
    return true;
}

float Prediction::Relu(float x)
{
    return x > 0.0f ? x : 0.0f;
}

int Prediction::ArgMax(const float values[], int size)
{
    int maxIndex = 0;

    for (int i = 1; i < size; i++)
    {
        if (values[i] > values[maxIndex])
        {
            maxIndex = i;
        }
    }

    return maxIndex;
}

void Prediction::Softmax(const float input[3], float output[3])
{
    float maxVal = input[0];
    for (int i = 1; i < 3; i++)
    {
        if (input[i] > maxVal)
        {
            maxVal = input[i];
        }
    }

    float sum = 0.0f;
    for (int i = 0; i < 3; i++)
    {
        output[i] = expf(input[i] - maxVal);
        sum += output[i];
    }

    if (sum > 0.0f)
    {
        for (int i = 0; i < 3; i++)
        {
            output[i] /= sum;
        }
    }
}

void Prediction::Forward(const float input[4], float output[3])
{
    const float W1[4][8] = {
        {-1.79645792e-29f, 7.36498821e-01f, -9.18600517e-02f, -2.10526377e-01f, -1.72966167e+00f, -3.01354694e-01f, -1.48001689e+00f, -3.95681758e-01f},
        {-1.35178853e-36f, 4.35073876e-01f, -2.91407781e-01f, 7.96449588e-01f, 9.78928099e-02f, -4.32019975e-01f, 2.23871442e-02f, -1.08806332e-01f},
        {-3.34885520e-17f, -5.12598063e-01f, 7.11559822e-01f, 1.64137753e-01f, 1.47223939e+00f, -8.45731228e-01f, 4.47049556e-01f, 8.05428686e-01f},
        {-2.90912615e-53f, -1.70884586e-01f, -1.53713027e-01f, -1.79259987e-01f, -1.72184163e-01f, -6.43409964e-01f, -5.11267071e-02f, -3.82988747e-01f}};

    const float b1[8] = {
        -0.61510994f, 0.98456392f, 1.34801918f, 0.62498837f,
        -0.62260429f, -0.53317746f, 1.48285706f, -0.62640286f};

    float hidden[8];

    for (int i = 0; i < 8; i++)
    {
        hidden[i] = b1[i];

        for (int j = 0; j < 4; j++)
        {
            hidden[i] += input[j] * W1[j][i];
        }

        hidden[i] = Relu(hidden[i]);
    }

    const float W2[8][3] = {
        {-2.38286671e-07f, 2.33239966e-61f, -1.26438362e-05f},
        {9.20674149e-01f, -3.07979130e-01f, -3.83742656e-01f},
        {-7.67074403e-01f, -9.62592381e-02f, 5.60254877e-01f},
        {-5.11815437e-01f, 3.25727921e-01f, 9.33384644e-01f},
        {2.81582279e-01f, 1.77559772e+00f, -1.34934653e+00f},
        {9.95476323e-01f, -1.09633088e+00f, -2.56174251e-01f},
        {-2.03751764e+00f, 8.73951817e-02f, 8.75917921e-01f},
        {-2.89642740e-01f, 1.10247179e+00f, -1.15883387e+00f}};

    const float b2[3] = {
        -0.58697311f, -0.24807985f, -0.09958299f};

    for (int i = 0; i < 3; i++)
    {
        output[i] = b2[i];

        for (int j = 0; j < 8; j++)
        {
            output[i] += hidden[j] * W2[j][i];
        }
    }
}

PredictionResult Prediction::Predict(const SensorData &data, const PlantConfiguration &config)
{
    float soilMoisture = data.soilMoisture;

    // Assumes these config values represent ideal values / target thresholds
    float humidityStress = fabs(data.humidity - config.getConfig().humidityThreshold);
    float moistureStress = fabs(data.soilMoisture - config.getConfig().moistureThreshold);
    float moistureTempInteraction = data.soilMoisture * data.temperature;

    const float mean[4] = {
        25.04231369f,
        8.39165835f,
        10.88692942f,
        600.06397933f};

    const float stdDev[4] = {
        8.77016247f,
        5.48159131f,
        7.58591024f,
        225.02376219f};

    float input[4];
    input[0] = (soilMoisture - mean[0]) / stdDev[0];
    input[1] = (humidityStress - mean[1]) / stdDev[1];
    input[2] = (moistureStress - mean[2]) / stdDev[2];
    input[3] = (moistureTempInteraction - mean[3]) / stdDev[3];

    float logits[3];
    Forward(input, logits);

    float probabilities[3];
    Softmax(logits, probabilities);

    int predictedClass = ArgMax(probabilities, 3);

    PredictionResult result;
    result.confidence = probabilities[predictedClass];
    result.probabilities[0] = probabilities[0];
    result.probabilities[1] = probabilities[1];
    result.probabilities[2] = probabilities[2];

    switch (predictedClass)
    {
    case 0:
        result.health = PlantHealth::HEALTHY;
        break;
    case 1:
        result.health = PlantHealth::HIGH_STRESS;
        break;
    case 2:
        result.health = PlantHealth::MODERATE_STRESS;
        break;
    default:
        result.health = PlantHealth::MODERATE_STRESS;
        break;
    }

    return result;
}