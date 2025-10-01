#pragma once

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
    #include <wx/wx.h>
#endif
#include <wx/listctrl.h>
#include <wx/spinctrl.h>
#include <vector>
#include <string>
#include "../interfaces/IActivityPanel.hpp"

struct FavoriteProduct {
    int id;
    std::string name;
};

class ProductEditorPanel : public wxPanel, public IActivityPanel {
public:
    ProductEditorPanel(wxWindow* parent);
    virtual ~ProductEditorPanel() = default;

    // IActivityPanel interface
    void OnTabActivated(const std::string& activityFilePath) override;
    void OnTabDeactivated() override;

    void LoadFile(const std::string& filePath);
    void ClearSelection();

private:
    void CreateLayout();
    void OnApply(wxCommandEvent& event);
    void OnAddFavorite(wxCommandEvent& event);
    void OnFavoriteSelected(wxListEvent& event);
    void UpdateCurrentProductDisplay();
    void LoadFavoriteProducts();
    void SaveFavoriteProducts();
    
    // UI components
    wxBoxSizer* m_mainSizer;
    wxStaticText* m_titleLabel;
    wxStaticText* m_filePathLabel;
    wxStaticText* m_filePathValue;
    wxStaticText* m_currentProductLabel;
    wxStaticText* m_currentProductValue;
    wxStaticText* m_newProductLabel;
    wxSpinCtrl* m_newProductInput;
    wxButton* m_applyButton;
    wxStaticText* m_favoriteNameLabel;
    wxTextCtrl* m_favoriteNameInput;
    wxButton* m_addFavoriteButton;
    wxListCtrl* m_favoritesList;
    
    // Data
    std::string m_currentFilePath;
    int m_currentProductId;
    bool m_hasSelection;
    std::vector<FavoriteProduct> m_favoriteProducts;
    
    // Events
    wxDECLARE_EVENT_TABLE();
};