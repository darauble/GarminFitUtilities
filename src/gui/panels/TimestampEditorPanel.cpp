#include "TimestampEditorPanel.hpp"

TimestampEditorPanel::TimestampEditorPanel(wxWindow* parent)
    : wxPanel(parent, wxID_ANY) {
    CreateLayout();
}

void TimestampEditorPanel::OnTabActivated(const std::string& activityFilePath) {
    // TODO: Implement timestamp editing for activity file
    (void)activityFilePath; // Suppress unused parameter warning
}

void TimestampEditorPanel::OnTabDeactivated() {
    // Nothing to do
}

void TimestampEditorPanel::CreateLayout() {
    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
    wxStaticText* label = new wxStaticText(this, wxID_ANY, "Timestamp Editor Panel - Coming Soon");
    sizer->Add(label, 1, wxEXPAND | wxALL, 10);
    SetSizer(sizer);
}