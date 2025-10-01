#pragma once

#include <wx/string.h>
#include <cstdint>

struct ActivityDisplayData {
    wxString date;
    wxString name;         // Activity name from Sport message Field #3 (UTF-8)
    wxString sport;
    wxString duration;
    wxString distance;
    wxString speedPace;
    wxString heartRate;
    wxString filePath;     // Display path (relative, for showing in UI)
    wxString fullPath;     // Full absolute path (for file operations)
    uint32_t timestamp;
};