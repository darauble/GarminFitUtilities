#include "MainFrame.hpp"
#include "version.hpp"
#include "panels/ActivitiesPanel.hpp"
#include "panels/ActivityDetailsPanel.hpp"
#include "panels/FileTreePanel.hpp"
#include "panels/ProductEditorPanel.hpp"
#include "panels/TimestampEditorPanel.hpp"
#include "panels/GpxEditorPanel.hpp"
#include "panels/RawEditorPanel.hpp"
#include "panels/PointsPanel.hpp"
#include "panels/RenamePanel.hpp"
#include "panels/MapPanel.hpp"
#include "dialogs/SettingsDialog.hpp"
#include "icon/garmin-disconnect-icon.h"
#include <wx/stream.h>
#include <wx/mstream.h>

wxBEGIN_EVENT_TABLE(MainFrame, wxFrame)
    EVT_MENU(ID_Exit, MainFrame::OnExit)
    EVT_MENU(ID_About, MainFrame::OnAbout)
    EVT_MENU(ID_Open, MainFrame::OnOpen)
    EVT_MENU(ID_Settings, MainFrame::OnSettings)
    EVT_MENU(ID_Refresh, MainFrame::OnRefresh)
    EVT_MENU(ID_TreeSelection, MainFrame::OnTreeSelection)
    EVT_MENU(ID_ActivitySelected, MainFrame::OnActivitySelected)
    EVT_BUTTON(ID_PreviousActivity, MainFrame::OnPreviousActivity)
    EVT_BUTTON(ID_NextActivity, MainFrame::OnNextActivity)
    EVT_NOTEBOOK_PAGE_CHANGED(wxID_ANY, MainFrame::OnTabChanged)
    EVT_CLOSE(MainFrame::OnClose)
wxEND_EVENT_TABLE()

MainFrame::MainFrame()
    : wxFrame(nullptr, wxID_ANY,
              wxString::Format("%s v%s", GARMIN_FIT_UTILITIES_APP_NAME, GARMIN_FIT_UTILITIES_VERSION_STRING),
              wxDefaultPosition, wxSize(1200, 800)),
      m_notebook(nullptr),
      m_splitter(nullptr),
      m_prevButton(nullptr),
      m_nextButton(nullptr),
      m_dataLoaded(false),
      m_selectedActivityIndex(-1),
      m_currentTabIndex(0) {
    
    // Set minimum size
    SetMinSize(wxSize(800, 600));
    
    // Set application icon
    wxMemoryInputStream iconStream(garmin_disconnect_png, garmin_disconnect_png_len);
    wxImage iconImage(iconStream, wxBITMAP_TYPE_PNG);
    if (iconImage.IsOk()) {
        wxIcon icon;
        icon.CopyFromBitmap(wxBitmap(iconImage));
        SetIcon(icon);
    }
    
    // Create UI components
    CreateMenuBar();
    CreateStatusBar();
    CreateMainLayout();
    
    // Load settings and apply them
    LoadSettings();
    
    // Center the frame if no saved position
    int x, y;
    SettingsManager::Instance().GetWindowPosition(x, y);
    if (x == -1 || y == -1) {
        Center();
    }
}

void MainFrame::CreateMenuBar() {
    wxMenuBar* menuBar = new wxMenuBar();
    
    // File menu
    wxMenu* fileMenu = new wxMenu();
    fileMenu->Append(ID_Open, "&Open Directory...\tCtrl+O", "Open directory containing FIT files");
    fileMenu->AppendSeparator();
    fileMenu->Append(ID_Refresh, "&Refresh\tF5", "Refresh current directory");
    fileMenu->AppendSeparator();
    fileMenu->Append(ID_Exit, "E&xit\tCtrl+Q", "Quit this program");
    menuBar->Append(fileMenu, "&File");
    
    // Tools menu
    wxMenu* toolsMenu = new wxMenu();
    toolsMenu->Append(ID_Settings, "&Settings...", "Configure application settings");
    menuBar->Append(toolsMenu, "&Tools");
    
    // Help menu
    wxMenu* helpMenu = new wxMenu();
    helpMenu->Append(ID_About, "&About...\tF1", "Show about dialog");
    menuBar->Append(helpMenu, "&Help");
    
    SetMenuBar(menuBar);
}

void MainFrame::CreateStatusBar() {
    wxFrame::CreateStatusBar(2);
    SetStatusText("Ready", 0);
    SetStatusText("No directory selected", 1);
}

void MainFrame::CreateMainLayout() {
    // Create main splitter window
    m_splitter = new wxSplitterWindow(this, wxID_ANY, wxDefaultPosition, wxDefaultSize,
                                      wxSP_3D | wxSP_LIVE_UPDATE);
    
    // Create a panel to hold notebook and navigation buttons
    wxPanel* rightPanel = new wxPanel(m_splitter);
    wxBoxSizer* rightSizer = new wxBoxSizer(wxVERTICAL);
    
    // Create navigation button bar
    wxBoxSizer* navSizer = new wxBoxSizer(wxHORIZONTAL);
    m_prevButton = new wxButton(rightPanel, ID_PreviousActivity, wxString::FromUTF8("◀"), wxDefaultPosition, wxSize(50, 36));
    m_nextButton = new wxButton(rightPanel, ID_NextActivity, wxString::FromUTF8("▶"), wxDefaultPosition, wxSize(50, 36));
    
    // Set tooltips
    m_prevButton->SetToolTip("Previous activity");
    m_nextButton->SetToolTip("Next activity");
    
    // Disable buttons initially (no selection)
    m_prevButton->Enable(false);
    m_nextButton->Enable(false);
    
    navSizer->AddSpacer(5);
    navSizer->Add(m_prevButton, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 2);
    navSizer->Add(m_nextButton, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 5);
    navSizer->AddStretchSpacer();
    
    // Create notebook for tabs
    m_notebook = new wxNotebook(rightPanel, wxID_ANY);
    
    // Create panels
    m_activitiesPanel = std::make_unique<ActivitiesPanel>(m_notebook);
    m_activityDetailsPanel = std::make_unique<ActivityDetailsPanel>(m_notebook);
    m_fileTreePanel = std::make_unique<FileTreePanel>(m_splitter);
    m_productEditorPanel = std::make_unique<ProductEditorPanel>(m_notebook);
    m_timestampEditorPanel = std::make_unique<TimestampEditorPanel>(m_notebook);
    m_gpxEditorPanel = std::make_unique<GpxEditorPanel>(m_notebook);
    m_rawEditorPanel = std::make_unique<RawEditorPanel>(m_notebook);
    m_pointsPanel = std::make_unique<PointsPanel>(m_notebook);
    m_renamePanel = std::make_unique<RenamePanel>(m_notebook);
    m_mapPanel = std::make_unique<MapPanel>(m_notebook);
    
    // Add pages to notebook
    m_notebook->AddPage(m_activitiesPanel.get(), "Activities", true);
    m_notebook->AddPage(m_activityDetailsPanel.get(), "Details");
    m_notebook->AddPage(m_mapPanel.get(), "Map");
    m_notebook->AddPage(m_productEditorPanel.get(), "Product Editor");
    m_notebook->AddPage(m_timestampEditorPanel.get(), "Timestamp Editor");
    m_notebook->AddPage(m_gpxEditorPanel.get(), "GPX Editor");
    m_notebook->AddPage(m_rawEditorPanel.get(), "Raw Editor");
    m_notebook->AddPage(m_pointsPanel.get(), "Points & Mapping");
    m_notebook->AddPage(m_renamePanel.get(), "File Rename");
    

    // Register panels that implement IActivityPanel interface
    m_activityPanels[m_productEditorPanel.get()] = m_productEditorPanel.get();
    m_activityPanels[m_timestampEditorPanel.get()] = m_timestampEditorPanel.get();
    m_activityPanels[m_gpxEditorPanel.get()] = m_gpxEditorPanel.get();
    m_activityPanels[m_rawEditorPanel.get()] = m_rawEditorPanel.get();
    m_activityPanels[m_pointsPanel.get()] = m_pointsPanel.get();
    m_activityPanels[m_renamePanel.get()] = m_renamePanel.get();
    m_activityPanels[m_mapPanel.get()] = m_mapPanel.get();
    
    // Layout right panel
    rightSizer->Add(navSizer, 0, wxEXPAND | wxALL, 2);
    rightSizer->Add(m_notebook, 1, wxEXPAND);
    rightPanel->SetSizer(rightSizer);
    
    // Set up splitter
    m_splitter->SplitVertically(m_fileTreePanel.get(), rightPanel, 300);
    m_splitter->SetMinimumPaneSize(200);
}

void MainFrame::OnExit(wxCommandEvent& WXUNUSED(event)) {
    Close(true);
}

void MainFrame::OnAbout(wxCommandEvent& WXUNUSED(event)) {
    wxString aboutText = wxString::Format(
        "%s v%s\n\n"
        "%s\n\n"
        "Built on: %s %s\n"
        "%s\n\n"
        "Features:\n"
        "• Activity analysis and visualization\n"
        "• File editing and modification (Product ID, timestamps)\n"
        "• Points visited tracking with GPS coordinates\n"
        "• Batch file operations and renaming\n"
        "• Drag & drop file management\n"
        "• Cross-platform file manager integration\n"
        "• Context menus and keyboard shortcuts\n\n"
        "Command-line utilities included:\n"
        "• garmin-edit - File analysis and editing\n"
        "• garmin-rename-files - Batch renaming\n"
        "• garmin-points-visited - Coordinate tracking\n\n"
        "For more information visit:\n%s",
        GARMIN_FIT_UTILITIES_APP_NAME,
        GARMIN_FIT_UTILITIES_VERSION_STRING,
        GARMIN_FIT_UTILITIES_DESCRIPTION,
        GARMIN_FIT_UTILITIES_BUILD_DATE,
        GARMIN_FIT_UTILITIES_BUILD_TIME,
        GARMIN_FIT_UTILITIES_COPYRIGHT,
        GARMIN_FIT_UTILITIES_WEBSITE
    );
    
    wxMessageBox(aboutText,
                 wxString::Format("About %s", GARMIN_FIT_UTILITIES_APP_NAME),
                 wxOK | wxICON_INFORMATION);
}

void MainFrame::OnOpen(wxCommandEvent& WXUNUSED(event)) {
    wxDirDialog dialog(this, "Choose directory containing FIT files", m_currentDirectory);
    
    if (dialog.ShowModal() == wxID_OK) {
        wxString newPath = dialog.GetPath();
        LoadDirectory(newPath);
    }
}

void MainFrame::OnSettings(wxCommandEvent& WXUNUSED(event)) {
    SettingsDialog dialog(this);
    if (dialog.ShowModal() == wxID_OK) {
        // Settings were changed, update map panel with new OSM file
        if (m_mapPanel) {
            SettingsManager& settings = SettingsManager::Instance();
            std::string osmFile = settings.GetString("map_osm_file", "").ToStdString();
            m_mapPanel->SetOSMFile(osmFile);
        }
    }
}

void MainFrame::OnRefresh(wxCommandEvent& WXUNUSED(event)) {
    if (!m_currentDirectory.IsEmpty()) {
        LoadDirectory(m_currentDirectory);
    } else {
        wxMessageBox("No directory selected. Use File > Open Directory first.", 
                     "Refresh", wxOK | wxICON_INFORMATION);
    }
}

void MainFrame::LoadDirectory(const wxString& path) {
    if (path.IsEmpty()) return;
    
    SetStatusText("Loading: " + path, 1);
    SetStatusText("Scanning files...", 0);
    
    // Check if directory exists and is accessible
    if (!wxDirExists(path)) {
        wxMessageBox("Directory does not exist or is not accessible: " + path,
                     "Error", wxOK | wxICON_ERROR);
        return;
    }
    
    m_currentDirectory = path;
    
    // Save the directory setting
    SettingsManager::Instance().SetLastDirectory(path);
    
    // Update panels with new directory
    RefreshAllPanels();
    
    // Update UI state
    m_dataLoaded = true;
    SetStatusText("Ready", 0);
    SetStatusText("Directory: " + path, 1);
}

void MainFrame::RefreshAllPanels() {
    if (m_currentDirectory.IsEmpty()) return;
    
    // Update file tree panel
    if (m_fileTreePanel) {
        m_fileTreePanel->SetDirectory(m_currentDirectory);
    }
    
    // Update activities panel
    if (m_activitiesPanel) {
        m_activitiesPanel->SetDirectory(m_currentDirectory);
    }
    
    // Other panels don't need directory updates for basic functionality
}

void MainFrame::LoadSettings() {
    SettingsManager& settings = SettingsManager::Instance();
    
    // Restore window size and position
    int width, height, x, y;
    settings.GetWindowSize(width, height);
    settings.GetWindowPosition(x, y);
    
    SetSize(width, height);
    if (x != -1 && y != -1) {
        SetPosition(wxPoint(x, y));
    }
    
    // Restore last directory if it exists
    wxString lastDir = settings.GetLastDirectory();
    if (!lastDir.IsEmpty() && wxDirExists(lastDir)) {
        LoadDirectory(lastDir);
    }
    
    // Restore view mode and granularity settings
    if (m_fileTreePanel) {
        // These will be applied when the panel is fully created
        // For now, we'll implement this after the file tree panel updates
    }
}

void MainFrame::SaveSettings() {
    SettingsManager& settings = SettingsManager::Instance();
    
    // Save window size and position
    wxSize size = GetSize();
    wxPoint pos = GetPosition();
    
    settings.SetWindowSize(size.GetWidth(), size.GetHeight());
    settings.SetWindowPosition(pos.x, pos.y);
    
    // Current directory is already saved in LoadDirectory()
    // View settings will be saved by panels when they change
}

void MainFrame::OnClose(wxCloseEvent& event) {
    SaveSettings();
    event.Skip(); // Continue with default close handling
}

void MainFrame::OnTreeSelection(wxCommandEvent& event) {
    wxString selectedPath = event.GetString();
    
    // Update activities panel to filter by selected path or time
    if (m_activitiesPanel) {
        if (selectedPath == m_currentDirectory) {
            // Root directory selected - clear all filters
            m_activitiesPanel->SetFilterPath("");
            m_activitiesPanel->ClearTimeFilter();
        } else if (selectedPath.StartsWith("TIME:")) {
            // Time hierarchy selection - parse and apply time filter
            wxString timeData = selectedPath.AfterFirst(':'); // Remove "TIME:" prefix
            wxArrayString parts = wxSplit(timeData, ',');
            
            // Parse time components: YEAR,MONTH,DAY,HOUR
            int year = -1, month = -1, day = -1, hour = -1;
            
            if (parts.size() >= 1 && !parts[0].IsEmpty()) {
                long val;
                if (parts[0].ToLong(&val)) year = val;
            }
            if (parts.size() >= 2 && !parts[1].IsEmpty()) {
                long val;
                if (parts[1].ToLong(&val)) month = val;
            }
            if (parts.size() >= 3 && !parts[2].IsEmpty()) {
                long val;
                if (parts[2].ToLong(&val)) day = val;
            }
            if (parts.size() >= 4 && !parts[3].IsEmpty()) {
                long val;
                if (parts[3].ToLong(&val)) hour = val;
            }
            
            // Apply time filter
            m_activitiesPanel->SetTimeFilter(year, month, day, hour);
        } else {
            // Subdirectory selected - filter to that path
            m_activitiesPanel->ClearTimeFilter();
            m_activitiesPanel->SetFilterPath(selectedPath);
        }
    }
}

void MainFrame::OnPreviousActivity(wxCommandEvent& WXUNUSED(event)) {
    if (!m_activitiesPanel) return;
    
    // Get total number of activities
    int activityCount = m_activitiesPanel->GetActivityCount();
    if (activityCount == 0) return;
    
    if (m_selectedActivityIndex == -1) {
        // No selection, select the first item
        m_selectedActivityIndex = 0;
    } else {
        // Go to next item, wrap around to first if at end
        m_selectedActivityIndex++;
        if (m_selectedActivityIndex >= activityCount) {
            m_selectedActivityIndex = 0;
        }
    }
    
    // Update selection in activities panel and details panel
    UpdateActivitySelection();
    }

void MainFrame::OnNextActivity(wxCommandEvent& WXUNUSED(event)) {
    if (!m_activitiesPanel) return;
    
    // Get total number of activities
    int activityCount = m_activitiesPanel->GetActivityCount();
    if (activityCount == 0) return;
    
    if (m_selectedActivityIndex == -1) {
        // No selection, select the first item
        m_selectedActivityIndex = 0;
    } else {
        // Go to previous item, wrap around to last if at beginning
        m_selectedActivityIndex--;
        if (m_selectedActivityIndex < 0) {
            m_selectedActivityIndex = activityCount - 1;
        }
    }
    
    // Update selection in activities panel and details panel
    UpdateActivitySelection();
}

void MainFrame::OnActivitySelected(wxCommandEvent& event) {
    // Get the selected index from the event
    m_selectedActivityIndex = event.GetInt();

    // Update the details panel
    if (m_activityDetailsPanel) {
        ActivityDisplayData activityData;
        bool hasData = m_activitiesPanel->GetActivityData(m_selectedActivityIndex, activityData);

        if (hasData) {
            m_activityDetailsPanel->SetActivityData(activityData);

            // Notify the currently active tab about the new selection
            std::string filePath = activityData.fullPath.ToStdString();
            NotifyTabActivation(m_currentTabIndex, filePath);

            // Enable navigation buttons
            m_prevButton->Enable(true);
            m_nextButton->Enable(true);
        } else {
            m_activityDetailsPanel->ClearSelection();

            // Notify the currently active tab that there's no selection
            NotifyTabActivation(m_currentTabIndex, "");

            m_prevButton->Enable(false);
            m_nextButton->Enable(false);
        }
    }
}

void MainFrame::UpdateActivitySelection() {
    if (!m_activitiesPanel || !m_activityDetailsPanel) return;

    // Get activity data from the activities panel
    ActivityDisplayData activityData;
    bool hasData = m_activitiesPanel->GetActivityData(m_selectedActivityIndex, activityData);

    if (hasData) {
        // Update the selection in the activities list
        m_activitiesPanel->SetSelection(m_selectedActivityIndex);

        // Update the details panel
        m_activityDetailsPanel->SetActivityData(activityData);

        // Notify the currently active tab about the new selection
        std::string filePath = activityData.fullPath.ToStdString();
        NotifyTabActivation(m_currentTabIndex, filePath);

        // Enable navigation buttons
        m_prevButton->Enable(true);
        m_nextButton->Enable(true);
    } else {
        // Clear selection
        m_selectedActivityIndex = -1;
        m_activityDetailsPanel->ClearSelection();

        // Notify the currently active tab that there's no selection
        NotifyTabActivation(m_currentTabIndex, "");

        // Disable navigation buttons
        m_prevButton->Enable(false);
        m_nextButton->Enable(false);
    }
}

std::string MainFrame::GetCurrentActivityFilePath() const {
    if (m_selectedActivityIndex < 0 || !m_activitiesPanel) {
        return "";
    }

    ActivityDisplayData activityData;
    bool hasData = m_activitiesPanel->GetActivityData(m_selectedActivityIndex, activityData);

    return hasData ? activityData.fullPath.ToStdString() : "";
}

void MainFrame::NotifyTabActivation(int tabIndex, const std::string& activityFilePath) {
    // Get the panel at the specified tab index
    wxWindow* panel = m_notebook->GetPage(tabIndex);
    if (!panel) return;

    // Check if this panel implements IActivityPanel
    auto it = m_activityPanels.find(panel);
    if (it != m_activityPanels.end()) {
        it->second->OnTabActivated(activityFilePath);
    }
}

void MainFrame::OnTabChanged(wxBookCtrlEvent& event) {
    int newTabIndex = event.GetSelection();
    int oldTabIndex = m_currentTabIndex;

    // Skip if same tab (shouldn't happen, but be safe)
    if (newTabIndex == oldTabIndex) {
        event.Skip();
        return;
    }

    // Notify old tab it's being deactivated
    if (oldTabIndex >= 0 && oldTabIndex < m_notebook->GetPageCount()) {
        wxWindow* oldPanel = m_notebook->GetPage(oldTabIndex);
        auto it = m_activityPanels.find(oldPanel);
        if (it != m_activityPanels.end()) {
            it->second->OnTabDeactivated();
        }
    }

    // Update current tab index
    m_currentTabIndex = newTabIndex;

    // Get current activity file path
    std::string activityFilePath = GetCurrentActivityFilePath();

    // Notify new tab it's being activated
    NotifyTabActivation(newTabIndex, activityFilePath);

    event.Skip();
}

MainFrame::~MainFrame() {
    // Manually remove all pages from the notebook before unique_ptr destructors run
    // This prevents the segfault that occurs when wxNotebook tries to delete already-destroyed panels
    if (m_notebook) {
        // Remove all pages without deleting them (the unique_ptr destructors will handle deletion)
        while (m_notebook->GetPageCount() > 0) {
            m_notebook->RemovePage(0);
        }
    }
}