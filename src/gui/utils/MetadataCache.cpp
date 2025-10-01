#include "MetadataCache.hpp"
#include <wx/textfile.h>
#include <wx/wfstream.h>
#include <fstream>
#include <sstream>

wxString MetadataCache::GetMetaFilePath(const wxString& fitFilePath) {
    wxFileName fn(fitFilePath);
    fn.SetExt("meta");
    return fn.GetFullPath();
}

uint16_t MetadataCache::GetFitChecksum(const wxString& fitFilePath) {
    std::ifstream file(fitFilePath.ToStdString(), std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        return 0;
    }

    std::streamsize size = file.tellg();
    if (size < 2) {
        return 0;
    }

    // Read last 2 bytes (CRC is stored at the end of FIT files)
    file.seekg(size - 2, std::ios::beg);
    uint8_t crcLow = 0, crcHigh = 0;
    file.read(reinterpret_cast<char*>(&crcLow), 1);
    file.read(reinterpret_cast<char*>(&crcHigh), 1);
    file.close();

    return static_cast<uint16_t>(crcLow) | (static_cast<uint16_t>(crcHigh) << 8);
}

bool MetadataCache::IsCacheValid(const wxString& fitFilePath) {
    wxString metaPath = GetMetaFilePath(fitFilePath);

    // Check if meta file exists
    if (!wxFileExists(metaPath)) {
        return false;
    }

    // Read checksum from meta file
    auto metaData = ReadKeyValueFile(metaPath);
    if (metaData.find("checksum") == metaData.end()) {
        return false;
    }

    // Get checksum from FIT file
    uint16_t fitChecksum = GetFitChecksum(fitFilePath);
    if (fitChecksum == 0) {
        return false;
    }

    // Compare checksums
    unsigned long metaChecksum = 0;
    metaData["checksum"].ToULong(&metaChecksum);

    return static_cast<uint16_t>(metaChecksum) == fitChecksum;
}

bool MetadataCache::LoadFromCache(const wxString& fitFilePath, ActivityDisplayData& data) {
    wxString metaPath = GetMetaFilePath(fitFilePath);

    if (!wxFileExists(metaPath)) {
        return false;
    }

    auto metaData = ReadKeyValueFile(metaPath);

    // Verify we have all required fields
    if (metaData.find("checksum") == metaData.end()) {
        return false;
    }

    // Populate ActivityDisplayData from cache
    data.name = metaData["name"];
    data.sport = metaData["sport"];
    data.duration = metaData["duration"];
    data.distance = metaData["distance"];
    data.speedPace = metaData["speed_pace"];
    data.heartRate = metaData["heart_rate"];
    data.date = metaData["date"];

    // Parse timestamp
    unsigned long ts = 0;
    metaData["timestamp"].ToULong(&ts);
    data.timestamp = static_cast<uint32_t>(ts);

    return true;
}

bool MetadataCache::SaveToCache(const wxString& fitFilePath, const ActivityDisplayData& data, bool preserveName) {
    wxString metaPath = GetMetaFilePath(fitFilePath);

    std::map<wxString, wxString> metaData;

    // Get FIT file checksum
    uint16_t checksum = GetFitChecksum(fitFilePath);
    if (checksum == 0) {
        return false;
    }

    // If preserving name, load existing meta file first
    wxString existingName;
    if (preserveName && wxFileExists(metaPath)) {
        auto existingData = ReadKeyValueFile(metaPath);
        if (existingData.find("name") != existingData.end()) {
            existingName = existingData["name"];
        }
    }

    // Build metadata
    metaData["checksum"] = wxString::Format("%u", checksum);
    metaData["name"] = preserveName && !existingName.IsEmpty() ? existingName : data.name;
    metaData["sport"] = data.sport;
    metaData["timestamp"] = wxString::Format("%u", data.timestamp);
    metaData["date"] = data.date;
    metaData["duration"] = data.duration;
    metaData["distance"] = data.distance;
    metaData["speed_pace"] = data.speedPace;
    metaData["heart_rate"] = data.heartRate;

    return WriteKeyValueFile(metaPath, metaData);
}

std::map<wxString, wxString> MetadataCache::ReadKeyValueFile(const wxString& filePath) {
    std::map<wxString, wxString> result;

    wxTextFile file(filePath);
    if (!file.Open(wxConvUTF8)) {
        return result;
    }

    for (size_t i = 0; i < file.GetLineCount(); i++) {
        wxString line = file.GetLine(i).Trim(true).Trim(false);

        // Skip empty lines and comments
        if (line.IsEmpty() || line.StartsWith("#")) {
            continue;
        }

        // Parse key=value
        int equalPos = line.Find('=');
        if (equalPos != wxNOT_FOUND) {
            wxString key = line.Left(equalPos).Trim(true).Trim(false);
            wxString value = line.Mid(equalPos + 1).Trim(true).Trim(false);
            result[key] = value;
        }
    }

    file.Close();
    return result;
}

bool MetadataCache::WriteKeyValueFile(const wxString& filePath, const std::map<wxString, wxString>& data) {
    wxTextFile file;

    // Remove existing file if it exists
    if (wxFileExists(filePath)) {
        if (!file.Open(filePath, wxConvUTF8)) {
            return false;
        }
        file.Clear();
    } else {
        if (!file.Create(filePath)) {
            return false;
        }
    }

    // Write header comment
    file.AddLine("# Garmin FIT Activity Metadata Cache");
    file.AddLine("# This file is auto-generated but human-editable");
    file.AddLine("");

    // Write key=value pairs in a predictable order
    const wxString orderedKeys[] = {
        "checksum", "name", "sport", "timestamp", "date",
        "duration", "distance", "speed_pace", "heart_rate"
    };

    for (const auto& key : orderedKeys) {
        auto it = data.find(key);
        if (it != data.end()) {
            file.AddLine(key + "=" + it->second);
        }
    }

    // Write any additional keys not in the ordered list
    for (const auto& pair : data) {
        bool isOrdered = false;
        for (const auto& orderedKey : orderedKeys) {
            if (pair.first == orderedKey) {
                isOrdered = true;
                break;
            }
        }
        if (!isOrdered) {
            file.AddLine(pair.first + "=" + pair.second);
        }
    }

    bool success = file.Write(wxTextFileType_Unix, wxConvUTF8);
    file.Close();

    return success;
}
