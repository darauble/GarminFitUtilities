#pragma once

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
    #include <wx/wx.h>
#endif

#include <wx/panel.h>
#include <wx/bitmap.h>
#include <vector>
#include <string>
#include <memory>
#include "../interfaces/IActivityPanel.hpp"

// Forward declarations
class MapnikRenderer;

struct GPSPoint {
    double latitude;
    double longitude;
    double altitude = 0.0;
    uint32_t timestamp = 0;
};

struct BoundingBox {
    double min_lat, max_lat;
    double min_lon, max_lon;
    
    BoundingBox() : min_lat(90), max_lat(-90), min_lon(180), max_lon(-180) {}
    
    void extend(double lat, double lon) {
        min_lat = std::min(min_lat, lat);
        max_lat = std::max(max_lat, lat);
        min_lon = std::min(min_lon, lon);
        max_lon = std::max(max_lon, lon);
    }
    
    bool isValid() const {
        return min_lat <= max_lat && min_lon <= max_lon;
    }
    
    void addPadding(double factor = 0.1) {
        if (!isValid()) return;
        double lat_padding = (max_lat - min_lat) * factor;
        double lon_padding = (max_lon - min_lon) * factor;
        min_lat -= lat_padding;
        max_lat += lat_padding;
        min_lon -= lon_padding;
        max_lon += lon_padding;
    }
};

class MapPanel : public wxPanel, public IActivityPanel {
public:
    MapPanel(wxWindow* parent);
    virtual ~MapPanel();

    // IActivityPanel interface
    void OnTabActivated(const std::string& activityFilePath) override;
    void OnTabDeactivated() override;

    // Main interface
    void SetTrack(const std::vector<GPSPoint>& track, const std::string& activityName = "");
    void LoadTrack(const std::string& fitFilePath);
    void SetOSMFile(const std::string& pbf_path);
    void ClearMap();
    
    // Navigation
    void ZoomToTrack();
    void ZoomIn();
    void ZoomOut();
    void ResetView();
    
    // Status
    bool HasTrack() const { return !m_currentTrack.empty(); }
    bool HasOSMData() const { return m_osmLoaded; }
    
private:
    // Event handlers
    void OnPaint(wxPaintEvent& event);
    void OnSize(wxSizeEvent& event);
    void OnMouseWheel(wxMouseEvent& event);
    void OnMouseDown(wxMouseEvent& event);
    void OnMouseMove(wxMouseEvent& event);
    void OnMouseUp(wxMouseEvent& event);
    void OnRightClick(wxMouseEvent& event);
    
    // Rendering
    void RenderMap();
    void RenderTrackOnly();
    void InvalidateCache();
    BoundingBox CalculateTrackBounds() const;

    // Map data conversion
    bool CheckAndConvertPBFToSpatialite(const std::string& pbf_path);
    std::string GenerateStylesheetForSpatialite(const std::string& sqlite_path);
    
    // Coordinate conversion
    std::pair<double, double> ScreenToWorld(int screen_x, int screen_y) const;
    std::pair<int, int> WorldToScreen(double world_x, double world_y) const;
    
    // Data members
    std::vector<GPSPoint> m_currentTrack;
    std::string m_activityName;
    std::string m_osmFilePath;
    bool m_osmLoaded;
    
    // Rendering components
    std::unique_ptr<MapnikRenderer> m_renderer;
    
    // Display state
    wxBitmap m_cachedBitmap;
    bool m_cacheValid;
    
    // View state
    double m_centerLat, m_centerLon;
    double m_zoomLevel;
    BoundingBox m_viewBounds;
    
    // Interaction state
    bool m_dragging;
    wxPoint m_lastMousePos;

    // Rendering throttle
    bool m_renderPending;
    wxLongLong m_lastRenderTime;

    wxDECLARE_EVENT_TABLE();
};