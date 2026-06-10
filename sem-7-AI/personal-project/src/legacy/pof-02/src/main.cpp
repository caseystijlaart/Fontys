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
MonitoringSystem monitoringSystem(soil, dht, light, ml, recEngine, 24);

const unsigned long INTERVAL = 3600000;
unsigned long lastRun = 0;
unsigned long logCount = 0;

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

String FormatTimestampUtc(std::int64_t unixTime)
{
    const std::time_t ts = static_cast<std::time_t>(unixTime);
    std::tm timeInfo{};
    gmtime_r(&ts, &timeInfo);

    char buffer[24];
    const std::size_t len = std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &timeInfo);
    if (len == 0)
    {
        return "1970-01-01 00:00:00";
    }

    return String(buffer);
}

String GetDailyLogFilePath(std::int64_t unixTime)
{
    const std::time_t ts = static_cast<std::time_t>(unixTime);
    std::tm timeInfo{};
    gmtime_r(&ts, &timeInfo);

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

    const String timestampUtc = FormatTimestampUtc(snapshot.unixTime);
    const String filePath = GetDailyLogFilePath(snapshot.unixTime);

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
    line += CsvEscape(timestampUtc) + ",";
    line += String(static_cast<long long>(snapshot.unixTime)) + ",";
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
    return "application/octet-stream";
}

void HandleRoot()
{
    String html;
    html.reserve(4096);

    html += "<!DOCTYPE html><html><head><meta charset='utf-8'>";
    html += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
    html += "<title>ESP32 Log Files</title></head><body>";
    html += "<h1>ESP32 Log Files</h1>";
    html += "<p>Device: ";
    html += DEVICE_NAME;
    html += " (ID ";
    html += String(DEVICE_ID);
    html += ")</p>";
    html += "<p>Plant: ";
    html += PLANT_LABEL;
    html += "</p>";
    html += "<ul>";

    File root = LittleFS.open("/");
    if (!root || !root.isDirectory())
    {
        html += "<li>Could not open root directory</li>";
    }
    else
    {
        File file = root.openNextFile();
        bool foundAny = false;

        while (file)
        {
            const String name = String(file.name());
            String downloadName = name;
            if (downloadName.startsWith("/"))
            {
                downloadName = downloadName.substring(1);
            }

            html += "<li><a href=\"/download?file=";
            html += downloadName;
            html += "\">";
            html += name;
            html += "</a> (";
            html += String(file.size());
            html += " bytes)</li>";

            foundAny = true;
            file = root.openNextFile();
        }

        if (!foundAny)
        {
            html += "<li>No files found</li>";
        }
    }

    html += "</ul>";
    html += "<p><a href=\"/latest\">Download latest log</a></p>";
    html += "<p><a href=\"/list\">View file list as plain text</a></p>";
    html += "</body></html>";

    server.send(200, "text/html", html);
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
    text.reserve(2048);

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
    server.on("/latest", HTTP_GET, HandleLatest);
    server.on("/list", HTTP_GET, HandleList);
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

    StartWebServer();

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

    const String latestFile = FindLatestLogFile();
    if (latestFile.length() > 0)
    {
        DumpFileToSerial(latestFile);
    }

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

        const auto result = monitoringSystem.RunCycleDetailed();
        LogToCsv(result);
    }
}