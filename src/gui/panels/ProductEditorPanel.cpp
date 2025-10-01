#include "ProductEditorPanel.hpp"
#include "../utils/SettingsManager.hpp"
#include "../../parsers/binary-mapper.hpp"
#include "../../parsers/product-scanner.hpp"
#include <fit.hpp>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/msgdlg.h>
#include <wx/filedlg.h>
#include <sstream>
#include <iostream>

enum {
    ID_APPLY = 2000,
    ID_ADD_FAVORITE,
    ID_FAVORITES_LIST
};

wxBEGIN_EVENT_TABLE(ProductEditorPanel, wxPanel)
    EVT_BUTTON(ID_APPLY, ProductEditorPanel::OnApply)
    EVT_BUTTON(ID_ADD_FAVORITE, ProductEditorPanel::OnAddFavorite)
    EVT_LIST_ITEM_SELECTED(ID_FAVORITES_LIST, ProductEditorPanel::OnFavoriteSelected)
wxEND_EVENT_TABLE()

ProductEditorPanel::ProductEditorPanel(wxWindow* parent)
    : wxPanel(parent, wxID_ANY),
      m_mainSizer(nullptr),
      m_titleLabel(nullptr),
      m_filePathLabel(nullptr),
      m_filePathValue(nullptr),
      m_currentProductLabel(nullptr),
      m_currentProductValue(nullptr),
      m_newProductLabel(nullptr),
      m_newProductInput(nullptr),
      m_applyButton(nullptr),
      m_favoriteNameLabel(nullptr),
      m_favoriteNameInput(nullptr),
      m_addFavoriteButton(nullptr),
      m_favoritesList(nullptr),
      m_currentProductId(-1),
      m_hasSelection(false) {
    
    CreateLayout();
    LoadFavoriteProducts();
    ClearSelection();
}

void ProductEditorPanel::CreateLayout() {
    m_mainSizer = new wxBoxSizer(wxVERTICAL);
    
    // Title
    m_titleLabel = new wxStaticText(this, wxID_ANY, "Product Editor");
    wxFont titleFont = m_titleLabel->GetFont();
    titleFont.SetPointSize(titleFont.GetPointSize() + 2);
    titleFont.SetWeight(wxFONTWEIGHT_BOLD);
    m_titleLabel->SetFont(titleFont);
    m_mainSizer->Add(m_titleLabel, 0, wxALL | wxALIGN_CENTER_HORIZONTAL, 10);
    
    // Main horizontal sizer for left and right panels
    wxBoxSizer* horizontalSizer = new wxBoxSizer(wxHORIZONTAL);
    
    // Left panel - File info and editing
    wxBoxSizer* leftSizer = new wxBoxSizer(wxVERTICAL);
    leftSizer->Add(new wxStaticText(this, wxID_ANY, "File Information"), 0, wxALL | wxALIGN_LEFT, 5);
    
    // File info grid
    wxFlexGridSizer* infoGrid = new wxFlexGridSizer(2, 2, 5, 15);
    infoGrid->AddGrowableCol(1);
    
    m_filePathLabel = new wxStaticText(this, wxID_ANY, "File:");
    m_currentProductLabel = new wxStaticText(this, wxID_ANY, "Current Product ID:");
    
    wxFont labelFont = m_filePathLabel->GetFont();
    labelFont.SetWeight(wxFONTWEIGHT_BOLD);
    m_filePathLabel->SetFont(labelFont);
    m_currentProductLabel->SetFont(labelFont);
    
    m_filePathValue = new wxStaticText(this, wxID_ANY, "No file selected");
    m_currentProductValue = new wxStaticText(this, wxID_ANY, "-");
    
    infoGrid->Add(m_filePathLabel, 0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL);
    infoGrid->Add(m_filePathValue, 0, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxEXPAND);
    infoGrid->Add(m_currentProductLabel, 0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL);
    infoGrid->Add(m_currentProductValue, 0, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxEXPAND);
    
    leftSizer->Add(infoGrid, 0, wxALL | wxEXPAND, 10);
    
    // Editing controls
    leftSizer->Add(new wxStaticText(this, wxID_ANY, "Replace Product ID"), 0, wxALL | wxALIGN_LEFT, 5);
    
    wxBoxSizer* editSizer = new wxBoxSizer(wxHORIZONTAL);
    m_newProductLabel = new wxStaticText(this, wxID_ANY, "New Product ID:");
    m_newProductInput = new wxSpinCtrl(this, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, 
                                       wxSP_ARROW_KEYS, 0, FIT_UINT16_INVALID, 0);
    m_applyButton = new wxButton(this, ID_APPLY, "Apply");
    
    editSizer->Add(m_newProductLabel, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 10);
    editSizer->Add(m_newProductInput, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 10);
    editSizer->Add(m_applyButton, 0, wxALIGN_CENTER_VERTICAL);
    
    leftSizer->Add(editSizer, 0, wxALL | wxEXPAND, 10);
    
    // Favorite products management
    leftSizer->Add(new wxStaticText(this, wxID_ANY, "Add to Favorites"), 0, wxALL | wxALIGN_LEFT, 5);
    
    wxBoxSizer* favSizer = new wxBoxSizer(wxHORIZONTAL);
    m_favoriteNameLabel = new wxStaticText(this, wxID_ANY, "Name:");
    m_favoriteNameInput = new wxTextCtrl(this, wxID_ANY, "", wxDefaultPosition, wxSize(150, -1));
    m_addFavoriteButton = new wxButton(this, ID_ADD_FAVORITE, "Add");
    
    favSizer->Add(m_favoriteNameLabel, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 10);
    favSizer->Add(m_favoriteNameInput, 1, wxALIGN_CENTER_VERTICAL | wxRIGHT, 10);
    favSizer->Add(m_addFavoriteButton, 0, wxALIGN_CENTER_VERTICAL);
    
    leftSizer->Add(favSizer, 0, wxALL | wxEXPAND, 10);
    
    horizontalSizer->Add(leftSizer, 1, wxALL | wxEXPAND, 10);
    
    // Right panel - Favorites list
    wxBoxSizer* rightSizer = new wxBoxSizer(wxVERTICAL);
    rightSizer->Add(new wxStaticText(this, wxID_ANY, "Favorite Products"), 0, wxALL | wxALIGN_LEFT, 5);
    
    m_favoritesList = new wxListCtrl(this, ID_FAVORITES_LIST, wxDefaultPosition, wxSize(250, 300), 
                                     wxLC_REPORT | wxLC_SINGLE_SEL);
    m_favoritesList->AppendColumn("ID", wxLIST_FORMAT_LEFT, 60);
    m_favoritesList->AppendColumn("Name", wxLIST_FORMAT_LEFT, 180);
    
    rightSizer->Add(m_favoritesList, 1, wxALL | wxEXPAND, 10);
    
    horizontalSizer->Add(rightSizer, 0, wxALL | wxEXPAND, 10);
    
    m_mainSizer->Add(horizontalSizer, 1, wxEXPAND);
    SetSizer(m_mainSizer);
}

void ProductEditorPanel::LoadFile(const std::string& filePath) {
    try {
        m_currentFilePath = filePath;
        m_hasSelection = true;
        
        // Use BinaryMapper and ProductScanner to read current product ID
        darauble::BinaryMapper mapper(filePath.c_str());
        mapper.parse();
        
        if (mapper.isParsed()) {
            darauble::ProductScanner scanner(mapper);
            scanner.scan();
            
            // Get the first product ID found (from file_id message)
            const auto& productIds = scanner.productIds();
            if (!productIds.empty()) {
                // Read the product ID from the first offset (need mutable copy for readU16)
                uint64_t offset = productIds[0].offset;
                uint16_t productId = mapper.readU16(offset, productIds[0].architecture);
                m_currentProductId = productId;
            } else {
                m_currentProductId = -1;
            }
        } else {
            m_currentProductId = -1;
        }
        
        UpdateCurrentProductDisplay();
        
    } catch (const std::exception& e) {
        m_currentProductId = -1;
        m_hasSelection = false;
        wxMessageBox(wxString::Format("Error reading file: %s", e.what()), 
                    "Error", wxOK | wxICON_ERROR, this);
        ClearSelection();
    }
}

void ProductEditorPanel::OnTabActivated(const std::string& activityFilePath) {
    if (activityFilePath.empty()) {
        ClearSelection();
    } else {
        LoadFile(activityFilePath);
    }
}

void ProductEditorPanel::OnTabDeactivated() {
    // Nothing to do on deactivation
}

void ProductEditorPanel::ClearSelection() {
    m_hasSelection = false;
    m_currentFilePath.clear();
    m_currentProductId = -1;
    UpdateCurrentProductDisplay();
}

void ProductEditorPanel::UpdateCurrentProductDisplay() {
    if (m_hasSelection && !m_currentFilePath.empty()) {
        // Extract just the filename from the full path
        size_t pos = m_currentFilePath.find_last_of("/\\");
        std::string filename = (pos != std::string::npos) ? 
            m_currentFilePath.substr(pos + 1) : m_currentFilePath;
        
        m_filePathValue->SetLabel(filename);
        
        if (m_currentProductId >= 0) {
            m_currentProductValue->SetLabel(wxString::Format("%d", m_currentProductId));
            m_newProductInput->SetValue(m_currentProductId);
        } else {
            m_currentProductValue->SetLabel("Unable to read");
            m_newProductInput->SetValue(0);
        }
        
        m_applyButton->Enable(true);
        m_addFavoriteButton->Enable(true);
    } else {
        m_filePathValue->SetLabel("No file selected");
        m_currentProductValue->SetLabel("-");
        m_newProductInput->SetValue(0);
        m_applyButton->Enable(false);
        m_addFavoriteButton->Enable(false);
    }
    
    Layout();
}

void ProductEditorPanel::OnApply(wxCommandEvent& event) {
    if (!m_hasSelection || m_currentFilePath.empty()) {
        wxMessageBox("No file selected", "Error", wxOK | wxICON_ERROR, this);
        return;
    }
    
    int newProductId = m_newProductInput->GetValue();
    if (newProductId == m_currentProductId) {
        wxMessageBox("New product ID is the same as current product ID", "Info", wxOK | wxICON_INFORMATION, this);
        return;
    }
    
    // Ask for output file
    wxFileName currentFile(m_currentFilePath);
    wxFileDialog saveDialog(this, "Save modified file as", 
                           currentFile.GetPath(),  // Same directory as input file
                           currentFile.GetName(),  // Same filename as input file
                           "FIT files (*.fit)|*.fit|All files (*.*)|*.*",
                           wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
    
    if (saveDialog.ShowModal() == wxID_CANCEL) {
        return;
    }
    
    std::string outputPath = saveDialog.GetPath().ToStdString();
    
    try {
        // Use the same logic as ProductCommand::replace
        darauble::BinaryMapper mapper(m_currentFilePath.c_str());
        mapper.parse();
        
        if (mapper.isParsed()) {
            uint16_t oldProductId = static_cast<uint16_t>(m_currentProductId);
            uint16_t newProductIdU16 = static_cast<uint16_t>(newProductId);
            
            darauble::ProductScanner scanner(mapper, oldProductId);
            scanner.scan();
            
            // Modify all matching product IDs
            for (const auto& productId : scanner.productIds()) {
                uint64_t offset = productId.offset;
                mapper.write(offset, newProductIdU16, productId.architecture);
            }
            
            mapper.writeCRC();
            mapper.save(outputPath.c_str());
            
            wxMessageBox(wxString::Format("Product ID successfully changed from %d to %d\nSaved to: %s", 
                        oldProductId, newProductId, outputPath), 
                        "Success", wxOK | wxICON_INFORMATION, this);
            
        } else {
            wxMessageBox("Failed to parse FIT file", "Error", wxOK | wxICON_ERROR, this);
        }
        
    } catch (const std::exception& e) {
        wxMessageBox(wxString::Format("Error processing file: %s", e.what()), 
                    "Error", wxOK | wxICON_ERROR, this);
    }
}

void ProductEditorPanel::OnAddFavorite(wxCommandEvent& event) {
    if (!m_hasSelection) {
        wxMessageBox("No file selected", "Error", wxOK | wxICON_ERROR, this);
        return;
    }
    
    std::string name = m_favoriteNameInput->GetValue().ToStdString();
    if (name.empty()) {
        wxMessageBox("Please enter a name for the product", "Error", wxOK | wxICON_ERROR, this);
        return;
    }
    
    int productId = m_newProductInput->GetValue();
    
    // Check if product ID already exists
    for (const auto& fav : m_favoriteProducts) {
        if (fav.id == productId) {
            wxMessageBox("Product ID already exists in favorites", "Error", wxOK | wxICON_ERROR, this);
            return;
        }
    }
    
    // Add to favorites
    m_favoriteProducts.push_back({productId, name});
    SaveFavoriteProducts();
    
    // Add to list control
    long index = m_favoritesList->GetItemCount();
    m_favoritesList->InsertItem(index, wxString::Format("%d", productId));
    m_favoritesList->SetItem(index, 1, name);
    
    // Clear input
    m_favoriteNameInput->Clear();
    
    wxMessageBox("Product added to favorites", "Success", wxOK | wxICON_INFORMATION, this);
}

void ProductEditorPanel::OnFavoriteSelected(wxListEvent& event) {
    long selectedIndex = event.GetIndex();
    if (selectedIndex >= 0 && selectedIndex < static_cast<long>(m_favoriteProducts.size())) {
        const FavoriteProduct& product = m_favoriteProducts[selectedIndex];
        m_newProductInput->SetValue(product.id);
    }
}

void ProductEditorPanel::LoadFavoriteProducts() {
    m_favoriteProducts.clear();
    m_favoritesList->DeleteAllItems();
    
    auto& settings = SettingsManager::Instance();
    int count = settings.GetInt("FavoriteProducts/Count", 0);
    
    for (int i = 0; i < count; i++) {
        int id = settings.GetInt(wxString::Format("FavoriteProducts/Product%d/ID", i), -1);
        std::string name = settings.GetString(wxString::Format("FavoriteProducts/Product%d/Name", i), "").ToStdString();
        
        if (id >= 0 && !name.empty()) {
            m_favoriteProducts.push_back({id, name});
            
            long index = m_favoritesList->GetItemCount();
            m_favoritesList->InsertItem(index, wxString::Format("%d", id));
            m_favoritesList->SetItem(index, 1, name);
        }
    }
}

void ProductEditorPanel::SaveFavoriteProducts() {
    auto& settings = SettingsManager::Instance();
    settings.SetInt("FavoriteProducts/Count", static_cast<int>(m_favoriteProducts.size()));
    
    for (size_t i = 0; i < m_favoriteProducts.size(); i++) {
        settings.SetInt(wxString::Format("FavoriteProducts/Product%zu/ID", i), m_favoriteProducts[i].id);
        settings.SetString(wxString::Format("FavoriteProducts/Product%zu/Name", i), m_favoriteProducts[i].name);
    }
}