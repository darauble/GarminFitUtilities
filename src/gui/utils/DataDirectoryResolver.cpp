#include "DataDirectoryResolver.hpp"
#include "SettingsManager.hpp"
#include <wx/filename.h>
#include <wx/dir.h>

wxString DataDirectoryResolver::GetDataDirectory() {
    auto searchPaths = GetSearchPaths();

    for (const auto& path : searchPaths) {
        if (IsValidDataDirectory(path)) {
            return path;
        }
    }

    return wxEmptyString;
}

wxString DataDirectoryResolver::FindDataFile(const wxString& relativePath) {
    wxString dataDir = GetDataDirectory();
    if (dataDir.IsEmpty()) {
        return wxEmptyString;
    }

    wxString fullPath = dataDir + wxFileName::GetPathSeparator() + relativePath;
    if (wxFileExists(fullPath)) {
        return fullPath;
    }

    return wxEmptyString;
}

std::vector<wxString> DataDirectoryResolver::GetSearchPaths() {
    std::vector<wxString> paths;

    // 1. User-configured path from settings (highest priority)
    wxString userPath = GetUserConfiguredPath();
    if (!userPath.IsEmpty()) {
        paths.push_back(userPath);
    }

    // Get standard paths once
    wxStandardPaths& stdPaths = wxStandardPaths::Get();

    // 2. Platform-specific standard locations
#ifdef __WXMSW__
    // Windows: %PROGRAMDATA%\garmin-disconnect\data
    wxString programData = wxGetenv("PROGRAMDATA");
    if (!programData.IsEmpty()) {
        paths.push_back(programData + "\\garmin-disconnect\\data");
    }
    // Windows: Application directory
    paths.push_back(stdPaths.GetDataDir() + "\\data");
#else
    // Linux/Unix: /usr/share/garmin-disconnect/data
    paths.push_back("/usr/share/garmin-disconnect/data");
    paths.push_back("/usr/local/share/garmin-disconnect/data");

    // User local installation: ~/.local/share/garmin-disconnect/data
    paths.push_back(stdPaths.GetUserDataDir() + "/data");
#endif

    // 3. Development fallback: ../data relative to executable
    wxString execPath = stdPaths.GetExecutablePath();
    wxFileName execFile(execPath);
    wxString execDir = execFile.GetPath();

    // Try ../data from executable location
    wxFileName devPath(execDir, wxEmptyString);
    devPath.AppendDir("..");
    devPath.AppendDir("data");
    paths.push_back(devPath.GetFullPath());

    // Try ./data from executable location
    paths.push_back(execDir + wxFileName::GetPathSeparator() + "data");

    return paths;
}

bool DataDirectoryResolver::IsValidDataDirectory(const wxString& path) {
    if (path.IsEmpty() || !wxDirExists(path)) {
        return false;
    }

    // Check for expected files to validate it's the correct data directory
    // Look for osmconf.ini as a marker file
    wxString markerFile = path + wxFileName::GetPathSeparator() + "osmconf.ini";
    return wxFileExists(markerFile);
}

wxString DataDirectoryResolver::GetUserConfiguredPath() {
    SettingsManager& settings = SettingsManager::Instance();
    wxString configPath = settings.GetString("data_directory", wxEmptyString);

    if (!configPath.IsEmpty() && wxDirExists(configPath)) {
        return configPath;
    }

    return wxEmptyString;
}
