#include <wx/wxprec.h>
#ifndef WX_PRECOMP
    #include <wx/wx.h>
#endif
#include <locale>

#include "MainFrame.hpp"

class GarminSportsManagerApp : public wxApp {
public:
    virtual bool OnInit() override;
};

bool GarminSportsManagerApp::OnInit() {
    if (!wxApp::OnInit()) {
        return false;
    }

    // Initialize image handlers
    wxInitAllImageHandlers();

    // Set UTF-8 locale
    std::setlocale(LC_ALL, "");
    
    // Ensure wxWidgets uses UTF-8 encoding
    wxLocale::AddCatalogLookupPathPrefix(".");

    // Create and show the main frame
    MainFrame* frame = new MainFrame();
    frame->Show(true);

    return true;
}

wxIMPLEMENT_APP(GarminSportsManagerApp);