#include "FileTreePanel.hpp"
#include <wx/dir.h>
#include <wx/filename.h>
#include <wx/datetime.h>
#include <algorithm>
#include <vector>
#include <map>
#include <filesystem>
#include "parsers/session-scanner.hpp"
#include "parsers/binary-mapper.hpp"
#include "utils/MetadataCache.hpp"
#include "models/ActivityData.hpp"
#include <fit_date_time.hpp>

// Import ID_TreeSelection constant
enum { ID_TreeSelection = 2000 };

wxBEGIN_EVENT_TABLE(FileTreePanel, wxPanel)
    EVT_CHOICE(ID_VIEW_MODE_CHOICE, FileTreePanel::OnViewModeChanged)
    EVT_CHOICE(ID_TIME_GRANULARITY_CHOICE, FileTreePanel::OnTimeGranularityChanged)
    EVT_TREE_SEL_CHANGED(wxID_ANY, FileTreePanel::OnTreeItemSelected)
    EVT_TREE_ITEM_RIGHT_CLICK(wxID_ANY, FileTreePanel::OnTreeRightClick)
    EVT_MENU(ID_CONTEXT_OPEN_FOLDER, FileTreePanel::OnContextOpenFolder)
    EVT_BUTTON(wxID_ANY, FileTreePanel::OnSortClicked)
wxEND_EVENT_TABLE()

FileTreePanel::FileTreePanel(wxWindow* parent)
    : wxPanel(parent, wxID_ANY),
      m_viewModeChoice(nullptr),
      m_timeGranularityChoice(nullptr),
      m_sortButton(nullptr),
      m_treeCtrl(nullptr),
      m_infoLabel(nullptr),
      m_currentMode(VIEW_FOLDER),
      m_sortAscending(true) {
    
    CreateLayout();
}

void FileTreePanel::CreateLayout() {
    wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);
    
    // View mode selection
    wxStaticText* modeLabel = new wxStaticText(this, wxID_ANY, "View Mode:");
    m_viewModeChoice = new wxChoice(this, ID_VIEW_MODE_CHOICE);
    m_viewModeChoice->Append("Folder Hierarchy");
    m_viewModeChoice->Append("Time Hierarchy");
    m_viewModeChoice->SetSelection(0);
    
    // Time granularity (only for time mode)
    wxStaticText* granularityLabel = new wxStaticText(this, wxID_ANY, "Time Granularity:");
    m_timeGranularityChoice = new wxChoice(this, ID_TIME_GRANULARITY_CHOICE);
    m_timeGranularityChoice->Append("Year");
    m_timeGranularityChoice->Append("Month");
    m_timeGranularityChoice->Append("Day");
    m_timeGranularityChoice->Append("Hour");
    m_timeGranularityChoice->SetSelection(1); // Default to Month
    m_timeGranularityChoice->Enable(false);
    
    // Sort button
    m_sortButton = new wxButton(this, wxID_ANY, "Sort A-Z");
    
    // Tree control
    m_treeCtrl = new wxTreeCtrl(this, wxID_ANY, wxDefaultPosition, wxDefaultSize,
                                wxTR_HAS_BUTTONS | wxTR_SINGLE);
    
    // Add sample tree structure
    wxTreeItemId root = m_treeCtrl->AddRoot("FIT Files");
    wxTreeItemId folder1 = m_treeCtrl->AppendItem(root, "2025");
    wxTreeItemId folder2 = m_treeCtrl->AppendItem(folder1, "January");
    m_treeCtrl->AppendItem(folder2, "activity1.fit");
    m_treeCtrl->AppendItem(folder2, "activity2.fit");
    m_treeCtrl->Expand(root);
    
    // Info label
    m_infoLabel = new wxStaticText(this, wxID_ANY, "Select directory to load files");
    
    // Layout
    mainSizer->Add(modeLabel, 0, wxALL, 2);
    mainSizer->Add(m_viewModeChoice, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 2);
    mainSizer->Add(granularityLabel, 0, wxALL, 2);
    mainSizer->Add(m_timeGranularityChoice, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 2);
    mainSizer->Add(m_sortButton, 0, wxEXPAND | wxALL, 2);
    mainSizer->Add(m_treeCtrl, 1, wxEXPAND | wxALL, 2);
    mainSizer->Add(m_infoLabel, 0, wxALL, 2);
    
    SetSizer(mainSizer);
}

void FileTreePanel::OnViewModeChanged(wxCommandEvent& event) {
    int selection = m_viewModeChoice->GetSelection();
    m_currentMode = (selection == 0) ? VIEW_FOLDER : VIEW_TIME;
    
    // Enable/disable time granularity choice
    m_timeGranularityChoice->Enable(m_currentMode == VIEW_TIME);
    
    // Rebuild tree based on new mode
    RebuildTree();
}

void FileTreePanel::OnTimeGranularityChanged(wxCommandEvent& event) {
    // Only rebuild if we're in time mode
    if (m_currentMode == VIEW_TIME) {
        RebuildTree();
    }
}

void FileTreePanel::OnTreeItemSelected(wxTreeEvent& event) {
    wxTreeItemId item = event.GetItem();
    if (item.IsOk()) {
        wxString text = m_treeCtrl->GetItemText(item);
        m_infoLabel->SetLabel("Selected: " + text);
        
        // Build full path from selected item
        if (m_currentMode == VIEW_FOLDER) {
            // For folder mode, build path from tree hierarchy
            wxString fullPath;
            wxTreeItemId current = item;
            wxString pathParts;
            
            // Build path by going up the tree (excluding root)
            while (current.IsOk() && current != m_treeCtrl->GetRootItem()) {
                wxString itemText = m_treeCtrl->GetItemText(current);
                if (!itemText.StartsWith("FIT Files:") && !itemText.EndsWith(".fit")) {
                    if (pathParts.IsEmpty()) {
                        pathParts = itemText;
                    } else {
                        pathParts = itemText + wxFileName::GetPathSeparator() + pathParts;
                    }
                }
                current = m_treeCtrl->GetItemParent(current);
            }
            
            if (!pathParts.IsEmpty()) {
                m_selectedPath = m_currentDirectory + wxFileName::GetPathSeparator() + pathParts;
            } else {
                m_selectedPath = m_currentDirectory;
            }
        } else {
            // For time mode, parse the tree hierarchy to build time filter
            wxString timeFilterData = "";
            wxTreeItemId current = item;
            std::vector<wxString> pathParts;
            
            // Build path from tree hierarchy (excluding root)
            while (current.IsOk() && current != m_treeCtrl->GetRootItem()) {
                wxString itemText = m_treeCtrl->GetItemText(current);
                if (!itemText.StartsWith("FIT Files:") && !itemText.EndsWith(".fit")) {
                    pathParts.insert(pathParts.begin(), itemText);
                }
                current = m_treeCtrl->GetItemParent(current);
            }
            
            // Parse time hierarchy: Year -> Month -> Day -> Hour
            if (!pathParts.empty()) {
                // Create time filter string: "YEAR,MONTH,DAY,HOUR"
                wxString year = "";
                wxString month = "";
                wxString day = "";
                wxString hour = "";
                
                if (pathParts.size() >= 1) {
                    year = pathParts[0]; // e.g., "2025"
                }
                if (pathParts.size() >= 2) {
                    // Extract month number from "02 - February" format
                    wxString monthStr = pathParts[1];
                    if (monthStr.Contains(" - ")) {
                        month = monthStr.BeforeFirst(' '); // e.g., "02"
                    }
                }
                if (pathParts.size() >= 3) {
                    day = pathParts[2]; // e.g., "15"
                }
                if (pathParts.size() >= 4) {
                    // Extract hour from "14:00" format
                    wxString hourStr = pathParts[3];
                    if (hourStr.Contains(":")) {
                        hour = hourStr.BeforeFirst(':'); // e.g., "14"
                    }
                }
                
                // Create time filter format: "TIME:YEAR,MONTH,DAY,HOUR"
                timeFilterData = wxString::Format("TIME:%s,%s,%s,%s", year, month, day, hour);
            }
            
            m_selectedPath = timeFilterData.IsEmpty() ? m_currentDirectory : timeFilterData;
        }
        
        // Notify main frame about selection change
        // Need to go up two levels: Panel -> Splitter -> MainFrame
        wxWindow* splitter = GetParent();
        if (splitter) {
            wxWindow* mainFrame = splitter->GetParent();
            if (mainFrame) {
                wxCommandEvent evt(wxEVT_COMMAND_MENU_SELECTED, ID_TreeSelection);
                evt.SetString(m_selectedPath);
                evt.SetEventObject(this);
                mainFrame->GetEventHandler()->ProcessEvent(evt);
            }
        }
    }
}

wxString FileTreePanel::GetSelectedPath() const {
    return m_selectedPath;
}

void FileTreePanel::SetDirectory(const wxString& path) {
    m_currentDirectory = path;
    RebuildTree();
}

void FileTreePanel::RebuildTree() {
    if (m_currentDirectory.IsEmpty()) return;
    
    m_treeCtrl->DeleteAllItems();
    
    wxTreeItemId root = m_treeCtrl->AddRoot("FIT Files: " + m_currentDirectory);
    
    if (m_currentMode == VIEW_FOLDER) {
        // Build folder hierarchy recursively
        BuildFolderHierarchy(root, m_currentDirectory, 0);
    } else {
        // Build time hierarchy using real FIT file timestamps
        BuildTimeHierarchy(root);
    }
    
    m_treeCtrl->Expand(root);
    
    // Sort the tree after building it (only for folder mode)
    if (m_currentMode == VIEW_FOLDER) {
        SortTreeRecursively(root);
    }
    
    // Force complete refresh of the tree control
    m_treeCtrl->Freeze();
    m_treeCtrl->Thaw();
    m_treeCtrl->Refresh();
    m_treeCtrl->Update();
    
    // Also force repaint of the parent window
    this->Refresh();
    this->Update();
    
    m_infoLabel->SetLabel(wxString::Format("Found FIT files in: %s", m_currentDirectory));
}

void FileTreePanel::OnSortClicked(wxCommandEvent& event) {
    // Toggle sort direction
    m_sortAscending = !m_sortAscending;
    
    // Update button text
    m_sortButton->SetLabel(m_sortAscending ? "Sort A-Z" : "Sort Z-A");
    
    // Sort the tree
    if (m_treeCtrl->GetRootItem().IsOk()) {
        SortTreeRecursively(m_treeCtrl->GetRootItem());
    }
}

void FileTreePanel::SortTreeRecursively(wxTreeItemId item) {
    if (!item.IsOk()) return;
    
    // Collect all children with their text and subtree information
    std::vector<std::pair<wxString, wxTreeItemId>> children;
    wxTreeItemIdValue cookie;
    wxTreeItemId child = m_treeCtrl->GetFirstChild(item, cookie);
    while (child.IsOk()) {
        children.push_back(std::make_pair(m_treeCtrl->GetItemText(child), child));
        child = m_treeCtrl->GetNextChild(item, cookie);
    }
    
    // If no children, nothing to sort
    if (children.empty()) return;
    
    // First recursively sort all children's subtrees
    for (const auto& child : children) {
        SortTreeRecursively(child.second);
    }
    
    // Sort the children by name
    if (m_sortAscending) {
        std::sort(children.begin(), children.end());
    } else {
        std::sort(children.begin(), children.end(), std::greater<std::pair<wxString, wxTreeItemId>>());
    }
    
    // Now reorder the children in the tree while preserving their subtrees
    // We'll do this by creating a temporary mapping and rebuilding
    std::vector<TreeNodeInfo> sortedNodes;
    
    // Collect complete node information including subtrees
    for (const auto& child : children) {
        TreeNodeInfo nodeInfo;
        nodeInfo.text = child.first;
        nodeInfo.itemId = child.second;
        CopySubtree(child.second, nodeInfo);
        sortedNodes.push_back(nodeInfo);
    }
    
    // Remove all children
    m_treeCtrl->DeleteChildren(item);
    
    // Recreate children in sorted order with their subtrees
    for (const auto& nodeInfo : sortedNodes) {
        wxTreeItemId newChild = m_treeCtrl->AppendItem(item, nodeInfo.text);
        RecreateSubtree(newChild, nodeInfo);
    }
}

void FileTreePanel::CopySubtree(wxTreeItemId source, TreeNodeInfo& nodeInfo) {
    if (!source.IsOk()) return;
    
    // Copy all children recursively
    wxTreeItemIdValue cookie;
    wxTreeItemId child = m_treeCtrl->GetFirstChild(source, cookie);
    while (child.IsOk()) {
        TreeNodeInfo childInfo;
        childInfo.text = m_treeCtrl->GetItemText(child);
        childInfo.itemId = child;
        
        // Recursively copy the child's subtree
        CopySubtree(child, childInfo);
        
        nodeInfo.children.push_back(childInfo);
        child = m_treeCtrl->GetNextChild(source, cookie);
    }
}

void FileTreePanel::RecreateSubtree(wxTreeItemId parent, const TreeNodeInfo& nodeInfo) {
    if (!parent.IsOk()) return;
    
    // Recreate all children
    for (const auto& childInfo : nodeInfo.children) {
        wxTreeItemId newChild = m_treeCtrl->AppendItem(parent, childInfo.text);
        
        // Recursively recreate the child's subtree
        RecreateSubtree(newChild, childInfo);
    }
}

void FileTreePanel::OnTreeRightClick(wxTreeEvent& event) {
    // Select the item that was right-clicked
    m_treeCtrl->SelectItem(event.GetItem());
    
    // Always show context menu if we have a current directory
    if (!m_currentDirectory.IsEmpty()) {
        wxMenu contextMenu;
        contextMenu.Append(ID_CONTEXT_OPEN_FOLDER, "Open Folder...");
        PopupMenu(&contextMenu);
    }
}

void FileTreePanel::OnContextOpenFolder(wxCommandEvent& event) {
    wxString path = GetSelectedPath();
    wxString fullPath;
    
    // Determine which directory to open
    if (m_currentMode == VIEW_FOLDER) {
        // In folder mode, use the selected path, or current directory if root selected
        if (path.IsEmpty() || path == m_currentDirectory) {
            fullPath = m_currentDirectory;
        } else {
            fullPath = path;
        }
    } else {
        // In time mode, always open the base directory
        fullPath = m_currentDirectory;
    }
    
    if (wxDirExists(fullPath)) {
        OpenInFileManager(fullPath);
    }
}


void FileTreePanel::BuildTimeHierarchy(wxTreeItemId root) {
    if (m_currentDirectory.IsEmpty()) return;
    
    // Map to organize files by time hierarchy
    std::map<wxString, std::map<wxString, std::map<wxString, std::map<wxString, std::vector<wxString>>>>> timeTree;
    
    // Get granularity level
    int granularity = m_timeGranularityChoice->GetSelection(); // 0=Year, 1=Month, 2=Day, 3=Hour
    
    // Scan directory recursively for FIT files
    ScanDirectoryForTimeHierarchy(m_currentDirectory, "", timeTree, granularity);
    
    // Build tree from organized data
    for (const auto& yearEntry : timeTree) {
        wxTreeItemId yearNode = m_treeCtrl->AppendItem(root, yearEntry.first);
        
        if (granularity > 0) { // Month level
            for (const auto& monthEntry : yearEntry.second) {
                wxTreeItemId monthNode = m_treeCtrl->AppendItem(yearNode, monthEntry.first);
                
                if (granularity > 1) { // Day level
                    for (const auto& dayEntry : monthEntry.second) {
                        wxTreeItemId dayNode = m_treeCtrl->AppendItem(monthNode, dayEntry.first);
                        
                        if (granularity > 2) { // Hour level
                            for (const auto& hourEntry : dayEntry.second) {
                                // Only add hour nodes if they contain files
                                if (!hourEntry.second.empty()) {
                                    m_treeCtrl->AppendItem(dayNode, hourEntry.first);
                                }
                            }
                        }
                    }
                }
            }
        }
        
        // Expand year nodes for better visibility when granularity > 0
        if (granularity > 0) {
            m_treeCtrl->Expand(yearNode);
        }
    }
}

void FileTreePanel::BuildFolderHierarchy(wxTreeItemId parentItem, const wxString& path, int depth) {
    wxDir dir(path);
    if (!dir.IsOpened()) {
        return;
    }
    
    wxString filename;
    
    // Add subdirectories recursively
    bool cont = dir.GetFirst(&filename, "", wxDIR_DIRS);
    while (cont) {
        // Create tree item for this subdirectory
        wxTreeItemId folderItem = m_treeCtrl->AppendItem(parentItem, filename);
        
        // Recursively build hierarchy for this subdirectory (limit depth to 3 levels)
        if (depth < 3) {
            wxString subPath = path + wxFileName::GetPathSeparator() + filename;
            BuildFolderHierarchy(folderItem, subPath, depth + 1);
        }
        
        cont = dir.GetNext(&filename);
    }
}

void FileTreePanel::ScanDirectoryForTimeHierarchy(const wxString& path, const wxString& relativePath,
    std::map<wxString, std::map<wxString, std::map<wxString, std::map<wxString, std::vector<wxString>>>>>& timeTree,
    int granularity) {
    
    wxDir dir(path);
    if (!dir.IsOpened()) return;
    
    wxString filename;
    
    // Scan for FIT files
    bool cont = dir.GetFirst(&filename, "*.fit", wxDIR_FILES);
    while (cont) {
        wxString fullPath = path + wxFileName::GetPathSeparator() + filename;
        wxString displayPath = relativePath.IsEmpty() ? filename : 
                              relativePath + wxFileName::GetPathSeparator() + filename;
        
        // Try to load timestamp from cache first
        uint32_t timestamp = 0;
        bool loadedFromCache = false;

        if (MetadataCache::IsCacheValid(fullPath)) {
            ActivityDisplayData cachedData;
            if (MetadataCache::LoadFromCache(fullPath, cachedData)) {
                timestamp = cachedData.timestamp;
                loadedFromCache = true;
            }
        }

        // If not in cache, parse FIT file
        if (!loadedFromCache) {
            try {
                std::filesystem::path fitPath(fullPath.ToStdString());
                darauble::BinaryMapper mapper(fitPath);
                darauble::SessionScanner scanner(filename.ToStdString(), mapper);

                scanner.scan();

                if (scanner.hasData()) {
                    const auto& activityData = scanner.getData();
                    auto aggregated = activityData.getAggregatedData();
                    timestamp = aggregated.timestamp;

                    // Create cache entry for faster subsequent scans
                    ActivityDisplayData displayData;
                    displayData.timestamp = timestamp;
                    displayData.fullPath = fullPath;
                    displayData.filePath = displayPath;

                    if (timestamp > 0) {
                        fit::DateTime fitDateTime(timestamp);
                        wxDateTime activityTime(fitDateTime.GetTimeT());
                        displayData.date = activityTime.Format("%Y-%m-%d %H:%M");
                    }

                    displayData.name = wxString::FromUTF8(activityData.activityName);
                    displayData.sport = wxString::FromUTF8(activityData.getPrimarySportName());
                    displayData.duration = wxString::FromUTF8(aggregated.getFormattedDuration());
                    displayData.distance = wxString::FromUTF8(aggregated.getFormattedDistance(activityData.primarySport, activityData.primarySubSport));
                    displayData.speedPace = wxString::FromUTF8(aggregated.getFormattedSpeed(activityData.primarySport, activityData.primarySubSport));
                    displayData.heartRate = wxString::FromUTF8(aggregated.getFormattedHeartRate());

                    // Save to cache
                    MetadataCache::SaveToCache(fullPath, displayData, false);
                }
            } catch (const std::exception& e) {
                // Parsing failed - timestamp remains 0
            }
        }

        // Add to time tree if we have a valid timestamp
        if (timestamp > 0) {
            try {
                // Convert FIT timestamp to UNIX timestamp using FIT SDK
                fit::DateTime fitDateTime(timestamp);
                wxDateTime activityTime(fitDateTime.GetTimeT());

                // Build hierarchy keys
                wxString year = wxString::Format("%d", activityTime.GetYear());
                wxString month = wxString::Format("%02d - %s", activityTime.GetMonth() + 1,
                                                activityTime.GetMonthName(activityTime.GetMonth()));
                wxString day = wxString::Format("%02d", activityTime.GetDay());
                wxString hour = wxString::Format("%02d:00", activityTime.GetHour());

                // Add to time tree
                timeTree[year][month][day][hour].push_back(displayPath);
            } catch (const std::exception& e) {
                // Fallback to Unknown
                timeTree["Unknown"]["Unknown"]["Unknown"]["Unknown"].push_back(displayPath);
            }
        } else {
            // No valid timestamp - add to Unknown
            timeTree["Unknown"]["Unknown"]["Unknown"]["Unknown"].push_back(displayPath);
        }
        
        cont = dir.GetNext(&filename);
    }
    
    // Recursively scan subdirectories
    cont = dir.GetFirst(&filename, "", wxDIR_DIRS);
    while (cont) {
        wxString subPath = path + wxFileName::GetPathSeparator() + filename;
        wxString newRelativePath = relativePath.IsEmpty() ? filename : 
                                 relativePath + wxFileName::GetPathSeparator() + filename;
        ScanDirectoryForTimeHierarchy(subPath, newRelativePath, timeTree, granularity);
        cont = dir.GetNext(&filename);
    }
}