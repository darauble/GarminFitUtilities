#pragma once

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
    #include <wx/wx.h>
#endif
#include <wx/filename.h>
#include <cstdint>
#include <map>
#include "../models/ActivityData.hpp"

/**
 * MetadataCache manages sidecar .meta files for FIT files.
 *
 * These files cache parsed activity data in human-editable key=value format.
 * The cache is validated using the FIT file's checksum to detect changes.
 * When checksums match, data is loaded from cache (fast). When they differ,
 * the FIT file is re-parsed and the cache is updated, but user-edited fields
 * (like 'name') are preserved.
 *
 * Sidecar file format (.meta):
 *   checksum=12345
 *   name=My Custom Activity Name
 *   sport=Running
 *   timestamp=1234567890
 *   duration=45:32
 *   distance=8.50 km
 *   speed_pace=5:21 /km
 *   heart_rate=145
 */
class MetadataCache {
public:
    /**
     * Check if cache is valid for given FIT file.
     * @param fitFilePath Path to the FIT file
     * @return true if .meta file exists and checksum matches
     */
    static bool IsCacheValid(const wxString& fitFilePath);

    /**
     * Load activity display data from cache file.
     * @param fitFilePath Path to the FIT file
     * @param data Output parameter for activity data
     * @return true if successfully loaded from cache
     */
    static bool LoadFromCache(const wxString& fitFilePath, ActivityDisplayData& data);

    /**
     * Save activity display data to cache file.
     * @param fitFilePath Path to the FIT file
     * @param data Activity data to save
     * @param preserveName If true, preserve existing name field from cache
     * @return true if successfully saved
     */
    static bool SaveToCache(const wxString& fitFilePath, const ActivityDisplayData& data, bool preserveName = false);

    /**
     * Get the checksum from a FIT file (last 2 bytes).
     * @param fitFilePath Path to the FIT file
     * @return Checksum value, or 0 if error
     */
    static uint16_t GetFitChecksum(const wxString& fitFilePath);

    /**
     * Get the path to the .meta sidecar file for a given FIT file.
     * @param fitFilePath Path to the FIT file
     * @return Path to .meta file
     */
    static wxString GetMetaFilePath(const wxString& fitFilePath);

private:
    /**
     * Read key=value pairs from a file.
     * @param filePath Path to the file
     * @return Map of key-value pairs
     */
    static std::map<wxString, wxString> ReadKeyValueFile(const wxString& filePath);

    /**
     * Write key=value pairs to a file.
     * @param filePath Path to the file
     * @param data Map of key-value pairs
     * @return true if successfully written
     */
    static bool WriteKeyValueFile(const wxString& filePath, const std::map<wxString, wxString>& data);
};
