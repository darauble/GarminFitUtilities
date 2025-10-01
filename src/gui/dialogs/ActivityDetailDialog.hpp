#pragma once

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
    #include <wx/wx.h>
#endif

class ActivityDetailDialog : public wxDialog {
public:
    ActivityDetailDialog(wxWindow* parent);
    virtual ~ActivityDetailDialog() = default;

private:
    void CreateLayout();
};