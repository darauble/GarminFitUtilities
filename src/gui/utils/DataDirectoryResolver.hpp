#pragma once

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
    #include <wx/wx.h>
#endif
#include <wx/stdpaths.h>
#include <vector>
#include <string>

/**
 * Resolves the data directory for application resources.
 *
 * Priority order:
 * 1. User-configured data_directory from settings (~/.config/garmin-disconnect)
 * 2. Standard system locations:
 *    - Linux: /usr/share/garmin-disconnect, /usr/local/share/garmin-disconnect
 *    - Windows: %PROGRAMDATA%/garmin-disconnect
 * 3. Development fallback: ../data relative to executable
 */
class DataDirectoryResolver {
public:
    /**
     * Get the data directory path.
     * @return Full path to data directory, or empty string if not found
     */
    static wxString GetDataDirectory();

    /**
     * Find a specific file in the data directory.
     * @param relativePath Path relative to data directory (e.g., "osmconf.ini" or "map-style/osm-spatialite.xml")
     * @return Full path to file, or empty string if not found
     */
    static wxString FindDataFile(const wxString& relativePath);

    /**
     * Get all potential data directory search paths.
     * @return Vector of search paths in priority order
     */
    static std::vector<wxString> GetSearchPaths();

private:
    /**
     * Check if directory exists and contains expected data.
     * @param path Directory to check
     * @return true if valid data directory
     */
    static bool IsValidDataDirectory(const wxString& path);

    /**
     * Get user-configured data directory from settings.
     * @return Configured path or empty string
     */
    static wxString GetUserConfiguredPath();
};
