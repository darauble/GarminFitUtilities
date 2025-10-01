#pragma once

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
    #include <wx/wx.h>
#endif

class SettingsDialog : public wxDialog {
public:
    SettingsDialog(wxWindow* parent);
    virtual ~SettingsDialog() = default;

private:
    void CreateLayout();
    void OnBrowseOSM(wxCommandEvent& event);
    void OnOK(wxCommandEvent& event);
    
    wxTextCtrl* m_osmFileCtrl;
    
    enum {
        ID_BROWSE_OSM = 5000
    };
    
    wxDECLARE_EVENT_TABLE();
};