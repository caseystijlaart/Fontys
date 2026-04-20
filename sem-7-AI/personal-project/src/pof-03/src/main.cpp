#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <LittleFS.h>
#include <FS.h>

#include <ctime>

#include "MonitoringSystem.hpp"

using namespace pof02;

SoilMoistureSensor soil(34, 3200.0f, 1200.0f);
TempHumiditySensor dht(4, 22);
LightSensor light(35);

MLLayer ml(MLBackend::TINYML_TFLM);
RecommendationEngine recEngine;
MonitoringSystem monitoringSystem(soil, dht, light, ml, recEngine, PlantRuleProfile{}, 24);

const unsigned long INTERVAL = 7200000UL; // 2 hours in milliseconds
unsigned long lastRun = 0;
unsigned long logCount = 0;
bool timeSynced = false;

const char *PLANT_SETTINGS_FILE = "/plant_settings.txt";
const char *LAST_STATE_FILE = "/last_state.txt";

WebServer server(80);

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

std::int64_t GetCurrentUnixTimeUtc()
{
    time_t now;
    time(&now);

    if (now < 1700000000)
    {
        return 0;
    }

    return static_cast<std::int64_t>(now);
}

String FormatTimestampLocal(std::int64_t unixTime)
{
    const std::time_t ts = static_cast<std::time_t>(unixTime);
    std::tm timeInfo{};
    localtime_r(&ts, &timeInfo);

    char buffer[24];
    const std::size_t len = std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &timeInfo);
    if (len == 0)
    {
        return "1970-01-01 00:00:00";
    }

    return String(buffer);
}

String PreferenceToString(PreferenceBand band)
{
    switch (band)
    {
    case PreferenceBand::pLow:
        return "pLow";
    case PreferenceBand::pHigh:
        return "pHigh";
    case PreferenceBand::pMid:
    default:
        return "pMid";
    }
}

bool SaveLastStateToFile(const MonitoringCycleResult &result)
{
    File file = LittleFS.open(LAST_STATE_FILE, FILE_WRITE);
    if (!file)
    {
        Serial.println("Failed to open last_state.txt for writing");
        return false;
    }

    std::int64_t unixTime = GetCurrentUnixTimeUtc();
    if (unixTime == 0)
    {
        unixTime = result.snapshot.unixTime;
    }

    file.println("timestamp_local=" + FormatTimestampLocal(unixTime));
    file.println("soil_moisture_pct=" + String(result.snapshot.soilMoisturePct, 3));
    file.println("temperature_c=" + String(result.snapshot.temperatureC, 3));
    file.println("humidity_pct=" + String(result.snapshot.humidityPct, 3));
    file.println("light_level_pct=" + String(result.snapshot.lightLevelPct, 3));
    file.println("risk_class=" + String(static_cast<int>(result.mlResult.risk)));
    file.println("confidence=" + String(result.mlResult.confidence, 6));
    file.println("prob_healthy=" + String(result.mlResult.probabilities[0], 6));
    file.println("prob_moderate_stress=" + String(result.mlResult.probabilities[1], 6));
    file.println("prob_high_stress=" + String(result.mlResult.probabilities[2], 6));
    file.println("action_water=" + String(result.recommendation.water ? 1 : 0));
    file.println("action_reduce_temp=" + String(result.recommendation.reduceTemp ? 1 : 0));
    file.println("action_increase_humidity=" + String(result.recommendation.increaseHumidity ? 1 : 0));
    file.println("action_increase_light=" + String(result.recommendation.increaseLight ? 1 : 0));
    file.println("recommendation_summary=" + String(result.recommendation.summary.c_str()));

    file.close();
    Serial.println("Saved /last_state.txt");
    return true;
}

String GetDailyLogFilePath(std::int64_t unixTime)
{
    const std::time_t ts = static_cast<std::time_t>(unixTime);
    std::tm timeInfo{};
    localtime_r(&ts, &timeInfo);

    char buffer[32];
    const std::size_t len = std::strftime(buffer, sizeof(buffer), "/log_%Y-%m-%d.csv", &timeInfo);
    if (len == 0)
    {
        return "/log_unknown_date.csv";
    }

    return String(buffer);
}

String CsvEscape(const String &input)
{
    String escaped = input;
    escaped.replace("\"", "\"\"");
    return "\"" + escaped + "\"";
}

PreferenceBand ParsePreferenceBand(const String &input)
{
    if (input.equalsIgnoreCase("pLow") || input.equalsIgnoreCase("low"))
    {
        return PreferenceBand::pLow;
    }
    if (input.equalsIgnoreCase("pHigh") || input.equalsIgnoreCase("high"))
    {
        return PreferenceBand::pHigh;
    }
    return PreferenceBand::pMid;
}

PlantRuleProfile BuildPlantSettingsFromForm(const PlantRuleProfile &existingProfile)
{
    PlantRuleProfile profile = existingProfile;

    if (server.hasArg("plantName"))
    {
        profile.plantName = server.arg("plantName").c_str();
    }

    profile.preferences.soilMoisture = ParsePreferenceBand(server.arg("soilPreference"));
    profile.preferences.temperature = ParsePreferenceBand(server.arg("temperaturePreference"));
    profile.preferences.humidity = ParsePreferenceBand(server.arg("humidityPreference"));
    profile.preferences.light = ParsePreferenceBand(server.arg("lightPreference"));

    return profile;
}

bool SyncTimeWithNtp(unsigned long timeoutMs = 20000)
{
    if (WiFi.status() != WL_CONNECTED)
    {
        Serial.println("Cannot sync time: WiFi not connected");
        timeSynced = false;
        return false;
    }

    configTzTime("CET-1CEST,M3.5.0/2,M10.5.0/3", "pool.ntp.org", "time.nist.gov", "time.google.com");

    Serial.println("Syncing time with NTP...");
    struct tm timeInfo{};
    const unsigned long start = millis();

    while (millis() - start < timeoutMs)
    {
        if (getLocalTime(&timeInfo, 1000))
        {
            char buffer[32];
            strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &timeInfo);

            Serial.print("NTP time synced. Local time: ");
            Serial.println(buffer);

            timeSynced = true;
            return true;
        }

        Serial.println("Waiting for NTP time...");
        delay(500);
    }

    Serial.println("NTP time sync failed");
    timeSynced = false;
    return false;
}

String EnsureLeadingSlash(const String &path)
{
    if (path.startsWith("/"))
    {
        return path;
    }
    return "/" + path;
}

bool EnsureCsvHeader(const String &filePath)
{
    if (LittleFS.exists(filePath))
    {
        return true;
    }

    File file = LittleFS.open(filePath, FILE_WRITE);
    if (!file)
    {
        Serial.print("Failed to create CSV file: ");
        Serial.println(filePath);
        return false;
    }

    file.println(
        "plant_label,device_name,device_id,timestamp_utc,unix_time,"
        "soil_moisture_pct,temperature_c,humidity_pct,light_level_pct,"
        "risk_class,confidence,prob_healthy,prob_moderate_stress,prob_high_stress,"
        "action_water,action_reduce_temp,action_increase_humidity,action_increase_light,"
        "recommendation_summary");

    file.close();

    Serial.print("Created CSV with header: ");
    Serial.println(filePath);
    return true;
}

bool LogToCsv(const MonitoringCycleResult &result)
{
    const auto &snapshot = result.snapshot;
    const auto &mlResult = result.mlResult;
    const auto &rec = result.recommendation;

    std::int64_t logUnixTime = snapshot.unixTime;
    const std::int64_t syncedUnixTime = GetCurrentUnixTimeUtc();
    if (syncedUnixTime > 0)
    {
        logUnixTime = syncedUnixTime;
    }

    const String timestampLocal = FormatTimestampLocal(logUnixTime);
    const String filePath = GetDailyLogFilePath(logUnixTime);

    if (!EnsureCsvHeader(filePath))
    {
        return false;
    }

    File file = LittleFS.open(filePath, FILE_APPEND);
    if (!file)
    {
        Serial.print("Failed to open CSV file for append: ");
        Serial.println(filePath);
        return false;
    }

    String line;
    line.reserve(512);

    line += CsvEscape(String(PLANT_LABEL)) + ",";
    line += CsvEscape(String(DEVICE_NAME)) + ",";
    line += String(DEVICE_ID) + ",";
    line += CsvEscape(timestampLocal) + ",";
    line += String(static_cast<long long>(logUnixTime)) + ",";
    line += String(snapshot.soilMoisturePct, 3) + ",";
    line += String(snapshot.temperatureC, 3) + ",";
    line += String(snapshot.humidityPct, 3) + ",";
    line += String(snapshot.lightLevelPct, 3) + ",";
    line += String(static_cast<int>(mlResult.risk)) + ",";
    line += String(mlResult.confidence, 6) + ",";
    line += String(mlResult.probabilities[0], 6) + ",";
    line += String(mlResult.probabilities[1], 6) + ",";
    line += String(mlResult.probabilities[2], 6) + ",";
    line += String(rec.water ? 1 : 0) + ",";
    line += String(rec.reduceTemp ? 1 : 0) + ",";
    line += String(rec.increaseHumidity ? 1 : 0) + ",";
    line += String(rec.increaseLight ? 1 : 0) + ",";
    line += CsvEscape(String(rec.summary.c_str()));

    file.println(line);
    file.flush();
    const size_t currentSize = file.size();
    file.close();

    ++logCount;
    Serial.print("Logged row #");
    Serial.println(logCount);
    Serial.print("Written to: ");
    Serial.println(filePath);
    Serial.println(line);
    Serial.print("File size: ");
    Serial.println(currentSize);

    return true;
}

void DumpFileToSerial(const String &filePath)
{
    File file = LittleFS.open(filePath, FILE_READ);
    if (!file)
    {
        Serial.print("Failed to open file for reading: ");
        Serial.println(filePath);
        return;
    }

    Serial.print("---- FILE START ");
    Serial.print(filePath);
    Serial.println(" ----");

    while (file.available())
    {
        Serial.write(file.read());
    }

    Serial.println();
    Serial.print("---- FILE END ");
    Serial.print(filePath);
    Serial.println(" ----");

    file.close();
}

void ConnectWiFi()
{
    if (String(WIFI_SSID).length() == 0)
    {
        Serial.println("WIFI_SSID is empty; WiFi disabled");
        return;
    }

    if (WiFi.status() == WL_CONNECTED)
    {
        Serial.print("WiFi connected. IP: ");
        Serial.println(WiFi.localIP());

        delay(1000);
        SyncTimeWithNtp();
        return;
    }

    Serial.print("Connecting to WiFi SSID: ");
    Serial.println(WIFI_SSID);

    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    const unsigned long startedAt = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - startedAt < 30000)
    {
        delay(500);
        Serial.print('.');
    }
    Serial.println();

    if (WiFi.status() == WL_CONNECTED)
    {
        Serial.print("WiFi connected. IP: ");
        Serial.println(WiFi.localIP());
        delay(1000);
        SyncTimeWithNtp();
    }
    else
    {
        Serial.println("WiFi connection timed out");
    }
}

String GetContentType(const String &path)
{
    if (path.endsWith(".csv"))
    {
        return "text/csv";
    }
    if (path.endsWith(".txt"))
    {
        return "text/plain";
    }
    if (path.endsWith(".html"))
    {
        return "text/html";
    }
    if (path.endsWith(".json"))
    {
        return "application/json";
    }
    return "application/octet-stream";
}

bool IsSafePath(const String &path)
{
    if (path.length() == 0)
    {
        return false;
    }

    if (path.indexOf("..") >= 0)
    {
        return false;
    }

    return true;
}

String NormalizeFilePath(const String &path)
{
    if (path.length() == 0)
    {
        return "";
    }

    if (path.startsWith("/"))
    {
        return path;
    }

    return "/" + path;
}

String FindLatestLogFile()
{
    File root = LittleFS.open("/");
    if (!root || !root.isDirectory())
    {
        return "";
    }

    String latestName;
    File file = root.openNextFile();
    while (file)
    {
        const String name = String(file.name());
        if (name.startsWith("/log_") && name.endsWith(".csv"))
        {
            if (latestName.length() == 0 || name > latestName)
            {
                latestName = name;
            }
        }
        file = root.openNextFile();
    }

    return latestName;
}

void HandleDelete()
{
    if (!server.hasArg("file"))
    {
        server.send(400, "text/plain", "Missing 'file' query parameter");
        return;
    }

    const String rawPath = server.arg("file");
    if (!IsSafePath(rawPath))
    {
        server.send(400, "text/plain", "Invalid file path");
        return;
    }

    const String filePath = NormalizeFilePath(rawPath);

    if (!LittleFS.exists(filePath))
    {
        server.send(404, "text/plain", "File not found");
        return;
    }

    if (!LittleFS.remove(filePath))
    {
        server.send(500, "text/plain", "Failed to delete file");
        return;
    }

    Serial.print("Deleted file: ");
    Serial.println(filePath);
    server.send(200, "text/plain", "Deleted " + filePath);
}

bool IsViewableTextFile(const String &path)
{
    return path.endsWith(".csv") ||
           path.endsWith(".txt") ||
           path.endsWith(".json") ||
           path.endsWith(".html");
}

bool IsImportantFile(const String &path)
{
    const String normalized = EnsureLeadingSlash(path);
    return normalized == PLANT_SETTINGS_FILE || normalized == LAST_STATE_FILE;
}

bool IsLogFile(const String &path)
{
    const String normalized = EnsureLeadingSlash(path);
    return normalized.endsWith(".csv");
}

size_t CountImportantFiles()
{
    File root = LittleFS.open("/");
    if (!root || !root.isDirectory())
    {
        return 0;
    }

    size_t count = 0;
    File file = root.openNextFile();
    while (file)
    {
        const String name = String(file.name());
        if (IsImportantFile(name))
        {
            ++count;
        }
        file = root.openNextFile();
    }
    return count;
}

size_t CountLogFiles()
{
    File root = LittleFS.open("/");
    if (!root || !root.isDirectory())
    {
        return 0;
    }

    size_t count = 0;
    File file = root.openNextFile();
    while (file)
    {
        const String name = String(file.name());
        if (IsLogFile(name))
        {
            ++count;
        }
        file = root.openNextFile();
    }
    return count;
}

String BuildFilteredFilesHtmlTable(bool (*filterFn)(const String &))
{
    String html;
    html.reserve(5000);

    File root = LittleFS.open("/");
    if (!root || !root.isDirectory())
    {
        html += "<p>Could not open LittleFS root directory.</p>";
        return html;
    }

    File file = root.openNextFile();
    bool foundAny = false;

    html += "<table border='1' cellpadding='6' cellspacing='0' style='border-collapse:collapse; width:100%; max-width:900px;'>";
    html += "<thead><tr><th align='left'>File</th><th align='left'>Size (bytes)</th><th align='left'>Actions</th></tr></thead><tbody>";

    while (file)
    {
        const String name = String(file.name());

        if (filterFn(name))
        {
            foundAny = true;

            String normalizedName = name;
            if (normalizedName.startsWith("/"))
            {
                normalizedName = normalizedName.substring(1);
            }

            html += "<tr>";
            html += "<td><code>";
            html += name;
            html += "</code></td>";
            html += "<td>";
            html += String(file.size());
            html += "</td>";
            html += "<td>";

            if (IsViewableTextFile(name))
            {
                html += "<a href='/view?file=" + normalizedName + "'>View</a> | ";
            }

            html += "<a href='/download?file=" + normalizedName + "'>Download</a>";

            if (!IsImportantFile(name))
            {
                html += " | <form method='POST' action='/delete' style='display:inline; margin:0;'>";
                html += "<input type='hidden' name='file' value='" + normalizedName + "'>";
                html += "<button type='submit' onclick='return confirm(\"Delete this file?\")'>Delete</button>";
                html += "</form>";
            }

            html += "</td>";
            html += "</tr>";
        }

        file = root.openNextFile();
    }

    html += "</tbody></table>";

    if (!foundAny)
    {
        return "<p>No matching files found.</p>";
    }

    return html;
}

void HandleRoot()
{
    const PlantRuleProfile profile = monitoringSystem.GetPlantProfile();

    String html;
    html.reserve(18000);

    html += "<!DOCTYPE html><html><head><meta charset='utf-8'>";
    html += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
    html += "<title>POC-03 Plant Settings</title>";
    html += "</head><body style='font-family:Arial,sans-serif;max-width:1100px;margin:0 auto;padding:20px;line-height:1.4;'>";

    html += "<h1>POC-03 Plant Settings & Device Files</h1>";
    html += "<p><strong>Device:</strong> ";
    html += DEVICE_NAME;
    html += " (ID ";
    html += String(DEVICE_ID);
    html += ")</p>";

    html += "<p><strong>Plant:</strong> ";
    html += String(profile.plantName.c_str());
    html += "</p>";

    html += "<p><strong>Time sync:</strong> ";
    html += timeSynced ? "NTP synced" : "Not synced";
    html += "</p>";

    const std::int64_t nowUnix = GetCurrentUnixTimeUtc();
    if (nowUnix > 0)
    {
        html += "<p><strong>Current local time:</strong> ";
        html += FormatTimestampLocal(nowUnix);
        html += "</p>";
    }

    html += "<h2>Adjust current plant settings</h2>";
    html += "<p>This ESP32 is already linked to a specific device. Use this page only to adjust the active plant preferences.</p>";

    html += "<form id='settingsForm'>";
    html += "<label>Plant name<br><input name='plantName' required style='width:320px;padding:6px;' value='";
    html += String(profile.plantName.c_str());
    html += "' /></label><br><br>";

    html += "<label>Humidity preference<br>";
    html += "<select name='humidityPreference' style='padding:6px;'>";
    html += "<option value='pLow'>pLow</option><option value='pMid'>pMid</option><option value='pHigh'>pHigh</option>";
    html += "</select></label><br><br>";

    html += "<label>Temperature preference<br>";
    html += "<select name='temperaturePreference' style='padding:6px;'>";
    html += "<option value='pLow'>pLow</option><option value='pMid'>pMid</option><option value='pHigh'>pHigh</option>";
    html += "</select></label><br><br>";

    html += "<label>Soil moisture preference<br>";
    html += "<select name='soilPreference' style='padding:6px;'>";
    html += "<option value='pLow'>pLow</option><option value='pMid'>pMid</option><option value='pHigh'>pHigh</option>";
    html += "</select></label><br><br>";

    html += "<label>Light preference<br>";
    html += "<select name='lightPreference' style='padding:6px;'>";
    html += "<option value='pLow'>pLow</option><option value='pMid'>pMid</option><option value='pHigh'>pHigh</option>";
    html += "</select></label><br><br>";

    html += "<button type='submit' style='padding:8px 14px;'>Save settings</button>";
    html += " <button type='button' onclick='location.reload()' style='padding:8px 14px;'>Refresh page</button>";
    html += "</form>";

    html += "<pre id='result' style='background:#f5f5f5;padding:10px;white-space:pre-wrap;'></pre>";

    html += "<hr>";
    html += "<h2>Important files</h2>";
    html += "<p>Total important files found: ";
    html += String(CountImportantFiles());
    html += "</p>";
    html += BuildFilteredFilesHtmlTable(IsImportantFile);

    html += "<hr>";
    html += "<h2>Log files</h2>";
    html += "<p>Total log files found: ";
    html += String(CountLogFiles());
    html += "</p>";
    html += BuildFilteredFilesHtmlTable(IsLogFile);

    html += "<p style='margin-top:16px;'><a href='/latest'>Download latest log</a></p>";
    html += "<p><a href='/list'>View file list as plain text</a></p>";

    html += "<script>";
    html += "async function loadSettings(){";
    html += "const r=await fetch('/api/plant-settings');";
    html += "const s=await r.json();";
    html += "const form=document.getElementById('settingsForm');";
    html += "form.plantName.value=s.plantName;";
    html += "form.humidityPreference.value=s.humidityPreference;";
    html += "form.temperaturePreference.value=s.temperaturePreference;";
    html += "form.soilPreference.value=s.soilPreference;";
    html += "form.lightPreference.value=s.lightPreference;";
    html += "}";
    html += "document.getElementById('settingsForm').addEventListener('submit', async (e)=>{";
    html += "e.preventDefault();";
    html += "const fd=new FormData(e.target);";
    html += "const body=new URLSearchParams(fd);";
    html += "const response=await fetch('/api/plant-settings',{method:'POST',headers:{'Content-Type':'application/x-www-form-urlencoded'},body});";
    html += "document.getElementById('result').textContent=await response.text();";
    html += "await loadSettings();";
    html += "});";
    html += "loadSettings();";
    html += "</script>";

    html += "</body></html>";

    server.send(200, "text/html", html);
}

void HandleDownload()
{
    if (!server.hasArg("file"))
    {
        server.send(400, "text/plain", "Missing 'file' query parameter");
        return;
    }

    const String rawPath = server.arg("file");
    Serial.print("Requested raw file path: ");
    Serial.println(rawPath);

    if (!IsSafePath(rawPath))
    {
        server.send(400, "text/plain", "Invalid file path");
        return;
    }

    const String filePath = NormalizeFilePath(rawPath);

    Serial.print("Normalized file path: ");
    Serial.println(filePath);

    if (!LittleFS.exists(filePath))
    {
        server.send(404, "text/plain", "File not found");
        return;
    }

    File file = LittleFS.open(filePath, FILE_READ);
    if (!file)
    {
        server.send(500, "text/plain", "Failed to open file");
        return;
    }

    server.sendHeader("Content-Disposition", "attachment; filename=\"" + filePath.substring(1) + "\"");
    server.streamFile(file, GetContentType(filePath));
    file.close();
}

void HandleView()
{
    if (!server.hasArg("file"))
    {
        server.send(400, "text/plain", "Missing 'file' query parameter");
        return;
    }

    const String rawPath = server.arg("file");
    if (!IsSafePath(rawPath))
    {
        server.send(400, "text/plain", "Invalid file path");
        return;
    }

    const String filePath = NormalizeFilePath(rawPath);

    if (!LittleFS.exists(filePath))
    {
        server.send(404, "text/plain", "File not found");
        return;
    }

    if (!IsViewableTextFile(filePath))
    {
        server.send(415, "text/plain", "Viewing not supported for this file type");
        return;
    }

    File file = LittleFS.open(filePath, FILE_READ);
    if (!file)
    {
        server.send(500, "text/plain", "Failed to open file");
        return;
    }

    String content;
    content.reserve(file.size() + 64);
    while (file.available())
    {
        content += char(file.read());
    }
    file.close();

    String html;
    html.reserve(content.length() + 1200);
    html += "<!DOCTYPE html><html><head><meta charset='utf-8'>";
    html += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
    html += "<title>View file</title></head><body style='font-family:Arial,sans-serif;padding:20px;'>";
    html += "<h1>Viewing ";
    html += filePath;
    html += "</h1>";
    html += "<p><a href='/'>Back</a> | ";
    html += "<a href='/download?file=" + rawPath + "'>Download</a></p>";
    html += "<pre style='white-space:pre-wrap;word-wrap:break-word;background:#f5f5f5;padding:12px;border:1px solid #ddd;'>";
    html += content;
    html += "</pre></body></html>";

    server.send(200, "text/html", html);
}

void HandleLatest()
{
    const String latestFile = FindLatestLogFile();
    if (latestFile.length() == 0)
    {
        server.send(404, "text/plain", "No log files found");
        return;
    }

    File file = LittleFS.open(latestFile, FILE_READ);
    if (!file)
    {
        server.send(500, "text/plain", "Failed to open latest log file");
        return;
    }

    server.sendHeader("Content-Disposition", "attachment; filename=\"" + latestFile.substring(1) + "\"");
    server.streamFile(file, "text/csv");
    file.close();
}

void HandleList()
{
    String text;
    text.reserve(4096);

    File root = LittleFS.open("/");
    if (!root || !root.isDirectory())
    {
        server.send(500, "text/plain", "Could not open root directory");
        return;
    }

    File file = root.openNextFile();
    while (file)
    {
        text += String(file.name());
        text += " | ";
        text += String(file.size());
        text += " bytes\n";
        file = root.openNextFile();
    }

    if (text.length() == 0)
    {
        text = "No files found\n";
    }

    server.send(200, "text/plain", text);
}

bool SavePlantSettingsToFile(const PlantRuleProfile &profile)
{
    File file = LittleFS.open(PLANT_SETTINGS_FILE, FILE_WRITE);
    if (!file)
    {
        Serial.println("Failed to open plant_settings.txt for writing");
        return false;
    }

    file.println("plantName=" + String(profile.plantName.c_str()));
    file.println("deviceName=" + String(profile.deviceName.c_str()));
    file.println("deviceId=" + String(profile.deviceId.c_str()));
    file.println("soilPreference=" + PreferenceToString(profile.preferences.soilMoisture));
    file.println("temperaturePreference=" + PreferenceToString(profile.preferences.temperature));
    file.println("humidityPreference=" + PreferenceToString(profile.preferences.humidity));
    file.println("lightPreference=" + PreferenceToString(profile.preferences.light));

    file.close();
    Serial.println("Saved /plant_settings.txt");
    return true;
}

void HandleGetPlantSettings()
{
    const PlantRuleProfile profile = monitoringSystem.GetPlantProfile();

    String json = "{";
    json += "\"plantName\":\"" + String(profile.plantName.c_str()) + "\",";
    json += "\"deviceId\":\"" + String(profile.deviceId.c_str()) + "\",";
    json += "\"deviceName\":\"" + String(profile.deviceName.c_str()) + "\",";
    json += "\"soilPreference\":\"" + PreferenceToString(profile.preferences.soilMoisture) + "\",";
    json += "\"temperaturePreference\":\"" + PreferenceToString(profile.preferences.temperature) + "\",";
    json += "\"humidityPreference\":\"" + PreferenceToString(profile.preferences.humidity) + "\",";
    json += "\"lightPreference\":\"" + PreferenceToString(profile.preferences.light) + "\"";
    json += "}";

    server.send(200, "application/json", json);
}

void HandleUpdatePlantSettings()
{
    if (!server.hasArg("soilPreference") ||
        !server.hasArg("temperaturePreference") ||
        !server.hasArg("humidityPreference") ||
        !server.hasArg("lightPreference"))
    {
        server.send(400, "text/plain", "Missing one or more preference fields");
        return;
    }

    const PlantRuleProfile existingProfile = monitoringSystem.GetPlantProfile();
    const PlantRuleProfile profile = BuildPlantSettingsFromForm(existingProfile);
    monitoringSystem.SetPlantProfile(profile);
    SavePlantSettingsToFile(profile);

    String response = "Updated personalized settings for ";
    response += String(profile.plantName.c_str());
    response += " on this ESP32.";
    response += " Preferences: soil=" + server.arg("soilPreference");
    response += ", temperature=" + server.arg("temperaturePreference");
    response += ", humidity=" + server.arg("humidityPreference");
    response += ", light=" + server.arg("lightPreference");

    server.send(200, "text/plain", response);
}

void StartWebServer()
{
#if ENABLE_WIFI_DOWNLOAD
    if (WiFi.status() != WL_CONNECTED)
    {
        Serial.println("WiFi not connected; web server not started");
        return;
    }

    server.on("/", HTTP_GET, HandleRoot);
    server.on("/download", HTTP_GET, HandleDownload);
    server.on("/view", HTTP_GET, HandleView);
    server.on("/latest", HTTP_GET, HandleLatest);
    server.on("/list", HTTP_GET, HandleList);
    server.on("/api/plant-settings", HTTP_GET, HandleGetPlantSettings);
    server.on("/api/plant-settings", HTTP_POST, HandleUpdatePlantSettings);
    server.on("/delete", HTTP_POST, HandleDelete);

    server.on("/favicon.ico", HTTP_GET, []()
              { server.send(204); });

    server.onNotFound([]()
                      {
        Serial.print("Not found route: ");
        Serial.print(server.uri());
        Serial.print(" | method: ");
        Serial.println(server.method() == HTTP_GET ? "GET" :
                       server.method() == HTTP_POST ? "POST" : "OTHER");
        server.send(404, "text/plain", "Route not found"); });

    server.begin();

    Serial.println("Web server started");
    Serial.print("Open in browser: http://");
    Serial.println(WiFi.localIP());
#else
    Serial.println("WiFi download disabled");
#endif
}

void setup()
{
    Serial.begin(115200);
    delay(200);

    if (!LittleFS.begin(true))
    {
        Serial.println("LittleFS mount failed");
        while (true)
        {
        }
    }
    Serial.println("LittleFS mounted");

    if (String(WIFI_SSID).length() > 0)
    {
        ConnectWiFi();
    }

    PlantRuleProfile initialProfile;
    initialProfile.plantName = PLANT_LABEL;
    initialProfile.deviceName = DEVICE_NAME;
    initialProfile.deviceId = String(DEVICE_ID).c_str();
    monitoringSystem.SetPlantProfile(initialProfile);
    SavePlantSettingsToFile(initialProfile);

    if (!monitoringSystem.Init())
    {
        Serial.println("Failed to initialize monitoring system");
        while (true)
        {
        }
    }

    Serial.println("Monitoring system initialized");

    const auto result = monitoringSystem.RunCycleDetailed();
    LogToCsv(result);
    SaveLastStateToFile(result);

    const String latestFile = FindLatestLogFile();
    if (latestFile.length() > 0)
    {
        DumpFileToSerial(latestFile);
    }

    StartWebServer();
    lastRun = millis();
}

void loop()
{
#if ENABLE_WIFI_DOWNLOAD
    server.handleClient();
#endif

    const unsigned long now = millis();

    if (now - lastRun >= INTERVAL)
    {
        lastRun = now;

        if (String(WIFI_SSID).length() > 0 && WiFi.status() != WL_CONNECTED)
        {
            ConnectWiFi();
        }
        else if (!timeSynced && WiFi.status() == WL_CONNECTED)
        {
            SyncTimeWithNtp();
        }

        const auto result = monitoringSystem.RunCycleDetailed();
        LogToCsv(result);
        SaveLastStateToFile(result);
    }
}