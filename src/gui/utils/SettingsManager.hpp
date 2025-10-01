#pragma once

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
    #include <wx/wx.h>
#endif
#include <wx/config.h>
#include <memory>

class SettingsManager {
public:
    static SettingsManager& Instance();
    
    // Directory settings
    void SetLastDirectory(const wxString& path);
    wxString GetLastDirectory() const;
    
    // Window settings
    void SetWindowPosition(int x, int y);
    void GetWindowPosition(int& x, int& y) const;
    void SetWindowSize(int width, int height);
    void GetWindowSize(int& width, int& height) const;
    
    // View settings
    void SetLastViewMode(int mode);
    int GetLastViewMode() const;
    void SetLastTimeGranularity(int granularity);
    int GetLastTimeGranularity() const;

    // Data directory settings
    void SetDataDirectory(const wxString& path);
    wxString GetDataDirectory() const;

    // Generic settings methods
    void SetInt(const wxString& key, int value);
    int GetInt(const wxString& key, int defaultValue = 0) const;
    void SetString(const wxString& key, const wxString& value);
    wxString GetString(const wxString& key, const wxString& defaultValue = wxEmptyString) const;
    
private:
    SettingsManager();
    ~SettingsManager();
    SettingsManager(const SettingsManager&) = delete;
    SettingsManager& operator=(const SettingsManager&) = delete;
    
    std::unique_ptr<wxConfig> m_config;
};