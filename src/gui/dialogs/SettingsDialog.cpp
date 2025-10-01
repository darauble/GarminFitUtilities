#include "SettingsDialog.hpp"
#include "../utils/SettingsManager.hpp"
#include <wx/filedlg.h>
#include <wx/statbox.h>

wxBEGIN_EVENT_TABLE(SettingsDialog, wxDialog)
    EVT_BUTTON(ID_BROWSE_OSM, SettingsDialog::OnBrowseOSM)
    EVT_BUTTON(wxID_OK, SettingsDialog::OnOK)
wxEND_EVENT_TABLE()

SettingsDialog::SettingsDialog(wxWindow* parent)
    : wxDialog(parent, wxID_ANY, "Settings", wxDefaultPosition, wxSize(500, 300)),
      m_osmFileCtrl(nullptr) {
    CreateLayout();
}

void SettingsDialog::CreateLayout() {
    wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);
    
    // Map Settings section
    wxStaticBoxSizer* mapSizer = new wxStaticBoxSizer(wxVERTICAL, this, "Map Settings");
    
    // OSM File setting
    wxBoxSizer* osmSizer = new wxBoxSizer(wxHORIZONTAL);
    wxStaticText* osmLabel = new wxStaticText(this, wxID_ANY, "OSM PBF File:");
    m_osmFileCtrl = new wxTextCtrl(this, wxID_ANY, "", wxDefaultPosition, wxSize(300, -1));
    wxButton* browseButton = new wxButton(this, ID_BROWSE_OSM, "Browse...");
    
    osmSizer->Add(osmLabel, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 10);
    osmSizer->Add(m_osmFileCtrl, 1, wxALIGN_CENTER_VERTICAL | wxRIGHT, 5);
    osmSizer->Add(browseButton, 0, wxALIGN_CENTER_VERTICAL);
    
    wxStaticText* osmHelp = new wxStaticText(this, wxID_ANY, 
        "Select an OpenStreetMap PBF file for displaying maps with tracks.\nLeave empty to show tracks only.");
    osmHelp->SetFont(osmHelp->GetFont().Smaller());
    
    mapSizer->Add(osmSizer, 0, wxEXPAND | wxALL, 5);
    mapSizer->Add(osmHelp, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 5);
    
    // Buttons
    wxStdDialogButtonSizer* buttonSizer = CreateStdDialogButtonSizer(wxOK | wxCANCEL);
    
    // Layout
    mainSizer->Add(mapSizer, 0, wxEXPAND | wxALL, 10);
    mainSizer->AddStretchSpacer();
    mainSizer->Add(buttonSizer, 0, wxEXPAND | wxALL, 10);
    
    SetSizer(mainSizer);
    
    // Load current settings
    SettingsManager& settings = SettingsManager::Instance();
    m_osmFileCtrl->SetValue(settings.GetString("map_osm_file", ""));
}

void SettingsDialog::OnBrowseOSM(wxCommandEvent& event) {
    wxFileDialog fileDialog(this, "Select OSM PBF File", "", "",
                           "OpenStreetMap PBF files (*.osm.pbf)|*.osm.pbf|All files (*.*)|*.*",
                           wxFD_OPEN | wxFD_FILE_MUST_EXIST);
    
    if (fileDialog.ShowModal() == wxID_OK) {
        m_osmFileCtrl->SetValue(fileDialog.GetPath());
    }
}

void SettingsDialog::OnOK(wxCommandEvent& event) {
    // Save settings
    SettingsManager& settings = SettingsManager::Instance();
    settings.SetString("map_osm_file", m_osmFileCtrl->GetValue());
    
    // Notify parent about OSM file change (if it changed)
    // The parent can retrieve the new setting and update the map panel
    
    EndModal(wxID_OK);
}