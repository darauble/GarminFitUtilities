#include "ActivityDetailDialog.hpp"

ActivityDetailDialog::ActivityDetailDialog(wxWindow* parent)
    : wxDialog(parent, wxID_ANY, "Activity Details", wxDefaultPosition, wxSize(600, 400)) {
    CreateLayout();
}

void ActivityDetailDialog::CreateLayout() {
    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
    wxStaticText* label = new wxStaticText(this, wxID_ANY, "Activity Detail Dialog - Coming Soon");
    sizer->Add(label, 1, wxEXPAND | wxALL, 10);
    
    wxButton* okButton = new wxButton(this, wxID_OK, "OK");
    sizer->Add(okButton, 0, wxALIGN_CENTER | wxALL, 5);
    
    SetSizer(sizer);
}