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

void Prediction::Forward(const float input[3], float output[3])
{
    const float W1[3][8] = {
        {-1.66090395f, 1.28863049f, 1.23965745f, 0.25400533f, -1.21944919f, -2.50832765f, -1.63462578f, 0.85975954f},
        {-0.02208931f, -0.01401896f, -0.13590374f, 0.57209713f, 0.82869254f, -0.05283282f, 0.00269596f, -0.86147397f},
        {-0.05387287f, -0.23540778f, -0.04325007f, -0.29359517f, -0.43342775f, -0.30334154f, -0.05451270f, -0.18861791f}};

    const float b1[8] = {
        1.13610458f, 0.51206488f, 0.17462122f, 0.93053437f,
        0.41859057f, -1.08522127f, 1.14938218f, 0.41616960f};

    float hidden[8];

    for (int i = 0; i < 8; i++)
    {
        hidden[i] = b1[i];

        for (int j = 0; j < 3; j++)
        {
            hidden[i] += input[j] * W1[j][i];
        }

        hidden[i] = Relu(hidden[i]);
    }

    const float W2[8][3] = {
        {-3.02247220f, 1.30793735f, 0.88963572f},
        {0.77690098f, 0.06565555f, -1.09702521f},
        {-0.30470337f, 0.70747872f, -0.61820458f},
        {0.24103667f, -1.19770835f, 0.89838840f},
        {-1.61411744f, 0.86172138f, -0.21602353f},
        {-0.32693233f, 1.73635854f, -2.54433441f},
        {-0.55514585f, 0.82761545f, 1.15932432f},
        {0.31188915f, -0.15159676f, 1.16560239f}};

    const float b2[3] = {
        -0.58318004f, -1.11053463f, -0.14287843f};

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
    float humidityStress = fabs(data.humidity - config.getConfig().humidityThreshold);
    float moistureTempInteraction = data.soilMoisture * data.temperature;

    const float mean[3] = {
        25.04231369f,
        8.39165835f,
        600.06397933f};

    const float stdDev[3] = {
        8.77016247f,
        5.48159131f,
        225.02376219f};

    float input[3];
    input[0] = (soilMoisture - mean[0]) / stdDev[0];
    input[1] = (humidityStress - mean[1]) / stdDev[1];
    input[2] = (moistureTempInteraction - mean[2]) / stdDev[2];

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