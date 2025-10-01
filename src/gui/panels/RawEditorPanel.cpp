#include "RawEditorPanel.hpp"

RawEditorPanel::RawEditorPanel(wxWindow* parent)
    : wxPanel(parent, wxID_ANY) {
    CreateLayout();
}

void RawEditorPanel::OnTabActivated(const std::string& activityFilePath) {
    // TODO: Implement raw editing for activity file
    (void)activityFilePath; // Suppress unused parameter warning
}

void RawEditorPanel::OnTabDeactivated() {
    // Nothing to do
}

void RawEditorPanel::CreateLayout() {
    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
    wxStaticText* label = new wxStaticText(this, wxID_ANY, "Raw Editor Panel - Coming Soon");
    sizer->Add(label, 1, wxEXPAND | wxALL, 10);
    SetSizer(sizer);
}
