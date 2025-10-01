#include "PointsPanel.hpp"

PointsPanel::PointsPanel(wxWindow* parent)
    : wxPanel(parent, wxID_ANY) {
    CreateLayout();
}

void PointsPanel::OnTabActivated(const std::string& activityFilePath) {
    // TODO: Implement points & mapping for activity file
    (void)activityFilePath; // Suppress unused parameter warning
}

void PointsPanel::OnTabDeactivated() {
    // Nothing to do
}

void PointsPanel::CreateLayout() {
    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
    wxStaticText* label = new wxStaticText(this, wxID_ANY, "Points & Mapping Panel - Coming Soon");
    sizer->Add(label, 1, wxEXPAND | wxALL, 10);
    SetSizer(sizer);
}
