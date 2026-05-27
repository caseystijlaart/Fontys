#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>

#include "MonitoringSystem.hpp"
#include "WiFiCommunication.hpp"

using namespace pof02;

namespace
{
    const unsigned long INTERVAL = 5 * 60 * 1000UL; // 5 minutes
    const char *PLANT_SETTINGS_FILE = "/plant_settings.txt";
    const char *LAST_STATE_FILE = "/last_state.txt";

#ifndef WIFI_SSID
#define WIFI_SSID ""
#endif

#ifndef WIFI_PASSWORD
#define WIFI_PASSWORD ""
#endif

#ifndef PLANT_LABEL
#define PLANT_LABEL "UNSET_PLANT"
#endif

#ifndef DEVICE_NAME
#define DEVICE_NAME "UNSET_DEVICE"
#endif

#ifndef DEVICE_ID
#define DEVICE_ID 0
#endif

#ifndef ENABLE_WIFI_DOWNLOAD
#define ENABLE_WIFI_DOWNLOAD 1
#endif

    // Calibrated for a chunkier mix (~50% soil / 25% perlite / 25% bark)
    // that retains moisture differently than dense potting soil.
    SoilMoistureSensor soil(34, 3500.0f, 1450.0f);
    TempHumiditySensor dht(4, 22);
    LightSensor light(35);
    MLLayer ml(MLBackend::TINYML_TFLM);
    RecommendationEngine recEngine;
    MonitoringSystem monitoringSystem(soil, dht, light, ml, recEngine, PlantRuleProfile{}, 24);

    WiFiCommunication wifiCommunication(WIFI_SSID, WIFI_PASSWORD);
    MonitoringCycleResult latestResult{};
    bool hasLatestResult = false;

    const char *API_URL = "https://yjjpgvsycxlaqubvedoa.supabase.co/rest/v1/plant_readings";
    const char* API_KEY = "sb_publishable_gbOIlHncD9H3VKJLcXKoUw_hJZ7J_-5";
    unsigned long lastRun = 0;

    String EscapeJsonString(const String &input)
    {
        String output = input;
        output.replace("\\", "\\\\");
        output.replace("\"", "\\\"");
        output.replace("\n", "\\n");
        output.replace("\r", "\\r");
        return output;
    }

    void SendToCloud(const String &payload)
    {
        if (WiFi.status() != WL_CONNECTED)
        {
            Serial.println("No WiFi, skipping cloud upload");
            return;
        }

        WiFiClientSecure client;
        client.setInsecure(); // ok for now (we can harden later)

        HTTPClient https;

        if (https.begin(client, API_URL))
        {
            https.addHeader("Content-Type", "application/json");
            https.addHeader("apikey", API_KEY);
            https.addHeader("Authorization", "Bearer " + String(API_KEY));
            https.addHeader("Prefer", "return=minimal");
            int httpCode = https.POST(payload);

            Serial.print("Cloud response code: ");
            Serial.println(httpCode);

            String response = https.getString();
            Serial.println(response);

            https.end();
        }
        else
        {
            Serial.println("HTTPS begin failed");
        }
    }

    void RunMonitoringCycle()
    {
        latestResult = monitoringSystem.RunCycleDetailed();
        hasLatestResult = true;

        String payload = "{";
        payload += "\"request_id\":\"" + String(millis()) + "\",";
        payload += "\"plant_label\":\"" + EscapeJsonString(String(PLANT_LABEL)) + "\",";
        payload += "\"device_id\":" + String(DEVICE_ID) + ",";
        payload += "\"soil_moisture_pct\":" + String(latestResult.snapshot.soilMoisturePct, 2) + ",";
        payload += "\"temperature_c\":" + String(latestResult.snapshot.temperatureC, 2) + ",";
        payload += "\"humidity_pct\":" + String(latestResult.snapshot.humidityPct, 2) + ",";
        payload += "\"light_level_pct\":" + String(latestResult.snapshot.lightLevelPct, 2) + ",";
        payload += "\"action_water\":" + String(latestResult.recommendation.water ? "true" : "false") + ",";
        payload += "\"action_reduce_temp\":" + String(latestResult.recommendation.reduceTemp ? "true" : "false") + ",";
        payload += "\"action_increase_light\":" + String(latestResult.recommendation.increaseLight ? "true" : "false") + ",";
        payload += "\"recommendation_summary\":\"" + EscapeJsonString(String(latestResult.recommendation.summary.c_str())) + "\"";
        payload += "}";

        SendToCloud(payload);
    }

} // namespace

void setup()
{
    Serial.begin(115200);
    delay(200);

    if (wifiCommunication.HasCredentials())
    {
        wifiCommunication.Connect();
    }

    PlantRuleProfile initialProfile;
    initialProfile.plantName = PLANT_LABEL;
    initialProfile.deviceName = DEVICE_NAME;
    initialProfile.deviceId = String(DEVICE_ID).c_str();
    monitoringSystem.SetPlantProfile(initialProfile);

    if (!monitoringSystem.Init())
    {
        Serial.println("Failed to initialize monitoring system");
        while (true)
        {
        }
    }

    Serial.println("Monitoring system initialized");

    lastRun = millis();
}

void loop()
{

    const unsigned long now = millis();
    if (now - lastRun < INTERVAL)
    {
        return;
    }

    lastRun = now;

    if (wifiCommunication.HasCredentials() && !wifiCommunication.IsConnected())
    {
        wifiCommunication.Connect();
    }

    RunMonitoringCycle();
}