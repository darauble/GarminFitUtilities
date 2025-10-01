#include "IFileOperations.hpp"
#include <wx/filename.h>

void IFileOperations::OpenInFileManager(const wxString& path, const wxString& selectFile) {
#ifdef __WXMSW__
    // Windows: Use explorer
    if (!selectFile.IsEmpty()) {
        wxString fullPath = path + wxFileName::GetPathSeparator() + selectFile;
        wxExecute(wxString::Format("explorer /select,\"%s\"", fullPath), wxEXEC_ASYNC);
    } else {
        wxExecute(wxString::Format("explorer \"%s\"", path), wxEXEC_ASYNC);
    }
#elif defined(__WXGTK__)
    // Linux: Try different approaches
    if (!selectFile.IsEmpty()) {
        // First try with file managers that support file selection
        wxString fullPath = path + wxFileName::GetPathSeparator() + selectFile;
        
        // Check which file managers are available and try them
        if (wxFileExists("/usr/bin/nautilus")) {
            wxExecute(wxString::Format("nautilus --select \"%s\"", fullPath), wxEXEC_ASYNC);
        } else if (wxFileExists("/usr/bin/dolphin")) {
            wxExecute(wxString::Format("dolphin --select \"%s\"", fullPath), wxEXEC_ASYNC);
        } else if (wxFileExists("/usr/bin/nemo")) {
            wxExecute(wxString::Format("nemo \"%s\"", fullPath), wxEXEC_ASYNC);
        } else {
            // Fallback to just opening the directory
            wxExecute(wxString::Format("xdg-open \"%s\"", path), wxEXEC_ASYNC);
        }
    } else {
        wxExecute(wxString::Format("xdg-open \"%s\"", path), wxEXEC_ASYNC);
    }
#elif defined(__WXMAC__)
    // macOS: Use Finder
    if (!selectFile.IsEmpty()) {
        wxString fullPath = path + wxFileName::GetPathSeparator() + selectFile;
        wxExecute(wxString::Format("open -R \"%s\"", fullPath), wxEXEC_ASYNC);
    } else {
        wxExecute(wxString::Format("open \"%s\"", path), wxEXEC_ASYNC);
    }
#else
    // Fallback: Try xdg-open
    wxExecute(wxString::Format("xdg-open \"%s\"", path), wxEXEC_ASYNC);
#endif
}