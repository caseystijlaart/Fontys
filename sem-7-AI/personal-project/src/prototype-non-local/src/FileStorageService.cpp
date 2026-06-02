#include "FileStorageService.hpp"

#include <FS.h>
#include <LittleFS.h>

#include <algorithm>
#include <ctime>
#include <vector>

#include "TimeService.hpp"

using namespace pof02;

namespace
{
String EnsureAbsolutePath(const String &path)
{
    if (path.startsWith("/"))
    {
        return path;
    }
    return "/" + path;
}

String JoinPath(const String &dir, const String &name)
{
    if (name.startsWith("/"))
    {
        return name;
    }

    if (dir == "/")
    {
        return "/" + name;
    }

    return dir + "/" + name;
}

String EntryNameOnly(const String &entryName)
{
    const int slashIndex = entryName.lastIndexOf('/');
    if (slashIndex >= 0)
    {
        return entryName.substring(slashIndex + 1);
    }
    return entryName;
}

std::vector<String> ListAllFilesRecursive()
{
    std::vector<String> files;
    std::vector<String> dirs{"/"};

    while (!dirs.empty())
    {
        const String dirPath = dirs.back();
        dirs.pop_back();

        File dir = LittleFS.open(dirPath, FILE_READ);
        if (!dir || !dir.isDirectory())
        {
            continue;
        }

        File entry = dir.openNextFile();
        while (entry)
        {
            const String entryName = EntryNameOnly(String(entry.name()));
            const String fullPath = JoinPath(dirPath, entryName);

            if (entry.isDirectory())
            {
                dirs.push_back(fullPath);
            }
            else
            {
                files.push_back(fullPath);
            }

            entry.close();
            entry = dir.openNextFile();
        }

        dir.close();
    }

    std::sort(files.begin(), files.end());
    return files;
}
}

FileStorageService::FileStorageService(TimeService &timeService)
    : timeService_(timeService),
      plantSettingsFile_("/plant_settings.txt"),
      lastStateFile_("/last_state.txt")
{
}

bool FileStorageService::SaveLastStateToFile(const MonitoringCycleResult &result, const char *lastStateFile)
{
    File file = LittleFS.open(lastStateFile, FILE_WRITE);
    if (!file)
    {
        Serial.println("Failed to open last_state.txt for writing");
        return false;
    }

    std::int64_t unixTime = timeService_.GetCurrentUnixTimeUtc();
    if (unixTime == 0)
    {
        unixTime = result.snapshot.unixTime;
    }

    file.println("timestamp_local=" + timeService_.FormatTimestampLocal(unixTime));
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
    file.println("action_increase_light=" + String(result.recommendation.increaseLight ? 1 : 0));
    file.println("recommendation_summary=" + String(result.recommendation.summary.c_str()));

    file.close();
    Serial.println("Saved /last_state.txt");
    return true;
}

bool FileStorageService::LogToCsv(const MonitoringCycleResult &result,
                                  const char *plantLabel,
                                  const char *deviceName,
                                  int deviceId,
                                  unsigned long &logCount)
{
    const auto &snapshot = result.snapshot;
    const auto &mlResult = result.mlResult;
    const auto &rec = result.recommendation;

    std::int64_t logUnixTime = snapshot.unixTime;
    const std::int64_t syncedUnixTime = timeService_.GetCurrentUnixTimeUtc();
    if (syncedUnixTime > 0)
    {
        logUnixTime = syncedUnixTime;
    }

    const String timestampLocal = timeService_.FormatTimestampLocal(logUnixTime);
    const String filePath = GetDailyLogFilePath(logUnixTime);
    const String mlFilePath = GetMlLogFilePath(logUnixTime);

    if (!EnsureCsvHeader(filePath) || !EnsureCsvHeader(mlFilePath))
    {
        return false;
    }

    File file = LittleFS.open(filePath, FILE_APPEND);
    File mlFile = LittleFS.open(mlFilePath, FILE_APPEND);
    if (!file || !mlFile)
    {
        Serial.println("Failed to open log file(s) for append");
        if (file)
            file.close();
        if (mlFile)
            mlFile.close();
        return false;
    }

    String line;
    line.reserve(320);
    line += CsvEscape(String(plantLabel)) + ",";
    line += String(deviceId) + ",";
    line += CsvEscape(timestampLocal) + ",";
    line += String(snapshot.soilMoisturePct, 3) + ",";
    line += String(snapshot.temperatureC, 3) + ",";
    line += String(snapshot.humidityPct, 3) + ",";
    line += String(snapshot.lightLevelPct, 3) + ",";
    line += String(rec.water ? 1 : 0) + ",";
    line += String(rec.reduceTemp ? 1 : 0) + ",";
    line += String(rec.increaseLight ? 1 : 0) + ",";
    line += CsvEscape(String(rec.summary.c_str()));

    String mlLine;
    mlLine.reserve(220);
    mlLine += CsvEscape(String(plantLabel)) + ",";
    mlLine += CsvEscape(timestampLocal) + ",";
    mlLine += String(static_cast<int>(mlResult.risk)) + ",";
    mlLine += String(mlResult.confidence, 6) + ",";
    mlLine += String(mlResult.probabilities[0], 6) + ",";
    mlLine += String(mlResult.probabilities[1], 6) + ",";
    mlLine += String(mlResult.probabilities[2], 6);

    file.println(line);
    mlFile.println(mlLine);
    file.flush();
    mlFile.flush();
    file.close();
    mlFile.close();

    ++logCount;
    Serial.print("Logged row #");
    Serial.println(logCount);
    Serial.println(filePath);
    Serial.println(mlFilePath);

    return true;
}

namespace
{

std::vector<String> ParseCsvLine(const String &line)
{
    std::vector<String> parts;
    int i = 0;
    const int len = static_cast<int>(line.length());

    while (i <= len)
    {
        if (i < len && line[i] == '"')
        {
            ++i;
            String field;
            while (i < len)
            {
                if (line[i] == '"')
                {
                    if (i + 1 < len && line[i + 1] == '"')
                    {
                        field += '"';
                        i += 2;
                    }
                    else
                    {
                        ++i;
                        break;
                    }
                }
                else
                {
                    field += line[i++];
                }
            }
            parts.push_back(field);
            if (i < len && line[i] == ',')
                ++i;
        }
        else
        {
            const int comma = line.indexOf(',', i);
            if (comma < 0)
            {
                parts.push_back(line.substring(i));
                break;
            }
            parts.push_back(line.substring(i, comma));
            i = comma + 1;
        }
    }

    return parts;
}

std::int64_t ParseTimestamp(const String &ts)
{
    int year, month, day, hour, min, sec;
    if (sscanf(ts.c_str(), "%d-%d-%d %d:%d:%d", &year, &month, &day, &hour, &min, &sec) != 6)
    {
        return 0;
    }

    std::tm t{};
    t.tm_year = year - 1900;
    t.tm_mon = month - 1;
    t.tm_mday = day;
    t.tm_hour = hour;
    t.tm_min = min;
    t.tm_sec = sec;
    t.tm_isdst = -1;

    const time_t result = mktime(&t);
    return result < 0 ? 0 : static_cast<std::int64_t>(result);
}

} // namespace

std::vector<SensorSnapshot> FileStorageService::LoadRecentSnapshots(std::size_t maxEntries) const
{
    // Collect all sensor log files (not ML logs), sorted newest-first
    std::vector<String> logFiles;
    for (const auto &name : ListAllFilesRecursive())
    {
        if (name.indexOf("/log_") >= 0 && name.endsWith(".csv") && name.indexOf("/ml_") < 0)
        {
            logFiles.push_back(name);
        }
    }
    std::sort(logFiles.rbegin(), logFiles.rend());

    // Collect snapshots newest-first across files, then reverse to chronological order
    std::vector<SensorSnapshot> collected;
    collected.reserve(maxEntries);

    for (const auto &filePath : logFiles)
    {
        if (collected.size() >= maxEntries)
            break;

        File file = LittleFS.open(filePath, FILE_READ);
        if (!file)
            continue;

        std::vector<String> lines;
        while (file.available())
        {
            String line = file.readStringUntil('\n');
            line.trim();
            if (line.length() > 0)
                lines.push_back(line);
        }
        file.close();

        // Skip header row (index 0), iterate rows newest-first
        for (int i = static_cast<int>(lines.size()) - 1; i >= 1; --i)
        {
            if (collected.size() >= maxEntries)
                break;

            const auto parts = ParseCsvLine(lines[i]);
            if (parts.size() < 7)
                continue;

            const std::int64_t ts = ParseTimestamp(parts[2]);
            if (ts <= 0)
                continue;

            SensorSnapshot s{};
            s.unixTime = ts;
            s.soilMoisturePct = parts[3].toFloat();
            s.temperatureC = parts[4].toFloat();
            s.humidityPct = parts[5].toFloat();
            s.lightLevelPct = parts[6].toFloat();
            collected.push_back(s);
        }
    }

    // Reverse to chronological order (oldest first) as SensorHistory expects
    std::reverse(collected.begin(), collected.end());
    return collected;
}

std::vector<SensorSnapshot> FileStorageService::LoadHistoryFile(const char *filePath, std::size_t maxEntries, std::size_t &outTotalCount) const
{
    outTotalCount = 0;

    File file = LittleFS.open(filePath, FILE_READ);
    if (!file)
        return {};

    std::vector<String> lines;
    while (file.available())
    {
        String line = file.readStringUntil('\n');
        line.trim();
        if (line.length() > 0)
            lines.push_back(line);
    }
    file.close();

    if (lines.size() <= 1)
        return {};

    const std::size_t dataRows = lines.size() - 1; // subtract header
    outTotalCount = dataRows;

    const std::size_t startIdx = dataRows > maxEntries ? lines.size() - maxEntries : 1;

    std::vector<SensorSnapshot> snapshots;
    snapshots.reserve(std::min(dataRows, maxEntries));

    for (std::size_t i = startIdx; i < lines.size(); ++i)
    {
        const auto parts = ParseCsvLine(lines[i]);
        if (parts.size() < 7)
            continue;

        const std::int64_t ts = ParseTimestamp(parts[2]);
        if (ts <= 0)
            continue;

        SensorSnapshot s{};
        s.unixTime = ts;
        s.soilMoisturePct = parts[3].toFloat();
        s.temperatureC = parts[4].toFloat();
        s.humidityPct = parts[5].toFloat();
        s.lightLevelPct = parts[6].toFloat();
        snapshots.push_back(s);
    }

    return snapshots;
}

bool FileStorageService::AppendToHistoryFile(const char *filePath, const pof02::MonitoringCycleResult &result, const char *plantLabel, int deviceId, std::size_t maxEntries)
{
    const std::int64_t unixTime = timeService_.GetCurrentUnixTimeUtc() > 0
                                      ? timeService_.GetCurrentUnixTimeUtc()
                                      : result.snapshot.unixTime;
    const String timestamp = timeService_.FormatTimestampLocal(unixTime);

    // Create file with header if it does not exist yet
    if (!LittleFS.exists(filePath))
    {
        File f = LittleFS.open(filePath, FILE_WRITE);
        if (!f)
            return false;
        f.println("plant_label,device_id,timestamp_utc,soil_moisture_pct,temperature_c,humidity_pct,light_level_pct,action_water,action_reduce_temp,action_increase_light,recommendation_summary");
        f.close();
    }

    File file = LittleFS.open(filePath, FILE_APPEND);
    if (!file)
        return false;

    const auto &s = result.snapshot;
    const auto &rec = result.recommendation;

    String line;
    line.reserve(200);
    line += CsvEscape(String(plantLabel)) + ",";
    line += String(deviceId) + ",";
    line += CsvEscape(timestamp) + ",";
    line += String(s.soilMoisturePct, 3) + ",";
    line += String(s.temperatureC, 3) + ",";
    line += String(s.humidityPct, 3) + ",";
    line += String(s.lightLevelPct, 3) + ",";
    line += String(rec.water ? 1 : 0) + ",";
    line += String(rec.reduceTemp ? 1 : 0) + ",";
    line += String(rec.increaseLight ? 1 : 0) + ",";
    line += CsvEscape(String(rec.summary.c_str()));

    file.println(line);
    file.close();

    // Trim file when it has grown to double the desired window so flash usage stays bounded
    std::size_t ignored = 0;
    const std::size_t total = LoadHistoryFile(filePath, SIZE_MAX, ignored).size() + 1; // +1 for what we just wrote, approximate
    // Re-count properly
    File countFile = LittleFS.open(filePath, FILE_READ);
    std::size_t rowCount = 0;
    if (countFile)
    {
        while (countFile.available())
        {
            countFile.readStringUntil('\n');
            ++rowCount;
        }
        countFile.close();
        rowCount = rowCount > 1 ? rowCount - 1 : 0; // subtract header
    }

    if (rowCount > maxEntries * 2)
    {
        std::size_t dummy = 0;
        const auto keep = LoadHistoryFile(filePath, maxEntries, dummy);

        File rewrite = LittleFS.open(filePath, FILE_WRITE);
        if (rewrite)
        {
            rewrite.println("plant_label,device_id,timestamp_utc,soil_moisture_pct,temperature_c,humidity_pct,light_level_pct,action_water,action_reduce_temp,action_increase_light,recommendation_summary");
            // We only have parsed snapshots here so we re-format them minimally
            for (const auto &snap : keep)
            {
                const String ts = timeService_.FormatTimestampLocal(snap.unixTime);
                String l;
                l += CsvEscape(String(plantLabel)) + ",";
                l += String(deviceId) + ",";
                l += CsvEscape(ts) + ",";
                l += String(snap.soilMoisturePct, 3) + ",";
                l += String(snap.temperatureC, 3) + ",";
                l += String(snap.humidityPct, 3) + ",";
                l += String(snap.lightLevelPct, 3) + ",0,0,0,\"\"";
                rewrite.println(l);
            }
            rewrite.close();
            Serial.println("Trimmed sensor history file");
        }
    }

    return true;
}

bool FileStorageService::SavePlantSettingsToFile(const PlantRuleProfile &profile, const char *plantSettingsFile)
{
    File file = LittleFS.open(plantSettingsFile, FILE_WRITE);
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

String FileStorageService::FindLatestLogFile()
{
    String latestName;

    for (const auto &name : ListAllFilesRecursive())
    {
        if (name.indexOf("/log_") >= 0 && name.endsWith(".csv") && name.indexOf("/ml_") < 0)
        {
            if (latestName.length() == 0 || name > latestName)
            {
                latestName = name;
            }
        }
    }

    return latestName;
}

void FileStorageService::DumpFileToSerial(const String &filePath)
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

String FileStorageService::GetContentType(const String &path) const
{
    if (path.endsWith(".csv"))
        return "text/csv";
    if (path.endsWith(".txt"))
        return "text/plain";
    if (path.endsWith(".html"))
        return "text/html";
    if (path.endsWith(".json"))
        return "application/json";

    return "application/octet-stream";
}

bool FileStorageService::IsSafePath(const String &path) const
{
    if (path.length() == 0)
    {
        return false;
    }

    return path.indexOf("..") < 0;
}

String FileStorageService::NormalizeFilePath(const String &path) const
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

bool FileStorageService::IsViewableTextFile(const String &path) const
{
    return path.endsWith(".csv") ||
           path.endsWith(".txt") ||
           path.endsWith(".json") ||
           path.endsWith(".html");
}

bool FileStorageService::IsImportantFile(const String &path) const
{
    const String normalized = EnsureLeadingSlash(path);
    return normalized == plantSettingsFile_ || normalized == lastStateFile_;
}

bool FileStorageService::IsLogFile(const String &path) const
{
    const String normalized = EnsureLeadingSlash(path);
    return normalized.endsWith(".csv");
}

size_t FileStorageService::CountImportantFiles(const char *plantSettingsFile, const char *lastStateFile)
{
    SetImportantFiles(plantSettingsFile, lastStateFile);

    size_t count = 0;

    for (const auto &fileName : ListAllFilesRecursive())
    {
        if (IsImportantFile(fileName))
        {
            ++count;
        }
    }

    return count;
}

size_t FileStorageService::CountLogFiles()
{
    size_t count = 0;

    for (const auto &fileName : ListAllFilesRecursive())
    {
        if (IsLogFile(fileName))
        {
            ++count;
        }
    }

    return count;
}

String FileStorageService::BuildFilteredFilesHtmlTable(bool (FileStorageService::*filterFn)(const String &) const,
                                                       const char *plantSettingsFile,
                                                       const char *lastStateFile)
{
    SetImportantFiles(plantSettingsFile, lastStateFile);

    String html;
    html.reserve(5000);

    bool foundAny = false;
    const std::vector<String> allFiles = ListAllFilesRecursive();

    html += "<table border='1' cellpadding='6' cellspacing='0' style='border-collapse:collapse; width:100%; max-width:900px;'>";
    html += "<thead><tr><th align='left'>File</th><th align='left'>Size (bytes)</th><th align='left'>Actions</th></tr></thead><tbody>";

    for (const auto &name : allFiles)
    {
        if ((this->*filterFn)(name))
        {
            foundAny = true;

            String normalizedName = name;
            if (normalizedName.startsWith("/"))
            {
                normalizedName = normalizedName.substring(1);
            }

            html += "<tr><td><code>" + name + "</code></td>";

            File file = LittleFS.open(EnsureAbsolutePath(name), FILE_READ);
            html += "<td>" + String(file ? file.size() : 0) + "</td><td>";
            if (file)
            {
                file.close();
            }

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

            html += "</td></tr>";
        }
    }

    html += "</tbody></table>";

    if (!foundAny)
    {
        return "<p>No matching files found.</p>";
    }

    return html;
}

String FileStorageService::BuildFolderBrowserHtmlTable(const String &currentDir,
                                                       bool (FileStorageService::*filterFn)(const String &) const,
                                                       const char *plantSettingsFile,
                                                       const char *lastStateFile)
{
    SetImportantFiles(plantSettingsFile, lastStateFile);

    String dirPath = NormalizeFilePath(currentDir);
    if (dirPath.length() == 0)
    {
        dirPath = "/logs";
    }

    if (!IsSafePath(dirPath) || !dirPath.startsWith("/logs"))
    {
        return "<p>Invalid folder path.</p>";
    }

    File dir = LittleFS.open(dirPath, FILE_READ);
    if (!dir || !dir.isDirectory())
    {
        return "<p>Folder not found: <code>" + dirPath + "</code></p>";
    }

    String html;
    html.reserve(7000);

    html += "<p>Current folder: <code>" + dirPath + "</code></p>";

    if (dirPath != "/logs")
    {
        const int lastSlash = dirPath.lastIndexOf('/');
        String parent = lastSlash <= 0 ? "/logs" : dirPath.substring(0, lastSlash);

        if (parent.length() == 0 || parent == "/" || !parent.startsWith("/logs"))
        {
            parent = "/logs";
        }

        html += "<p><a href='/?dir=" + parent + "'>Back</a></p>";
    }

    html += "<table border='1' cellpadding='6' cellspacing='0' style='border-collapse:collapse; width:100%; max-width:900px;'>";
    html += "<thead><tr><th align='left'>Name</th><th align='left'>Type</th><th align='left'>Size</th><th align='left'>Actions</th></tr></thead><tbody>";

    bool foundAny = false;

    File entry = dir.openNextFile();
    while (entry)
    {
        const String entryName = EntryNameOnly(String(entry.name()));
        const String fullPath = JoinPath(dirPath, entryName);

        if (entry.isDirectory())
        {
            foundAny = true;

            html += "<tr>";
            html += "<td><code>" + entryName + "</code></td>";
            html += "<td>Folder</td>";
            html += "<td>-</td>";
            html += "<td>";
            html += "<a href='/?dir=" + fullPath + "'>Open</a>";

            if (fullPath != "/logs")
            {
                html += " | <form method='POST' action='/delete-folder' style='display:inline; margin:0;'>";
                html += "<input type='hidden' name='folder' value='" + fullPath + "'>";
                html += "<button type='submit' onclick='return confirm(\"Delete this folder and ALL contents?\")'>Delete folder</button>";
                html += "</form>";
            }

            html += "</td>";
            html += "</tr>";
        }
        else if ((this->*filterFn)(fullPath))
        {
            foundAny = true;

            String normalizedName = fullPath;
            if (normalizedName.startsWith("/"))
            {
                normalizedName = normalizedName.substring(1);
            }

            html += "<tr>";
            html += "<td><code>" + entryName + "</code></td>";
            html += "<td>File</td>";
            html += "<td>" + String(entry.size()) + "</td>";
            html += "<td>";

            if (IsViewableTextFile(fullPath))
            {
                html += "<a href='/view?file=" + normalizedName + "'>View</a> | ";
            }

            html += "<a href='/download?file=" + normalizedName + "'>Download</a>";

            if (!IsImportantFile(fullPath))
            {
                html += " | <form method='POST' action='/delete' style='display:inline; margin:0;'>";
                html += "<input type='hidden' name='file' value='" + normalizedName + "'>";
                html += "<button type='submit' onclick='return confirm(\"Delete this file?\")'>Delete</button>";
                html += "</form>";
            }

            html += "</td></tr>";
        }

        entry.close();
        entry = dir.openNextFile();
    }

    dir.close();

    html += "</tbody></table>";

    if (!foundAny)
    {
        html += "<p>No matching files found in this folder.</p>";
    }

    return html;
}

bool FileStorageService::DeleteFolderRecursive(const String &folderPath)
{
    const String dirPath = NormalizeFilePath(folderPath);

    if (!IsSafePath(dirPath) || !dirPath.startsWith("/logs") || dirPath == "/logs")
    {
        Serial.print("Refusing to delete folder: ");
        Serial.println(dirPath);
        return false;
    }

    File dir = LittleFS.open(dirPath, FILE_READ);
    if (!dir || !dir.isDirectory())
    {
        Serial.print("Folder not found or not directory: ");
        Serial.println(dirPath);
        return false;
    }

    File entry = dir.openNextFile();
    while (entry)
    {
        const String entryName = EntryNameOnly(String(entry.name()));
        const String fullPath = JoinPath(dirPath, entryName);

        if (entry.isDirectory())
        {
            entry.close();

            if (!DeleteFolderRecursive(fullPath))
            {
                dir.close();
                return false;
            }
        }
        else
        {
            entry.close();

            if (!LittleFS.remove(fullPath))
            {
                Serial.print("Failed to delete file: ");
                Serial.println(fullPath);
                dir.close();
                return false;
            }

            Serial.print("Deleted file: ");
            Serial.println(fullPath);
        }

        entry = dir.openNextFile();
    }

    dir.close();

    if (!LittleFS.rmdir(dirPath))
    {
        Serial.print("Failed to remove folder: ");
        Serial.println(dirPath);
        return false;
    }

    Serial.print("Deleted folder: ");
    Serial.println(dirPath);
    return true;
}

void FileStorageService::SetImportantFiles(const char *plantSettingsFile, const char *lastStateFile)
{
    plantSettingsFile_ = plantSettingsFile;
    lastStateFile_ = lastStateFile;
}

String FileStorageService::PreferenceToString(PreferenceBand band) const
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

String FileStorageService::EnsureLeadingSlash(const String &path) const
{
    if (path.startsWith("/"))
    {
        return path;
    }

    return "/" + path;
}

String FileStorageService::GetMonthlyDirectoryPath(std::int64_t unixTime) const
{
    const std::time_t ts = static_cast<std::time_t>(unixTime);
    std::tm timeInfo{};
    localtime_r(&ts, &timeInfo);

    char buffer[24];
    if (std::strftime(buffer, sizeof(buffer), "/logs/%Y-%m", &timeInfo) == 0)
    {
        return "/logs/unknown";
    }

    return String(buffer);
}

String FileStorageService::GetDailyLogFilePath(std::int64_t unixTime) const
{
    const String monthDir = GetMonthlyDirectoryPath(unixTime);

    const std::time_t ts = static_cast<std::time_t>(unixTime);
    std::tm timeInfo{};
    localtime_r(&ts, &timeInfo);

    char buffer[32];
    if (std::strftime(buffer, sizeof(buffer), "log_%Y-%m-%d.csv", &timeInfo) == 0)
    {
        return monthDir + "/log_unknown_date.csv";
    }

    return monthDir + "/" + String(buffer);
}

String FileStorageService::GetMlLogFilePath(std::int64_t unixTime) const
{
    const String monthDir = GetMonthlyDirectoryPath(unixTime);

    const std::time_t ts = static_cast<std::time_t>(unixTime);
    std::tm timeInfo{};
    localtime_r(&ts, &timeInfo);

    char buffer[32];
    if (std::strftime(buffer, sizeof(buffer), "ml_%Y-%m-%d.csv", &timeInfo) == 0)
    {
        return monthDir + "/ml_unknown_date.csv";
    }

    return monthDir + "/" + String(buffer);
}

bool FileStorageService::EnsureDirectoryExists(const String &dirPath) const
{
    if (LittleFS.exists(dirPath))
    {
        return true;
    }

    Serial.print("Creating directory: ");
    Serial.println(dirPath);

    if (!LittleFS.mkdir(dirPath))
    {
        Serial.print("Failed to create directory: ");
        Serial.println(dirPath);
        return false;
    }

    return true;
}

String FileStorageService::CsvEscape(const String &input) const
{
    String escaped = input;
    escaped.replace("\"", "\"\"");
    return "\"" + escaped + "\"";
}

bool FileStorageService::EnsureCsvHeader(const String &filePath)
{
    if (LittleFS.exists(filePath))
    {
        return true;
    }

    const int slashIndex = filePath.lastIndexOf('/');
    if (slashIndex > 0)
    {
        const String dirPath = filePath.substring(0, slashIndex);
        const int parentSlash = dirPath.lastIndexOf('/');

        if (parentSlash > 0)
        {
            const String parentDir = dirPath.substring(0, parentSlash);

            if (!EnsureDirectoryExists(parentDir))
            {
                return false;
            }
        }

        if (!EnsureDirectoryExists(dirPath))
        {
            return false;
        }
    }

    File file = LittleFS.open(filePath, FILE_WRITE);
    if (!file)
    {
        Serial.print("Failed to create CSV file: ");
        Serial.println(filePath);
        return false;
    }

    if (filePath.indexOf("/ml_") >= 0)
    {
        file.println("plant_label,timestamp_utc,risk_class,confidence,prob_healthy,prob_moderate_stress,prob_high_stress");
    }
    else
    {
        file.println("plant_label,device_id,timestamp_utc,soil_moisture_pct,temperature_c,humidity_pct,light_level_pct,action_water,action_reduce_temp,action_increase_light,recommendation_summary");
    }

    file.close();

    Serial.print("Created CSV with header: ");
    Serial.println(filePath);
    return true;
}
