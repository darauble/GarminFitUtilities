#pragma once

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
    #include <wx/wx.h>
#endif
#include "../interfaces/IActivityPanel.hpp"

class GpxEditorPanel : public wxPanel, public IActivityPanel {
public:
    GpxEditorPanel(wxWindow* parent);
    virtual ~GpxEditorPanel() = default;

    // IActivityPanel interface
    void OnTabActivated(const std::string& activityFilePath) override;
    void OnTabDeactivated() override;

private:
    void CreateLayout();
};
