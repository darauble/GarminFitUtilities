#include "ActivityDetailsPanel.hpp"
#include <wx/sizer.h>
#include <wx/stattext.h>

ActivityDetailsPanel::ActivityDetailsPanel(wxWindow* parent)
    : wxPanel(parent, wxID_ANY),
      m_titleLabel(nullptr),
      m_dateLabel(nullptr),
      m_sportLabel(nullptr),
      m_durationLabel(nullptr),
      m_distanceLabel(nullptr),
      m_speedPaceLabel(nullptr),
      m_heartRateLabel(nullptr),
      m_filePathLabel(nullptr),
      m_dateValue(nullptr),
      m_sportValue(nullptr),
      m_durationValue(nullptr),
      m_distanceValue(nullptr),
      m_speedPaceValue(nullptr),
      m_heartRateValue(nullptr),
      m_filePathValue(nullptr),
      m_mainSizer(nullptr),
      m_gridSizer(nullptr),
      m_noSelectionText(nullptr),
      m_hasSelection(false) {
    
    CreateLayout();
    ClearSelection(); // Start with no selection
}

void ActivityDetailsPanel::CreateLayout() {
    m_mainSizer = new wxBoxSizer(wxVERTICAL);
    
    // Title
    m_titleLabel = new wxStaticText(this, wxID_ANY, "Activity Details");
    wxFont titleFont = m_titleLabel->GetFont();
    titleFont.SetPointSize(titleFont.GetPointSize() + 2);
    titleFont.SetWeight(wxFONTWEIGHT_BOLD);
    m_titleLabel->SetFont(titleFont);
    
    m_mainSizer->Add(m_titleLabel, 0, wxALL | wxALIGN_CENTER_HORIZONTAL, 10);
    
    // Grid for labels and values
    m_gridSizer = new wxFlexGridSizer(7, 2, 5, 15); // 7 rows, 2 cols, 5px vertical gap, 15px horizontal gap
    m_gridSizer->AddGrowableCol(1); // Make the value column expandable
    
    // Create labels (left column)
    m_dateLabel = new wxStaticText(this, wxID_ANY, "Date:");
    m_sportLabel = new wxStaticText(this, wxID_ANY, "Sport:");
    m_durationLabel = new wxStaticText(this, wxID_ANY, "Total Time:");
    m_distanceLabel = new wxStaticText(this, wxID_ANY, "Distance:");
    m_speedPaceLabel = new wxStaticText(this, wxID_ANY, "Speed/Pace:");
    m_heartRateLabel = new wxStaticText(this, wxID_ANY, "Average Heart Rate:");
    m_filePathLabel = new wxStaticText(this, wxID_ANY, "File:");
    
    // Make labels bold
    wxFont labelFont = m_dateLabel->GetFont();
    labelFont.SetWeight(wxFONTWEIGHT_BOLD);
    m_dateLabel->SetFont(labelFont);
    m_sportLabel->SetFont(labelFont);
    m_durationLabel->SetFont(labelFont);
    m_distanceLabel->SetFont(labelFont);
    m_speedPaceLabel->SetFont(labelFont);
    m_heartRateLabel->SetFont(labelFont);
    m_filePathLabel->SetFont(labelFont);
    
    // Create value labels (right column)
    m_dateValue = new wxStaticText(this, wxID_ANY, "");
    m_sportValue = new wxStaticText(this, wxID_ANY, "");
    m_durationValue = new wxStaticText(this, wxID_ANY, "");
    m_distanceValue = new wxStaticText(this, wxID_ANY, "");
    m_speedPaceValue = new wxStaticText(this, wxID_ANY, "");
    m_heartRateValue = new wxStaticText(this, wxID_ANY, "");
    m_filePathValue = new wxStaticText(this, wxID_ANY, "");
    
    // Add to grid
    m_gridSizer->Add(m_dateLabel, 0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL);
    m_gridSizer->Add(m_dateValue, 0, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxEXPAND);
    
    m_gridSizer->Add(m_sportLabel, 0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL);
    m_gridSizer->Add(m_sportValue, 0, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxEXPAND);
    
    m_gridSizer->Add(m_durationLabel, 0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL);
    m_gridSizer->Add(m_durationValue, 0, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxEXPAND);
    
    m_gridSizer->Add(m_distanceLabel, 0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL);
    m_gridSizer->Add(m_distanceValue, 0, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxEXPAND);
    
    m_gridSizer->Add(m_speedPaceLabel, 0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL);
    m_gridSizer->Add(m_speedPaceValue, 0, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxEXPAND);
    
    m_gridSizer->Add(m_heartRateLabel, 0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL);
    m_gridSizer->Add(m_heartRateValue, 0, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxEXPAND);
    
    m_gridSizer->Add(m_filePathLabel, 0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL);
    m_gridSizer->Add(m_filePathValue, 0, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxEXPAND);
    
    m_mainSizer->Add(m_gridSizer, 0, wxALL | wxEXPAND, 20);
    
    SetSizer(m_mainSizer);
}

void ActivityDetailsPanel::SetActivityData(const ActivityDisplayData& data) {
    m_hasSelection = true;
    
    m_dateValue->SetLabel(data.date);
    m_sportValue->SetLabel(data.sport);
    m_durationValue->SetLabel(data.duration);
    m_distanceValue->SetLabel(data.distance);
    m_speedPaceValue->SetLabel(data.speedPace);
    m_heartRateValue->SetLabel(data.heartRate);
    m_filePathValue->SetLabel(data.filePath);
    
    // Show the grid and hide the "no selection" message
    m_gridSizer->ShowItems(true);
    if (m_noSelectionText) {
        m_noSelectionText->Hide();
    }
    
    Layout();
}

void ActivityDetailsPanel::ClearSelection() {
    m_hasSelection = false;
    
    // Clear all values
    m_dateValue->SetLabel("");
    m_sportValue->SetLabel("");
    m_durationValue->SetLabel("");
    m_distanceValue->SetLabel("");
    m_speedPaceValue->SetLabel("");
    m_heartRateValue->SetLabel("");
    m_filePathValue->SetLabel("");
    
    // Hide the grid
    m_gridSizer->ShowItems(false);
    
    // Create "no selection" message if it doesn't exist
    if (!m_noSelectionText) {
        m_noSelectionText = new wxStaticText(this, wxID_ANY, "No workout selected");
        wxFont font = m_noSelectionText->GetFont();
        font.SetPointSize(font.GetPointSize() + 2);
        font.SetStyle(wxFONTSTYLE_ITALIC);
        m_noSelectionText->SetFont(font);
        m_noSelectionText->SetForegroundColour(wxColour(128, 128, 128));
        
        m_mainSizer->Add(m_noSelectionText, 1, wxALL | wxALIGN_CENTER, 20);
    } else {
        // Show the existing "no selection" message
        m_noSelectionText->Show();
    }
    
    Layout();
}