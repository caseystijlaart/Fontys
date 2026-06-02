#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <LittleFS.h>

#include "MonitoringSystem.hpp"
#include "WiFiCommunication.hpp"
#include "PlantTypes.hpp"
#include "FileStorageService.hpp"
#include "TimeService.hpp"
#include "Certs.hpp"

using namespace pof02;

namespace
{
    const unsigned long INTERVAL = 1000UL * 60UL * 60UL * 3UL; // 3 hours
    const char *PLANT_SETTINGS_FILE = "/plant_settings.txt";
    const char *LAST_STATE_FILE = "/last_state.txt";
    const char *SENSOR_HISTORY_FILE = "/sensor_history.csv";

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
    constexpr std::size_t kHistorySize = 56; // 1 week at 3h intervals
    MonitoringSystem monitoringSystem(soil, dht, light, ml, recEngine, PlantRuleProfile{}, kHistorySize);

    TimeService timeService;
    FileStorageService fileStorage(timeService);

    WiFiCommunication wifiCommunication(WIFI_SSID, WIFI_PASSWORD);
    MonitoringCycleResult latestResult{};
    bool hasLatestResult = false;

    const char *BASE_URL = "https://yjjpgvsycxlaqubvedoa.supabase.co";
    const char *API_URL = "https://yjjpgvsycxlaqubvedoa.supabase.co/rest/v1/plant_readings";
    String API_URL_SETTING = "https://yjjpgvsycxlaqubvedoa.supabase.co/rest/v1/plant_settings"
                             "?plant_label=eq." +
                             String(PLANT_LABEL) +
                             "&select=*"
                             "&limit=1";
#ifndef API_KEY
#error "API_KEY must be set as a build flag in platformio.ini: -D API_KEY=\"your_key\""
#endif

    uint8_t current_version = 1;

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
    bool ReceiveProfileSettings(String payload)
    {

        if (payload.length() > 0)
        {
            PlantRuleProfile profile = monitoringSystem.GetPlantProfile();
            if (payload.indexOf("\"plant_label\":\"" + String(PLANT_LABEL) + "\"") == -1)
            {
                Serial.println("No settings found for this plant label");
                return false;
            }
            profile.preferences.humidity = payload.indexOf("\"humidityPreference\":\"pLow\"") >= 0 ? PreferenceBand::pLow : (payload.indexOf("\"humidityPreference\":\"pHigh\"") >= 0 ? PreferenceBand::pHigh : PreferenceBand::pMid);
            profile.preferences.light = payload.indexOf("\"lightPreference\":\"pLow\"") >= 0 ? PreferenceBand::pLow : (payload.indexOf("\"lightPreference\":\"pHigh\"") >= 0 ? PreferenceBand::pHigh : PreferenceBand::pMid);
            profile.preferences.soilMoisture = payload.indexOf("\"soilPreference\":\"pLow\"") >= 0 ? PreferenceBand::pLow : (payload.indexOf("\"soilPreference\":\"pHigh\"") >= 0 ? PreferenceBand::pHigh : PreferenceBand::pMid);
            profile.preferences.temperature = payload.indexOf("\"temperaturePreference\":\"pLow\"") >= 0 ? PreferenceBand::pLow : (payload.indexOf("\"temperaturePreference\":\"pHigh\"") >= 0 ? PreferenceBand::pHigh : PreferenceBand::pMid);
            monitoringSystem.SetPlantProfile(profile);
            return true;
        }
        return false;
    }

    bool GetProfileSettings()
    {

        WiFiClientSecure client;
        client.setCACert(kSupabaseRootCA);

        HTTPClient https;

        if (https.begin(client, API_URL_SETTING))
        {
            https.addHeader("apikey", API_KEY);
            https.addHeader("Authorization", String("Bearer ") + API_KEY);
        }

        int httpCode = https.GET();

        if (httpCode > 0)
        {
            String payload = https.getString();

            Serial.println(payload);

            if (payload.indexOf("\"plant_label\":\"" + String(PLANT_LABEL) + "\"") == -1)
            {
                Serial.println("No settings found for this plant label");
                return false;
            }

            // Parse the actual version number from the JSON payload
            int latestVersion = 0;
            const int versionIdx = payload.indexOf("\"version\":");
            if (versionIdx >= 0)
            {
                latestVersion = payload.substring(versionIdx + 10).toInt();
            }

            if (latestVersion != current_version)
            {
                if (ReceiveProfileSettings(payload))
                {
                    current_version = static_cast<uint8_t>(latestVersion);
                    Serial.println("Received updated profile settings");
                    return true;
                }
                else
                {
                    Serial.println("Failed to receive profile settings");
                    return false;
                }
            }
            else
            {
                Serial.println("Profile settings are up to date");
                return true;
            }
        }
        return false;
    }

    void SendToCloud(const String &payload)
    {
        if (WiFi.status() != WL_CONNECTED)
        {
            Serial.println("No WiFi, skipping cloud upload");
            return;
        }

        WiFiClientSecure client;
        client.setCACert(kSupabaseRootCA);

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
        fileStorage.AppendToHistoryFile(SENSOR_HISTORY_FILE, latestResult, PLANT_LABEL, DEVICE_ID, kHistorySize);

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
        payload += "\"recommendation_summary\":\"" + EscapeJsonString(String(latestResult.recommendation.summary.c_str())) + "\",";
        payload += "\"risk_class\":" + String(static_cast<int>(latestResult.mlResult.risk));
        payload += "}";

        SendToCloud(payload);
    }

} // namespace

void setup()
{
    Serial.begin(115200);
    delay(200);

    LittleFS.begin(true);

    if (wifiCommunication.HasCredentials())
    {
        wifiCommunication.Connect();
        timeService.SyncTimeWithNtp(10000);
    }

    const std::int64_t nowUnix = timeService.GetCurrentUnixTimeUtc();
    if (nowUnix > 0)
    {
        monitoringSystem.SetStartUnixTime(nowUnix);
    }

    PlantRuleProfile initialProfile;
    if (!GetProfileSettings())
    {
        Serial.println("Failed to get profile settings, using defaults");
        Serial.println(String(PLANT_LABEL));
    }
    initialProfile.plantName = PLANT_LABEL;
    initialProfile.deviceName = DEVICE_NAME;
    initialProfile.deviceId = String(DEVICE_ID).c_str();
    monitoringSystem.SetPlantProfile(initialProfile);

    std::size_t existingEntries = 0;
    const auto historicalSnapshots = fileStorage.LoadHistoryFile(SENSOR_HISTORY_FILE, kHistorySize, existingEntries);
    if (!historicalSnapshots.empty())
    {
        monitoringSystem.LoadHistoricalSnapshots(historicalSnapshots);
        Serial.printf("Restored %u snapshots (%u total on file)\n", historicalSnapshots.size(), existingEntries);
    }
    else
    {
        Serial.printf("No history file found, starting fresh\n");
    }

    if (!monitoringSystem.Init())
    {
        Serial.println("Failed to initialize monitoring system");
        while (true)
        {
        }
    }

    Serial.println("Monitoring system initialized");
    RunMonitoringCycle();

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

    GetProfileSettings();
    RunMonitoringCycle();
}