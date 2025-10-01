#include "ActivitiesPanel.hpp"
#include <wx/dir.h>
#include <wx/filename.h>
#include <wx/datetime.h>
#include <wx/textdlg.h>
#include "parsers/session-scanner.hpp"
#include "parsers/binary-mapper.hpp"
#include "utils/MetadataCache.hpp"
#include <fit_date_time.hpp>
#include <filesystem>
#include <algorithm>

// Event table for DragDropListCtrl
wxBEGIN_EVENT_TABLE(DragDropListCtrl, wxListCtrl)
    EVT_LIST_BEGIN_DRAG(wxID_ANY, DragDropListCtrl::OnBeginDrag)
    EVT_LIST_ITEM_RIGHT_CLICK(wxID_ANY, DragDropListCtrl::OnRightClick)
    EVT_KEY_DOWN(DragDropListCtrl::OnKeyDown)
    EVT_CHAR(DragDropListCtrl::OnChar)
wxEND_EVENT_TABLE()

wxBEGIN_EVENT_TABLE(ActivitiesPanel, wxPanel)
    EVT_LIST_ITEM_SELECTED(wxID_ANY, ActivitiesPanel::OnItemSelected)
    EVT_LIST_ITEM_ACTIVATED(wxID_ANY, ActivitiesPanel::OnItemDoubleClicked)
    EVT_BUTTON(wxID_ANY, ActivitiesPanel::OnRefreshClicked)
    EVT_MENU(ID_CONTEXT_OPEN_CONTAINING_FOLDER, ActivitiesPanel::OnContextOpenContainingFolder)
    EVT_LIST_END_LABEL_EDIT(wxID_ANY, ActivitiesPanel::OnEndLabelEdit)
wxEND_EVENT_TABLE()

// DragDropListCtrl implementation
DragDropListCtrl::DragDropListCtrl(ActivitiesPanel* parent, wxWindowID id)
    : wxListCtrl(parent, id, wxDefaultPosition, wxDefaultSize, wxLC_REPORT | wxLC_SINGLE_SEL | wxLC_EDIT_LABELS),
      m_activitiesPanel(parent) {
    // wxLC_EDIT_LABELS enables in-place editing
}

void DragDropListCtrl::OnBeginDrag(wxListEvent& event) {
    long itemIndex = event.GetIndex();
    bool shiftPressed = wxGetKeyState(WXK_SHIFT);
    
    if (m_activitiesPanel) {
        m_activitiesPanel->StartDragDrop(itemIndex, shiftPressed);
    }
}

void DragDropListCtrl::OnRightClick(wxListEvent& event) {
    long itemIndex = event.GetIndex();
    
    if (m_activitiesPanel && itemIndex >= 0) {
        m_activitiesPanel->ShowContextMenu(itemIndex);
    }
}

void DragDropListCtrl::OnKeyDown(wxKeyEvent& event) {
    int keyCode = event.GetKeyCode();

    if (keyCode == WXK_F2) {
        // F2 pressed - start editing the Name column
        long selectedItem = GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
        if (selectedItem != -1) {
            StartEditing(selectedItem, 1); // Column 1 is Name
        }
    } else {
        event.Skip();
    }
}

void DragDropListCtrl::OnChar(wxKeyEvent& event) {
    // Handle character events if needed
    event.Skip();
}

void DragDropListCtrl::StartEditing(long item, int column) {
    if (item < 0 || item >= GetItemCount()) {
        return;
    }

    // In wxListCtrl with wxLC_EDIT_LABELS, EditLabel() only works for column 0
    // For other columns, we need a workaround
    if (column == 1) {
        // We'll use a text control overlay for editing
        // But first, let's try the simpler approach: temporarily swap columns
        // Actually, wxListCtrl doesn't support editing arbitrary columns directly
        // We need to use EditLabel which only works on the first column

        // Workaround: Get the name value, let them edit the first visible text
        wxString nameValue = GetItemText(item, 1);

        // Create a custom edit
        if (m_activitiesPanel) {
            m_activitiesPanel->StartEditingName(item);
        }
    }
}

ActivitiesPanel::ActivitiesPanel(wxWindow* parent)
    : wxPanel(parent, wxID_ANY),
      m_listCtrl(nullptr),
      m_refreshButton(nullptr),
      m_detailsButton(nullptr),
      m_useTimeFilter(false),
      m_filterYear(-1),
      m_filterMonth(-1),
      m_filterDay(-1),
      m_filterHour(-1) {
    
    CreateLayout();
}

void ActivitiesPanel::CreateLayout() {
    wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);
    
    // Control buttons
    wxBoxSizer* buttonSizer = new wxBoxSizer(wxHORIZONTAL);
    m_refreshButton = new wxButton(this, wxID_ANY, "Refresh");
    m_detailsButton = new wxButton(this, wxID_ANY, "Show Details");
    m_detailsButton->Enable(false);
    
    buttonSizer->Add(m_refreshButton, 0, wxRIGHT, 5);
    buttonSizer->Add(m_detailsButton, 0, wxLEFT, 5);
    buttonSizer->AddStretchSpacer();
    
    // Activity list with drag & drop support
    m_listCtrl = new DragDropListCtrl(this, wxID_ANY);
    
    // Add columns
    m_listCtrl->AppendColumn("Date", wxLIST_FORMAT_LEFT, 120);
    m_listCtrl->AppendColumn("Name", wxLIST_FORMAT_LEFT, 150);
    m_listCtrl->AppendColumn("Sport", wxLIST_FORMAT_LEFT, 100);
    m_listCtrl->AppendColumn("Duration", wxLIST_FORMAT_LEFT, 80);
    m_listCtrl->AppendColumn("Work", wxLIST_FORMAT_LEFT, 80);
    m_listCtrl->AppendColumn("Result", wxLIST_FORMAT_LEFT, 150);
    m_listCtrl->AppendColumn("Avg HR", wxLIST_FORMAT_LEFT, 60);
    m_listCtrl->AppendColumn("File", wxLIST_FORMAT_LEFT, 200);
    
    // Add sample data for demonstration
    long index = m_listCtrl->InsertItem(0, "2025-01-15");
    m_listCtrl->SetItem(index, 1, "Morning Run");
    m_listCtrl->SetItem(index, 2, "Running");
    m_listCtrl->SetItem(index, 3, "45:32");
    m_listCtrl->SetItem(index, 4, "8.5 km");
    m_listCtrl->SetItem(index, 5, "5:21 /km");
    m_listCtrl->SetItem(index, 6, "145");
    m_listCtrl->SetItem(index, 7, "sample-activity.fit");
    
    // Layout
    mainSizer->Add(buttonSizer, 0, wxEXPAND | wxALL, 5);
    mainSizer->Add(m_listCtrl, 1, wxEXPAND | wxALL, 5);
    
    SetSizer(mainSizer);
}

void ActivitiesPanel::OnItemSelected(wxListEvent& event) {
    m_detailsButton->Enable(true);
    
    // Notify parent about selection change
    long selectedIndex = event.GetIndex();
    
    // Send a command event to the MainFrame with the selected index
    wxCommandEvent selectionEvent(wxEVT_COMMAND_MENU_SELECTED, 2003); // ID_ActivitySelected
    selectionEvent.SetInt(selectedIndex);
    selectionEvent.SetEventObject(this);
    
    // Find the MainFrame (traverse up the parent hierarchy)
    wxWindow* parent = GetParent();
    while (parent && !dynamic_cast<wxFrame*>(parent)) {
        parent = parent->GetParent();
    }
    
    if (parent) {
        parent->GetEventHandler()->AddPendingEvent(selectionEvent);
    }
}

void ActivitiesPanel::OnItemDoubleClicked(wxListEvent& event) {
    wxMessageBox("Activity details dialog would open here", "Activity Details", 
                 wxOK | wxICON_INFORMATION);
}

void ActivitiesPanel::SetDirectory(const wxString& path) {
    m_currentDirectory = path;
    m_filterPath.Clear(); // Clear any existing filter when setting new directory
    LoadActivitiesFromDirectory();
}

void ActivitiesPanel::LoadActivitiesFromDirectory() {
    if (m_currentDirectory.IsEmpty()) return;
    
    // Clear existing data
    m_listCtrl->DeleteAllItems();
    m_activityData.clear();
    
    // Determine which directory to scan
    wxString scanPath = m_currentDirectory;
    if (!m_filterPath.IsEmpty()) {
        scanPath = m_filterPath;
    }
    
    // Start recursive scan to collect data
    ScanDirectoryRecursive(scanPath);
    
    // Sort and populate the list
    PopulateListFromData();
}

void ActivitiesPanel::ScanDirectoryRecursive(const wxString& path, const wxString& relativePath) {
    wxDir dir(path);
    if (!dir.IsOpened()) return;
    
    wxString filename;
    
    // First, scan for FIT files in current directory
    bool cont = dir.GetFirst(&filename, "*.fit", wxDIR_FILES);
    while (cont) {
        wxString fullPath = path + wxFileName::GetPathSeparator() + filename;
        wxString displayPath = relativePath.IsEmpty() ? filename : 
                              relativePath + wxFileName::GetPathSeparator() + filename;
        
        // Try to load from cache first
        ActivityDisplayData displayData;
        displayData.filePath = displayPath;
        displayData.fullPath = fullPath;

        bool loadedFromCache = false;
        if (MetadataCache::IsCacheValid(fullPath)) {
            if (MetadataCache::LoadFromCache(fullPath, displayData)) {
                loadedFromCache = true;
            }
        }

        // Parse FIT file if not loaded from cache
        if (!loadedFromCache) {
            try {
                std::filesystem::path fitPath(fullPath.ToStdString());
                darauble::BinaryMapper mapper(fitPath);
                darauble::SessionScanner scanner(filename.ToStdString(), mapper);

                scanner.scan();
            
            if (scanner.hasData()) {
                const auto& activityData = scanner.getData();
                auto aggregated = activityData.getAggregatedData();
                
                // Store timestamp for sorting
                displayData.timestamp = aggregated.timestamp;
                
                // Format timestamp as date
                if (aggregated.timestamp > 0) {
                    // Convert FIT timestamp to UNIX timestamp using FIT SDK
                    fit::DateTime fitDateTime(aggregated.timestamp);
                    wxDateTime activityTime(fitDateTime.GetTimeT());
                    displayData.date = activityTime.Format("%Y-%m-%d %H:%M");
                } else {
                    displayData.date = "Unknown Date";
                }
                
                displayData.name = wxString::FromUTF8(activityData.activityName);
                displayData.sport = wxString::FromUTF8(activityData.getPrimarySportName());
                if (activityData.isMultisport()) {
                    displayData.sport += wxString::Format(" (%zu sessions)", activityData.sessions.size());
                }

                displayData.duration = wxString::FromUTF8(aggregated.getFormattedDuration());
                displayData.distance = wxString::FromUTF8(aggregated.getFormattedDistance(activityData.primarySport, activityData.primarySubSport));
                displayData.speedPace = wxString::FromUTF8(aggregated.getFormattedSpeed(activityData.primarySport, activityData.primarySubSport));
                displayData.heartRate = wxString::FromUTF8(aggregated.getFormattedHeartRate());

                // Save to cache (preserveName=true if cache already existed, to not overwrite user edits)
                MetadataCache::SaveToCache(fullPath, displayData, wxFileExists(MetadataCache::GetMetaFilePath(fullPath)));
                } else {
                    // Fallback for files that couldn't be parsed
                    displayData.timestamp = 0;
                    displayData.date = "Parse Error";
                    displayData.name = "";
                    displayData.sport = "Unknown";
                    displayData.duration = "--:--";
                    displayData.distance = "-- km";
                    displayData.speedPace = "--";
                    displayData.heartRate = "--";
                }

            } catch (const std::exception& e) {
                // Handle parsing errors gracefully
                displayData.timestamp = 0;
                displayData.date = "Parse Error";
                displayData.name = "";
                displayData.sport = "Error";
                displayData.duration = "--:--";
                displayData.distance = "-- km";
                displayData.speedPace = "--";
                displayData.heartRate = "--";
            }
        }

        // Apply time filtering (for both cached and freshly parsed data)
        if (m_useTimeFilter && displayData.timestamp > 0) {
            fit::DateTime fitDateTime(displayData.timestamp);
            wxDateTime activityTime(fitDateTime.GetTimeT());

            bool passesFilter = true;

            // Year filter (required)
            if (m_filterYear != -1 && activityTime.GetYear() != m_filterYear) {
                passesFilter = false;
            }

            // Month filter (1-based, optional)
            if (passesFilter && m_filterMonth != -1 && (activityTime.GetMonth() + 1) != m_filterMonth) {
                passesFilter = false;
            }

            // Day filter (optional)
            if (passesFilter && m_filterDay != -1 && activityTime.GetDay() != m_filterDay) {
                passesFilter = false;
            }

            // Hour filter (optional)
            if (passesFilter && m_filterHour != -1 && activityTime.GetHour() != m_filterHour) {
                passesFilter = false;
            }

            // Skip this file if it doesn't pass the time filter
            if (!passesFilter) {
                cont = dir.GetNext(&filename);
                continue;
            }
        }

        m_activityData.push_back(displayData);
        
        cont = dir.GetNext(&filename);
    }
    
    // Always scan subdirectories recursively 
    // (This ensures clicking any folder shows all files from that folder and below)
    cont = dir.GetFirst(&filename, "", wxDIR_DIRS);
    while (cont) {
        wxString subPath = path + wxFileName::GetPathSeparator() + filename;
        wxString newRelativePath = relativePath.IsEmpty() ? filename : 
                                 relativePath + wxFileName::GetPathSeparator() + filename;
        ScanDirectoryRecursive(subPath, newRelativePath);
        cont = dir.GetNext(&filename);
    }
}

void ActivitiesPanel::SetFilterPath(const wxString& filterPath) {
    m_filterPath = filterPath;
    LoadActivitiesFromDirectory();
}

void ActivitiesPanel::SetTimeFilter(int year, int month, int day, int hour) {
    m_useTimeFilter = true;
    m_filterYear = year;
    m_filterMonth = month;
    m_filterDay = day;
    m_filterHour = hour;
    
    // Clear path filter when using time filter
    m_filterPath.Clear();
    
    LoadActivitiesFromDirectory();
}

void ActivitiesPanel::ClearTimeFilter() {
    m_useTimeFilter = false;
    m_filterYear = -1;
    m_filterMonth = -1;
    m_filterDay = -1;
    m_filterHour = -1;
    LoadActivitiesFromDirectory();
}

void ActivitiesPanel::PopulateListFromData() {
    // Sort by timestamp descending (newest first)
    std::sort(m_activityData.begin(), m_activityData.end(), 
              [](const ActivityDisplayData& a, const ActivityDisplayData& b) {
                  return a.timestamp > b.timestamp;  // Descending order
              });
    
    // Populate the list control
    for (const auto& data : m_activityData) {
        int itemIndex = m_listCtrl->GetItemCount();
        long index = m_listCtrl->InsertItem(itemIndex, data.date);
        m_listCtrl->SetItem(index, 1, data.name);
        m_listCtrl->SetItem(index, 2, data.sport);
        m_listCtrl->SetItem(index, 3, data.duration);
        m_listCtrl->SetItem(index, 4, data.distance);
        m_listCtrl->SetItem(index, 5, data.speedPace);
        m_listCtrl->SetItem(index, 6, data.heartRate);
        m_listCtrl->SetItem(index, 7, data.filePath);
    }
    
    // If no files found, show message
    if (m_activityData.empty()) {
        long index = m_listCtrl->InsertItem(0, "No FIT files found");
        m_listCtrl->SetItem(index, 1, "");
        m_listCtrl->SetItem(index, 2, "");
        m_listCtrl->SetItem(index, 3, "");
        m_listCtrl->SetItem(index, 4, "");
        m_listCtrl->SetItem(index, 5, "");
        m_listCtrl->SetItem(index, 6, m_filterPath.IsEmpty() ? 
                            "Try opening a different directory" : 
                            "No files in selected folder");
    }
    
    // Auto-size columns to fit content
    for (int col = 0; col < m_listCtrl->GetColumnCount(); col++) {
        m_listCtrl->SetColumnWidth(col, wxLIST_AUTOSIZE);
        
        // Also check header width and use the larger of content vs header
        int headerWidth = m_listCtrl->GetColumnWidth(col);
        m_listCtrl->SetColumnWidth(col, wxLIST_AUTOSIZE_USEHEADER);
        int contentWidth = m_listCtrl->GetColumnWidth(col);
        
        m_listCtrl->SetColumnWidth(col, wxMax(headerWidth, contentWidth));
    }
}

void ActivitiesPanel::OnRefreshClicked(wxCommandEvent& event) {
    LoadActivitiesFromDirectory();
}

int ActivitiesPanel::GetActivityCount() const {
    return static_cast<int>(m_activityData.size());
}

bool ActivitiesPanel::GetActivityData(int index, ActivityDisplayData& data) const {
    if (index < 0 || index >= static_cast<int>(m_activityData.size())) {
        return false;
    }
    
    data = m_activityData[index];
    return true;
}

void ActivitiesPanel::SetSelection(int index) {
    if (index < 0 || index >= m_listCtrl->GetItemCount()) {
        // Clear selection
        long selected = m_listCtrl->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
        if (selected != -1) {
            m_listCtrl->SetItemState(selected, 0, wxLIST_STATE_SELECTED);
        }
        return;
    }
    
    // Clear previous selection
    long selected = m_listCtrl->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
    if (selected != -1) {
        m_listCtrl->SetItemState(selected, 0, wxLIST_STATE_SELECTED);
    }
    
    // Set new selection
    m_listCtrl->SetItemState(index, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
    m_listCtrl->EnsureVisible(index); // Scroll to make the item visible
}

void ActivitiesPanel::StartDragDrop(long itemIndex, bool moveOperation) {
    // Validate item index
    if (itemIndex < 0 || itemIndex >= static_cast<long>(m_activityData.size())) {
        return;
    }
    
    const ActivityDisplayData& data = m_activityData[itemIndex];
    
    // Create a file data object with the FIT file path
    wxFileDataObject fileData;
    fileData.AddFile(data.fullPath);
    
    // Create drag source
    wxDropSource dragSource(this);
    dragSource.SetData(fileData);
    
    // Perform drag operation
    wxDragResult result = dragSource.DoDragDrop(moveOperation ? wxDrag_DefaultMove : wxDrag_CopyOnly);
    
    // Handle the result if needed
    if (result == wxDragMove && moveOperation) {
        // File was moved successfully - could update UI if needed
        // For now, just refresh the list to reflect any changes
        LoadActivitiesFromDirectory();
    }
}

void ActivitiesPanel::ShowContextMenu(long itemIndex) {
    // Validate item index
    if (itemIndex < 0 || itemIndex >= static_cast<long>(m_activityData.size())) {
        return;
    }
    
    // Create context menu
    wxMenu contextMenu;
    contextMenu.Append(ID_CONTEXT_OPEN_CONTAINING_FOLDER, "Open Containing Folder...");
    
    // Show the menu
    PopupMenu(&contextMenu);
}

void ActivitiesPanel::OnContextOpenContainingFolder(wxCommandEvent& event) {
    // Get the currently selected item
    long selectedIndex = m_listCtrl->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
    if (selectedIndex < 0 || selectedIndex >= static_cast<long>(m_activityData.size())) {
        return;
    }
    
    const ActivityDisplayData& data = m_activityData[selectedIndex];
    
    // Extract directory and filename
    wxFileName fileName(data.fullPath);
    wxString directory = fileName.GetPath();
    wxString filename = fileName.GetFullName();
    
    // Open in file manager with file selection
    OpenInFileManager(directory, filename);
}

void ActivitiesPanel::StartEditingName(long itemIndex) {
    // Validate item index
    if (itemIndex < 0 || itemIndex >= static_cast<long>(m_activityData.size())) {
        return;
    }

    const ActivityDisplayData& data = m_activityData[itemIndex];
    wxString currentName = data.name;

    // Show a text entry dialog for editing the name
    wxTextEntryDialog dialog(this,
                             "Edit activity name:",
                             "Edit Name",
                             currentName,
                             wxOK | wxCANCEL | wxCENTRE);

    if (dialog.ShowModal() == wxID_OK) {
        wxString newName = dialog.GetValue();
        if (newName != currentName) {
            SaveEditedName(itemIndex, newName);
        }
    }
}

void ActivitiesPanel::SaveEditedName(long itemIndex, const wxString& newName) {
    // Validate item index
    if (itemIndex < 0 || itemIndex >= static_cast<long>(m_activityData.size())) {
        return;
    }

    // Update the activity data
    m_activityData[itemIndex].name = newName;

    // Update the list control display
    m_listCtrl->SetItem(itemIndex, 1, newName);

    // Update the metadata cache file - only update the name field
    const ActivityDisplayData& data = m_activityData[itemIndex];

    // Read existing metadata
    wxString metaPath = MetadataCache::GetMetaFilePath(data.fullPath);
    if (wxFileExists(metaPath)) {
        // Load existing metadata
        ActivityDisplayData tempData;
        if (MetadataCache::LoadFromCache(data.fullPath, tempData)) {
            // Update only the name
            tempData.name = newName;
            // Save back with all existing fields but updated name
            MetadataCache::SaveToCache(data.fullPath, tempData, false);
        }
    } else {
        // Create new metadata file
        MetadataCache::SaveToCache(data.fullPath, data, false);
    }
}

void ActivitiesPanel::OnEndLabelEdit(wxListEvent& event) {
    // This event is triggered when inline editing completes
    // We're using a dialog instead, so this might not be used
    // But we keep it for potential future inline editing
    event.Skip();
}

