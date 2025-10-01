#include "GpxEditorPanel.hpp"

GpxEditorPanel::GpxEditorPanel(wxWindow* parent)
    : wxPanel(parent, wxID_ANY) {
    CreateLayout();
}

void GpxEditorPanel::OnTabActivated(const std::string& activityFilePath) {
    // TODO: Implement GPX editing for activity file
    (void)activityFilePath; // Suppress unused parameter warning
}

void GpxEditorPanel::OnTabDeactivated() {
    // Nothing to do
}

void GpxEditorPanel::CreateLayout() {
    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
    wxStaticText* label = new wxStaticText(this, wxID_ANY, "GPX Editor Panel - Coming Soon");
    sizer->Add(label, 1, wxEXPAND | wxALL, 10);
    SetSizer(sizer);
}
