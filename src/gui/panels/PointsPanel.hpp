#pragma once

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
    #include <wx/wx.h>
#endif
#include "../interfaces/IActivityPanel.hpp"

class PointsPanel : public wxPanel, public IActivityPanel {
public:
    PointsPanel(wxWindow* parent);
    virtual ~PointsPanel() = default;

    // IActivityPanel interface
    void OnTabActivated(const std::string& activityFilePath) override;
    void OnTabDeactivated() override;

private:
    void CreateLayout();
};
