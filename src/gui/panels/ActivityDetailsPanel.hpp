#pragma once

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
    #include <wx/wx.h>
#endif
#include <wx/stattext.h>
#include <wx/sizer.h>
#include "../models/ActivityData.hpp"

class ActivityDetailsPanel : public wxPanel {
public:
    ActivityDetailsPanel(wxWindow* parent);
    virtual ~ActivityDetailsPanel() = default;

    // Update the panel with activity data
    void SetActivityData(const ActivityDisplayData& data);
    
    // Clear the panel and show "No workout selected"
    void ClearSelection();

private:
    void CreateLayout();

    // UI components
    wxStaticText* m_titleLabel;
    wxStaticText* m_dateLabel;
    wxStaticText* m_sportLabel;
    wxStaticText* m_durationLabel;
    wxStaticText* m_distanceLabel;
    wxStaticText* m_speedPaceLabel;
    wxStaticText* m_heartRateLabel;
    wxStaticText* m_filePathLabel;
    
    wxStaticText* m_dateValue;
    wxStaticText* m_sportValue;
    wxStaticText* m_durationValue;
    wxStaticText* m_distanceValue;
    wxStaticText* m_speedPaceValue;
    wxStaticText* m_heartRateValue;
    wxStaticText* m_filePathValue;
    
    wxBoxSizer* m_mainSizer;
    wxFlexGridSizer* m_gridSizer;
    wxStaticText* m_noSelectionText;
    
    bool m_hasSelection;
};