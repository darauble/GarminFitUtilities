#pragma once

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
    #include <wx/wx.h>
#endif
#include "../interfaces/IActivityPanel.hpp"

class TimestampEditorPanel : public wxPanel, public IActivityPanel {
public:
    TimestampEditorPanel(wxWindow* parent);
    virtual ~TimestampEditorPanel() = default;

    // IActivityPanel interface
    void OnTabActivated(const std::string& activityFilePath) override;
    void OnTabDeactivated() override;

private:
    void CreateLayout();
};