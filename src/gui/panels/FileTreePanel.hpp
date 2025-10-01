#pragma once

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
    #include <wx/wx.h>
#endif
#include <wx/treectrl.h>
#include <wx/choice.h>
#include <map>
#include <vector>
#include "../interfaces/IFileOperations.hpp"

class FileTreePanel : public wxPanel, public IFileOperations {
public:
    FileTreePanel(wxWindow* parent);
    virtual ~FileTreePanel() = default;

private:
    enum {
        ID_VIEW_MODE_CHOICE = 1000,
        ID_TIME_GRANULARITY_CHOICE = 1001,
        ID_CONTEXT_OPEN_FOLDER = 1002
    };
    
    enum ViewMode {
        VIEW_FOLDER,
        VIEW_TIME
    };

    void CreateLayout();
    void OnViewModeChanged(wxCommandEvent& event);
    void OnTimeGranularityChanged(wxCommandEvent& event);
    void OnTreeItemSelected(wxTreeEvent& event);
    void OnTreeRightClick(wxTreeEvent& event);
    void OnContextOpenFolder(wxCommandEvent& event);
    void OnSortClicked(wxCommandEvent& event);
    
public:
    void SetDirectory(const wxString& path);
    wxString GetSelectedPath() const;
    
private:
    // Structure to store tree node information for sorting
    struct TreeNodeInfo {
        wxString text;
        wxTreeItemId itemId;
        std::vector<TreeNodeInfo> children;
    };
    
    void RebuildTree();
    void SortTreeRecursively(wxTreeItemId item);
    void BuildTimeHierarchy(wxTreeItemId root);
    void BuildFolderHierarchy(wxTreeItemId parentItem, const wxString& path, int depth = 0);
    void ScanDirectoryForTimeHierarchy(const wxString& path, const wxString& relativePath,
        std::map<wxString, std::map<wxString, std::map<wxString, std::map<wxString, std::vector<wxString>>>>>& timeTree,
        int granularity);
    
    // Helper methods for sorting with hierarchy preservation
    void CopySubtree(wxTreeItemId source, TreeNodeInfo& nodeInfo);
    void RecreateSubtree(wxTreeItemId parent, const TreeNodeInfo& nodeInfo);

    wxChoice* m_viewModeChoice;
    wxChoice* m_timeGranularityChoice;
    wxButton* m_sortButton;
    wxTreeCtrl* m_treeCtrl;
    wxStaticText* m_infoLabel;

    ViewMode m_currentMode;
    wxString m_currentDirectory;
    wxString m_selectedPath; // Currently selected path in tree
    bool m_sortAscending; // Sort direction A-Z (true) or Z-A (false)

    wxDECLARE_EVENT_TABLE();
};