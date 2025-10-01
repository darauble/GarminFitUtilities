#pragma once

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
    #include <wx/wx.h>
#endif
#include <wx/notebook.h>
#include <wx/splitter.h>
#include <memory>
#include <unordered_map>
#include "utils/SettingsManager.hpp"
#include "interfaces/IActivityPanel.hpp"

// Forward declarations for panels
class ActivitiesPanel;
class ActivityDetailsPanel;
class FileTreePanel;
class ProductEditorPanel;
class TimestampEditorPanel;
class GpxEditorPanel;
class RawEditorPanel;
class PointsPanel;
class RenamePanel;
class MapPanel;

class MainFrame : public wxFrame {
public:
    MainFrame();
    virtual ~MainFrame();

private:
    enum {
        ID_Exit = wxID_EXIT,
        ID_About = wxID_ABOUT,
        ID_Open = wxID_OPEN,
        ID_Settings = 1001,
        ID_Refresh = 1002,
        ID_TreeSelection = 2000,
        ID_PreviousActivity = 2001,
        ID_NextActivity = 2002,
        ID_ActivitySelected = 2003,
        ID_TabChanged = 2004
    };

    // Menu event handlers
    void OnExit(wxCommandEvent& event);
    void OnAbout(wxCommandEvent& event);
    void OnOpen(wxCommandEvent& event);
    void OnSettings(wxCommandEvent& event);
    void OnRefresh(wxCommandEvent& event);
    void OnClose(wxCloseEvent& event);
    void OnTreeSelection(wxCommandEvent& event);
    void OnPreviousActivity(wxCommandEvent& event);
    void OnNextActivity(wxCommandEvent& event);
    void OnActivitySelected(wxCommandEvent& event);
    void OnTabChanged(wxBookCtrlEvent& event);

    // Window layout
    void CreateMenuBar();
    void CreateStatusBar();
    void CreateMainLayout();

    // Main UI components
    wxNotebook* m_notebook;
    wxSplitterWindow* m_splitter;
    wxButton* m_prevButton;
    wxButton* m_nextButton;
    
    // Panels
    std::unique_ptr<ActivitiesPanel> m_activitiesPanel;
    std::unique_ptr<ActivityDetailsPanel> m_activityDetailsPanel;
    std::unique_ptr<FileTreePanel> m_fileTreePanel;
    std::unique_ptr<ProductEditorPanel> m_productEditorPanel;
    std::unique_ptr<TimestampEditorPanel> m_timestampEditorPanel;
    std::unique_ptr<GpxEditorPanel> m_gpxEditorPanel;
    std::unique_ptr<RawEditorPanel> m_rawEditorPanel;
    std::unique_ptr<PointsPanel> m_pointsPanel;
    std::unique_ptr<RenamePanel> m_renamePanel;
    std::unique_ptr<MapPanel> m_mapPanel;

    // Application state
    wxString m_currentDirectory;
    bool m_dataLoaded;
    int m_selectedActivityIndex; // -1 if no selection
    int m_currentTabIndex; // Currently active tab index

    // Map of wxWindow* -> IActivityPanel* for panels that support tab activation
    std::unordered_map<wxWindow*, IActivityPanel*> m_activityPanels;
    
    // Internal methods
    void LoadDirectory(const wxString& path);
    void RefreshAllPanels();
    void LoadSettings();
    void SaveSettings();
    void UpdateActivitySelection();
    void NotifyTabActivation(int tabIndex, const std::string& activityFilePath);
    std::string GetCurrentActivityFilePath() const;

    wxDECLARE_EVENT_TABLE();
};