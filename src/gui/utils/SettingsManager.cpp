#include "SettingsManager.hpp"
#include <wx/stdpaths.h>
#include <memory>

SettingsManager& SettingsManager::Instance() {
    static SettingsManager instance;
    return instance;
}

SettingsManager::SettingsManager() {
    // Create config with application name - wxConfig handles platform-specific paths
    // Linux: ~/.config/garmin-disconnect/
    // Windows: HKEY_CURRENT_USER\Software\garmin-disconnect\ (registry)
    // or %APPDATA%\garmin-disconnect\ (file-based)
    m_config = std::make_unique<wxConfig>("garmin-disconnect");
}

SettingsManager::~SettingsManager() = default;

void SettingsManager::SetLastDirectory(const wxString& path) {
    m_config->Write("/Directory/LastOpened", path);
    m_config->Flush();
}

wxString SettingsManager::GetLastDirectory() const {
    return m_config->Read("/Directory/LastOpened", wxEmptyString);
}

void SettingsManager::SetWindowPosition(int x, int y) {
    m_config->Write("/Window/X", x);
    m_config->Write("/Window/Y", y);
    m_config->Flush();
}

void SettingsManager::GetWindowPosition(int& x, int& y) const {
    x = m_config->ReadLong("/Window/X", -1);
    y = m_config->ReadLong("/Window/Y", -1);
}

void SettingsManager::SetWindowSize(int width, int height) {
    m_config->Write("/Window/Width", width);
    m_config->Write("/Window/Height", height);
    m_config->Flush();
}

void SettingsManager::GetWindowSize(int& width, int& height) const {
    width = m_config->ReadLong("/Window/Width", 1200);
    height = m_config->ReadLong("/Window/Height", 800);
}

void SettingsManager::SetLastViewMode(int mode) {
    m_config->Write("/View/Mode", mode);
    m_config->Flush();
}

int SettingsManager::GetLastViewMode() const {
    return m_config->ReadLong("/View/Mode", 0); // Default to folder view
}

void SettingsManager::SetLastTimeGranularity(int granularity) {
    m_config->Write("/View/TimeGranularity", granularity);
    m_config->Flush();
}

int SettingsManager::GetLastTimeGranularity() const {
    return m_config->ReadLong("/View/TimeGranularity", 2); // Default to Day
}

void SettingsManager::SetDataDirectory(const wxString& path) {
    m_config->Write("/DataDirectory/Path", path);
    m_config->Flush();
}

wxString SettingsManager::GetDataDirectory() const {
    return m_config->Read("/DataDirectory/Path", wxEmptyString);
}

void SettingsManager::SetInt(const wxString& key, int value) {
    m_config->Write(key, value);
    m_config->Flush();
}

int SettingsManager::GetInt(const wxString& key, int defaultValue) const {
    return m_config->ReadLong(key, defaultValue);
}

void SettingsManager::SetString(const wxString& key, const wxString& value) {
    m_config->Write(key, value);
    m_config->Flush();
}

wxString SettingsManager::GetString(const wxString& key, const wxString& defaultValue) const {
    return m_config->Read(key, defaultValue);
}