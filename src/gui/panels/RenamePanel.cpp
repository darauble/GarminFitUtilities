#include "RenamePanel.hpp"

RenamePanel::RenamePanel(wxWindow* parent)
    : wxPanel(parent, wxID_ANY) {
    CreateLayout();
}

void RenamePanel::OnTabActivated(const std::string& activityFilePath) {
    // TODO: Implement file rename for activity file
    (void)activityFilePath; // Suppress unused parameter warning
}

void RenamePanel::OnTabDeactivated() {
    // Nothing to do
}

void RenamePanel::CreateLayout() {
    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
    wxStaticText* label = new wxStaticText(this, wxID_ANY, "File Rename Panel - Coming Soon");
    sizer->Add(label, 1, wxEXPAND | wxALL, 10);
    SetSizer(sizer);
}
