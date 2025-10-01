#pragma once

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
    #include <wx/wx.h>
#endif
#include <wx/listctrl.h>
#include <wx/dnd.h>
#include <vector>
#include "../models/ActivityData.hpp"
#include "../interfaces/IFileOperations.hpp"

// Forward declaration
class ActivitiesPanel;

// Custom list control with drag & drop and editing support
class DragDropListCtrl : public wxListCtrl {
public:
    DragDropListCtrl(ActivitiesPanel* parent, wxWindowID id = wxID_ANY);

    void StartEditing(long item, int column);

private:
    void OnBeginDrag(wxListEvent& event);
    void OnRightClick(wxListEvent& event);
    void OnKeyDown(wxKeyEvent& event);
    void OnChar(wxKeyEvent& event);

    ActivitiesPanel* m_activitiesPanel;

    wxDECLARE_EVENT_TABLE();
};

class ActivitiesPanel : public wxPanel, public IFileOperations {
public:
    ActivitiesPanel(wxWindow* parent);
    virtual ~ActivitiesPanel() = default;
    
    void SetDirectory(const wxString& path);
    void SetFilterPath(const wxString& filterPath); // Filter to specific subdirectory
    void SetTimeFilter(int year, int month = -1, int day = -1, int hour = -1); // Filter by time range
    void ClearTimeFilter(); // Clear time-based filtering
    
    // Navigation support methods
    int GetActivityCount() const;
    bool GetActivityData(int index, ActivityDisplayData& data) const;
    void SetSelection(int index);
    
    // Drag & drop support
    void StartDragDrop(long itemIndex, bool moveOperation = false);
    
    // Context menu support
    void ShowContextMenu(long itemIndex);

    // Editing support
    void StartEditingName(long itemIndex);
    void SaveEditedName(long itemIndex, const wxString& newName);

private:
    enum {
        ID_CONTEXT_OPEN_CONTAINING_FOLDER = 1100
    };
    
    void CreateLayout();
    void OnItemSelected(wxListEvent& event);
    void OnItemDoubleClicked(wxListEvent& event);
    void OnRefreshClicked(wxCommandEvent& event);
    void OnContextOpenContainingFolder(wxCommandEvent& event);
    void OnEndLabelEdit(wxListEvent& event);

    DragDropListCtrl* m_listCtrl;
    wxButton* m_refreshButton;
    wxButton* m_detailsButton;
    
    wxString m_currentDirectory;
    wxString m_filterPath; // Current filter path for tree selection
    std::vector<ActivityDisplayData> m_activityData; // Collected activity data for sorting
    
    // Time filtering
    bool m_useTimeFilter;
    int m_filterYear;
    int m_filterMonth; // -1 means no month filter
    int m_filterDay;   // -1 means no day filter  
    int m_filterHour;  // -1 means no hour filter
    
    void LoadActivitiesFromDirectory();
    void ScanDirectoryRecursive(const wxString& path, const wxString& relativePath = "");
    void PopulateListFromData();

    wxDECLARE_EVENT_TABLE();
};