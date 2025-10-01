#pragma once

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
    #include <wx/wx.h>
#endif

/**
 * Interface class providing file manager operations for GUI panels.
 * Provides cross-platform functionality to open directories and select files
 * in the system's default file manager.
 */
class IFileOperations {
public:
    virtual ~IFileOperations() = default;
    
protected:
    /**
     * Opens a directory in the system's default file manager.
     * 
     * @param path The directory path to open
     * @param selectFile Optional filename to select within the directory
     *                   (file selection support varies by platform and file manager)
     */
    void OpenInFileManager(const wxString& path, const wxString& selectFile = "");
};