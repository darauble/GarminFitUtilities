#include "MapPanel.hpp"
#include "MapRenderer.hpp"
#include "utils/SettingsManager.hpp"
#include "utils/DataDirectoryResolver.hpp"
#include "parsers/coordinates-scanner.hpp"
#include "parsers/binary-mapper.hpp"
#include <wx/msgdlg.h>
#include <wx/menu.h>
#include <wx/progdlg.h>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <sstream>

enum {
    ID_ZOOM_IN = 3000,
    ID_ZOOM_OUT,
    ID_ZOOM_TO_TRACK,
    ID_RESET_VIEW
};

wxBEGIN_EVENT_TABLE(MapPanel, wxPanel)
    EVT_PAINT(MapPanel::OnPaint)
    EVT_SIZE(MapPanel::OnSize)
    EVT_MOUSEWHEEL(MapPanel::OnMouseWheel)
    EVT_LEFT_DOWN(MapPanel::OnMouseDown)
    EVT_MOTION(MapPanel::OnMouseMove)
    EVT_LEFT_UP(MapPanel::OnMouseUp)
    EVT_RIGHT_DOWN(MapPanel::OnRightClick)
wxEND_EVENT_TABLE()

MapPanel::MapPanel(wxWindow* parent)
    : wxPanel(parent, wxID_ANY),
      m_osmLoaded(false),
      m_cacheValid(false),
      m_centerLat(54.9),
      m_centerLon(23.9),
      m_zoomLevel(10.0),
      m_dragging(false),
      m_renderPending(false),
      m_lastRenderTime(0) {

    SetBackgroundColour(*wxWHITE);

    // Load map data path from settings
    SettingsManager& settings = SettingsManager::Instance();

    // Check for map_osm_file (PBF or Spatialite) first, then fall back to map_stylesheet
    std::string osmFile = settings.GetString("map_osm_file", "").ToStdString();
    if (!osmFile.empty() && std::filesystem::exists(osmFile)) {
        m_osmFilePath = osmFile;
    } else {
        // Try the stylesheet path
        std::string stylesheet = settings.GetString("map_stylesheet", "").ToStdString();
        if (!stylesheet.empty() && std::filesystem::exists(stylesheet)) {
            m_osmFilePath = stylesheet;
        }
    }

    // Initialize renderer as nullptr - will be created on demand
    m_renderer = nullptr;
}

MapPanel::~MapPanel() {
    // Smart pointers will handle cleanup
}

void MapPanel::OnTabActivated(const std::string& activityFilePath) {
    if (activityFilePath.empty()) {
        ClearMap();
    } else {
        LoadTrack(activityFilePath);
    }
}

void MapPanel::OnTabDeactivated() {
    // Nothing to do on deactivation - keep the map cached
}

void MapPanel::SetTrack(const std::vector<GPSPoint>& track, const std::string& activityName) {
    m_currentTrack = track;
    m_activityName = activityName;
    
    InvalidateCache();
    
    if (!track.empty()) {
        ZoomToTrack();
    }
    
    Refresh();
}

void MapPanel::LoadTrack(const std::string& fitFilePath) {
    try {
        // Use the pattern from MapRenderer.cpp
        std::vector<int32_t> latitudes, longitudes;
        
        darauble::BinaryMapper mapper{std::filesystem::path(fitFilePath)};
        darauble::CoordinatesScanner scanner{mapper, FIT_SPORT_ALL, latitudes, longitudes};
        scanner.scan();
        
        // Convert to GPSPoint vector
        std::vector<GPSPoint> track;
        track.reserve(latitudes.size());
        
        for (size_t i = 0; i < latitudes.size() && i < longitudes.size(); ++i) {
            if (latitudes[i] != FIT_SINT32_INVALID && longitudes[i] != FIT_SINT32_INVALID) {
                GPSPoint point;
                point.latitude = latitudes[i] / 11930464.7111; // Convert from semicircles to degrees
                point.longitude = longitudes[i] / 11930464.7111;
                track.push_back(point);
            }
        }
        
        // Extract activity name from file path (just filename without extension)
        std::string activityName = fitFilePath;
        size_t lastSlash = activityName.find_last_of("/\\");
        if (lastSlash != std::string::npos) {
            activityName = activityName.substr(lastSlash + 1);
        }
        size_t lastDot = activityName.find_last_of(".");
        if (lastDot != std::string::npos) {
            activityName = activityName.substr(0, lastDot);
        }
        
        SetTrack(track, activityName);
    } catch (const std::exception& e) {
        // Handle error - clear track
        ClearMap();
    }
}

void MapPanel::SetOSMFile(const std::string& stylesheet_path) {
    if (m_osmFilePath == stylesheet_path) return; // No change

    m_osmFilePath = stylesheet_path;
    m_osmLoaded = false;

    // Save to settings
    SettingsManager& settings = SettingsManager::Instance();
    settings.SetString("map_osm_file", stylesheet_path);

    // Check if this is a PBF file - if so, trigger conversion check
    if (stylesheet_path.ends_with(".pbf") || stylesheet_path.ends_with(".osm.pbf")) {
        // This is a PBF file, need to convert to Spatialite
        CheckAndConvertPBFToSpatialite(stylesheet_path);
        return; // CheckAndConvertPBFToSpatialite handles initialization
    }

    // Check if this is a Spatialite file - generate stylesheet for it
    if (stylesheet_path.ends_with(".sqlite")) {
        std::string generated_stylesheet = GenerateStylesheetForSpatialite(stylesheet_path);
        if (!generated_stylesheet.empty() && m_renderer) {
            m_osmLoaded = m_renderer->initialize(generated_stylesheet);
            InvalidateCache();
            if (HasTrack()) {
                CallAfter([this]() { RenderMap(); });
            }
        }
        return;
    }

    // Otherwise assume it's a stylesheet XML file - use it directly
    settings.SetString("map_stylesheet", stylesheet_path);
    if (m_renderer) {
        m_osmLoaded = m_renderer->initialize(stylesheet_path);
        InvalidateCache();

        if (HasTrack()) {
            CallAfter([this]() { RenderMap(); });
        }
    }
}

void MapPanel::ClearMap() {
    m_currentTrack.clear();
    m_activityName.clear();
    InvalidateCache();
    Refresh();
}

void MapPanel::ZoomToTrack() {
    if (m_currentTrack.empty()) return;

    BoundingBox bounds = CalculateTrackBounds();
    if (!bounds.isValid()) return;

    bounds.addPadding(0.15); // 15% padding

    m_centerLat = (bounds.min_lat + bounds.max_lat) / 2.0;
    m_centerLon = (bounds.min_lon + bounds.max_lon) / 2.0;

    // Calculate zoom level to fit bounds
    wxSize size = GetSize();
    if (size.x > 0 && size.y > 0) {
        // Calculate the required scale to fit the track
        double width_deg = bounds.max_lon - bounds.min_lon;
        double height_deg = bounds.max_lat - bounds.min_lat;

        // Avoid division by zero
        if (width_deg <= 0) width_deg = 0.001;
        if (height_deg <= 0) height_deg = 0.001;

        // Calculate zoom level based on degrees per pixel
        // zoom_level * 111000 = pixels per degree (approximately)
        double zoom_x = size.x / (width_deg * 111000.0);
        double zoom_y = size.y / (height_deg * 111000.0);

        m_zoomLevel = std::min(zoom_x, zoom_y);
        m_zoomLevel = std::max(0.000001, std::min(1.0, m_zoomLevel));
    }

    InvalidateCache();
    RenderMap();
}

void MapPanel::ZoomIn() {
    m_zoomLevel *= 1.5;
    InvalidateCache();
    RenderMap();
}

void MapPanel::ZoomOut() {
    m_zoomLevel /= 1.5;
    InvalidateCache();
    RenderMap();
}

void MapPanel::ResetView() {
    if (!m_currentTrack.empty()) {
        ZoomToTrack();
    } else {
        m_centerLat = 0.0;
        m_centerLon = 0.0;
        m_zoomLevel = 1.0;
        InvalidateCache();
        RenderMap();
    }
}

void MapPanel::OnPaint(wxPaintEvent& event) {
    wxPaintDC dc(this);
    
    if (m_cachedBitmap.IsOk()) {
        dc.DrawBitmap(m_cachedBitmap, 0, 0);
    } else {
        // No cached bitmap - render on demand
        dc.SetBackground(*wxLIGHT_GREY_BRUSH);
        dc.Clear();
        
        if (m_currentTrack.empty()) {
            dc.SetTextForeground(*wxBLACK);
            dc.DrawText("No track loaded", 10, 10);
            dc.DrawText("Select an activity to view its track", 10, 30);
        } else {
            dc.DrawText("Rendering map...", 10, 10);
            // Trigger rendering on next idle
            CallAfter([this]() { RenderMap(); });
        }
    }
}

void MapPanel::OnSize(wxSizeEvent& event) {
    InvalidateCache();
    if (HasTrack()) {
        // Delay rendering to avoid multiple renders during window resize
        CallAfter([this]() { RenderMap(); });
    }
    event.Skip();
}

void MapPanel::OnMouseWheel(wxMouseEvent& event) {
    if (event.GetWheelRotation() > 0) {
        ZoomIn();
    } else {
        ZoomOut();
    }
    event.Skip();
}

void MapPanel::OnMouseDown(wxMouseEvent& event) {
    m_dragging = true;
    m_lastMousePos = event.GetPosition();
    CaptureMouse();
}

void MapPanel::OnMouseMove(wxMouseEvent& event) {
    if (m_dragging && event.Dragging()) {
        wxPoint currentPos = event.GetPosition();
        wxPoint delta = currentPos - m_lastMousePos;

        // Convert screen delta to degrees
        double degrees_per_pixel = 1.0 / (m_zoomLevel * 111000.0);

        // Apply delta (negative for x because drag right = pan left)
        m_centerLon -= delta.x * degrees_per_pixel;
        m_centerLat += delta.y * degrees_per_pixel; // Positive because screen Y is inverted

        m_lastMousePos = currentPos;

        InvalidateCache();

        // Throttle rendering during drag - only render if not already pending
        if (!m_renderPending) {
            m_renderPending = true;
            CallAfter([this]() {
                RenderMap();
                m_renderPending = false;
            });
        }
    }
}

void MapPanel::OnMouseUp(wxMouseEvent& event) {
    if (m_dragging) {
        m_dragging = false;
        if (HasCapture()) {
            ReleaseMouse();
        }
    }
}

void MapPanel::OnRightClick(wxMouseEvent& event) {
    wxMenu contextMenu;
    
    if (HasTrack()) {
        contextMenu.Append(ID_ZOOM_TO_TRACK, "Zoom to Track");
        contextMenu.AppendSeparator();
    }
    
    contextMenu.Append(ID_ZOOM_IN, "Zoom In");
    contextMenu.Append(ID_ZOOM_OUT, "Zoom Out");
    contextMenu.Append(ID_RESET_VIEW, "Reset View");
    
    contextMenu.Bind(wxEVT_COMMAND_MENU_SELECTED, [this](wxCommandEvent& evt) {
        switch (evt.GetId()) {
            case ID_ZOOM_IN: ZoomIn(); break;
            case ID_ZOOM_OUT: ZoomOut(); break;
            case ID_ZOOM_TO_TRACK: ZoomToTrack(); break;
            case ID_RESET_VIEW: ResetView(); break;
        }
    });
    
    PopupMenu(&contextMenu);
}

void MapPanel::RenderMap() {
    wxSize size = GetSize();
    if (size.x <= 0 || size.y <= 0) return;

    // Show busy cursor during rendering
    wxBusyCursor wait;

    // Create and initialize renderer if needed
    if (!m_renderer) {
        m_renderer = std::make_unique<MapnikRenderer>(size.x, size.y);

        // Initialize with stylesheet if available
        if (!m_osmFilePath.empty()) {
            // Check if this is a PBF file that needs conversion
            if (m_osmFilePath.ends_with(".pbf") || m_osmFilePath.ends_with(".osm.pbf")) {
                // Trigger conversion check (this will initialize the renderer if successful)
                CheckAndConvertPBFToSpatialite(m_osmFilePath);
            } else {
                m_osmLoaded = m_renderer->initialize(m_osmFilePath);
                if (!m_osmLoaded) {
                    wxLogError("Failed to load Mapnik stylesheet from: %s", m_osmFilePath);
                }
            }
        }
    } else {
        // Update size if changed
        m_renderer->setSize(size.x, size.y);
    }

    if (!m_renderer->isValid()) {
        // Fallback to track-only rendering
        RenderTrackOnly();
        return;
    }

    // Calculate bounds for rendering based on current view state
    // The zoom level represents a scale where we calculate degrees per pixel
    // This is simplified - a more accurate approach would use the Mercator scale factor

    // Calculate degrees per pixel based on zoom level
    // At equator: 1 degree ≈ 111 km, and screen coordinates relate to world via zoom
    double degrees_per_pixel = 1.0 / (m_zoomLevel * 111000.0); // rough approximation

    // Calculate bounds in lat/lon
    double half_width_deg = (size.x / 2.0) * degrees_per_pixel;
    double half_height_deg = (size.y / 2.0) * degrees_per_pixel;

    double min_lon = m_centerLon - half_width_deg;
    double max_lon = m_centerLon + half_width_deg;
    double min_lat = m_centerLat - half_height_deg;
    double max_lat = m_centerLat + half_height_deg;

    // Set view bounds (renderer will convert to Mercator internally)
    m_renderer->setBounds(min_lon, min_lat, max_lon, max_lat);

    // Render map with Mapnik
    wxBitmap mapBitmap = m_renderer->render();

    // Overlay GPS track if we have one
    if (HasTrack()) {
        m_renderer->renderTrackOverlay(mapBitmap, m_currentTrack,
                                       min_lon, min_lat, max_lon, max_lat);
    }

    // Draw zoom level indicator
    {
        wxMemoryDC dc(mapBitmap);
        dc.SetFont(wxFont(12, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD));

        // Calculate approximate map scale denominator (similar to Mapnik's scale)
        // At the current latitude, calculate meters per pixel
        double lat_rad = m_centerLat * M_PI / 180.0;
        double meters_per_degree_lon = 111320.0 * cos(lat_rad);
        double meters_per_pixel = degrees_per_pixel * meters_per_degree_lon;
        double scale_denominator = meters_per_pixel * 90.7; // 90.7 is DPI factor (72 / 0.0254 * 0.0254)

        wxString zoomText = wxString::Format("Scale: 1:%.0f (zoom: %.6f)", scale_denominator, m_zoomLevel);

        // Draw semi-transparent background
        wxSize textSize = dc.GetTextExtent(zoomText);
        dc.SetBrush(wxBrush(wxColor(255, 255, 255, 200)));
        dc.SetPen(*wxTRANSPARENT_PEN);
        dc.DrawRectangle(10, 10, textSize.x + 10, textSize.y + 6);

        // Draw text
        dc.SetTextForeground(*wxBLACK);
        dc.DrawText(zoomText, 15, 13);
        dc.SelectObject(wxNullBitmap);
    }

    m_cachedBitmap = mapBitmap;
    m_cacheValid = true;

    Refresh();
    Update();
}

void MapPanel::RenderTrackOnly() {
    wxSize size = GetSize();
    if (size.x <= 0 || size.y <= 0) return;
    
    // Simple fallback rendering without Cairo/OSM
    wxBitmap bitmap(size.x, size.y);
    wxMemoryDC dc(bitmap);
    
    dc.SetBackground(*wxWHITE_BRUSH);
    dc.Clear();
    
    if (HasTrack()) {
        BoundingBox bounds = CalculateTrackBounds();
        if (bounds.isValid()) {
            bounds.addPadding(0.1);
            
            double scale_x = size.x / (bounds.max_lon - bounds.min_lon);
            double scale_y = size.y / (bounds.max_lat - bounds.min_lat);
            double scale = std::min(scale_x, scale_y);
            
            double offset_x = (size.x - (bounds.max_lon - bounds.min_lon) * scale) / 2.0;
            double offset_y = (size.y - (bounds.max_lat - bounds.min_lat) * scale) / 2.0;
            
            // Draw track
            dc.SetPen(wxPen(*wxRED, 2));
            
            if (m_currentTrack.size() > 1) {
                wxPoint prev_pt;
                bool first = true;
                
                for (const auto& point : m_currentTrack) {
                    int x = static_cast<int>((point.longitude - bounds.min_lon) * scale + offset_x);
                    int y = static_cast<int>(size.y - ((point.latitude - bounds.min_lat) * scale + offset_y));
                    
                    if (!first) {
                        dc.DrawLine(prev_pt, wxPoint(x, y));
                    }
                    prev_pt = wxPoint(x, y);
                    first = false;
                }
                
                // Start/end markers
                if (!m_currentTrack.empty()) {
                    auto& start = m_currentTrack.front();
                    auto& end = m_currentTrack.back();
                    
                    int start_x = static_cast<int>((start.longitude - bounds.min_lon) * scale + offset_x);
                    int start_y = static_cast<int>(size.y - ((start.latitude - bounds.min_lat) * scale + offset_y));
                    
                    int end_x = static_cast<int>((end.longitude - bounds.min_lon) * scale + offset_x);
                    int end_y = static_cast<int>(size.y - ((end.latitude - bounds.min_lat) * scale + offset_y));
                    
                    dc.SetBrush(*wxGREEN_BRUSH);
                    dc.DrawCircle(start_x, start_y, 5);
                    
                    dc.SetBrush(*wxRED_BRUSH);
                    dc.DrawCircle(end_x, end_y, 5);
                }
            }
        }
    }
    
    // Activity name
    if (!m_activityName.empty()) {
        dc.SetTextForeground(*wxBLACK);
        dc.DrawText(m_activityName, 10, 10);
    }
    
    dc.DrawText("Track-only mode (no OSM data)", 10, size.y - 30);
    dc.SelectObject(wxNullBitmap);
    
    m_cachedBitmap = bitmap;
    m_cacheValid = true;
    
    Refresh();
}

void MapPanel::InvalidateCache() {
    m_cacheValid = false;
    m_cachedBitmap = wxBitmap();
}

BoundingBox MapPanel::CalculateTrackBounds() const {
    BoundingBox bounds;
    
    for (const auto& point : m_currentTrack) {
        if (point.latitude != 0.0 || point.longitude != 0.0) {
            bounds.extend(point.latitude, point.longitude);
        }
    }
    
    return bounds;
}

std::pair<double, double> MapPanel::ScreenToWorld(int screen_x, int screen_y) const {
    // Simple implementation - could be improved
    wxSize size = GetSize();
    double world_x = m_centerLon + (screen_x - size.x / 2.0) / m_zoomLevel * 0.001;
    double world_y = m_centerLat - (screen_y - size.y / 2.0) / m_zoomLevel * 0.001;
    return {world_x, world_y};
}

std::pair<int, int> MapPanel::WorldToScreen(double world_x, double world_y) const {
    // Simple implementation - could be improved
    wxSize size = GetSize();
    int screen_x = static_cast<int>((world_x - m_centerLon) * m_zoomLevel * 1000 + size.x / 2.0);
    int screen_y = static_cast<int>((m_centerLat - world_y) * m_zoomLevel * 1000 + size.y / 2.0);
    return {screen_x, screen_y};
}

bool MapPanel::CheckAndConvertPBFToSpatialite(const std::string& pbf_path) {
    namespace fs = std::filesystem;

    // Check if PBF file exists
    if (!fs::exists(pbf_path)) {
        wxLogError("PBF file not found: %s", pbf_path);
        return false;
    }

    // Generate Spatialite path (same directory, same name, .sqlite extension)
    fs::path pbf_file(pbf_path);
    fs::path sqlite_path = pbf_file.parent_path() / (pbf_file.stem().string() + ".sqlite");

    // Check if Spatialite file already exists
    if (fs::exists(sqlite_path)) {
        // Check if it's newer than PBF file
        auto pbf_time = fs::last_write_time(pbf_path);
        auto sqlite_time = fs::last_write_time(sqlite_path);

        if (sqlite_time >= pbf_time) {
            // Generate and load stylesheet
            std::string stylesheet = GenerateStylesheetForSpatialite(sqlite_path.string());
            if (!stylesheet.empty() && m_renderer) {
                m_osmLoaded = m_renderer->initialize(stylesheet);
            }
            return true;
        }
    }

    // Ask user if they want to convert
    wxString message = wxString::Format(
        "To improve map rendering performance, the PBF file needs to be converted to Spatialite format.\n\n"
        "Source: %s\n"
        "Output: %s\n\n"
        "This is a one-time process and may take several minutes depending on file size.\n\n"
        "Convert now?",
        pbf_path, sqlite_path.string());

    int response = wxMessageBox(message, "Map Data Conversion Required",
                                wxYES_NO | wxICON_QUESTION, this);

    if (response != wxYES) {
        return false;
    }

    // Create progress dialog
    wxProgressDialog progress("Converting Map Data",
                             "Converting PBF to Spatialite format...\nThis may take several minutes.",
                             100, this,
                             wxPD_APP_MODAL | wxPD_AUTO_HIDE | wxPD_CAN_ABORT);

    // Build ogr2ogr command with custom osmconf for POI and route support
    // Find custom osmconf.ini using DataDirectoryResolver
    wxString osmconfWx = DataDirectoryResolver::FindDataFile("osmconf.ini");
    std::string osmconf_path = osmconfWx.ToStdString();

    std::string cmd = "ogr2ogr -f SQLite \"" + sqlite_path.string() + "\" \"" + pbf_path + "\" ";
    cmd += "-dsco SPATIALITE=YES ";
    cmd += "--config OSM_USE_CUSTOM_INDEXING NO ";

    if (!osmconf_path.empty()) {
        cmd += "--config OSM_CONFIG_FILE \"" + osmconf_path + "\" ";
    }

    cmd += "-progress 2>&1";

    wxLogMessage("Running conversion: %s", cmd);
    progress.Pulse("Running ogr2ogr conversion...");

    // Run conversion
    FILE* pipe = popen(cmd.c_str(), "r");
    if (!pipe) {
        wxLogError("Failed to start ogr2ogr conversion");
        return false;
    }

    char buffer[256];
    bool user_cancelled = false;
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        // Update progress dialog
        if (!progress.Pulse(wxString(buffer))) {
            user_cancelled = true;
            break;
        }
        wxYield(); // Allow UI to update
    }

    int result = pclose(pipe);

    if (user_cancelled) {
        wxLogMessage("Conversion cancelled by user");
        // Clean up partial file
        if (fs::exists(sqlite_path)) {
            fs::remove(sqlite_path);
        }
        return false;
    }

    if (result != 0) {
        wxLogError("Conversion failed with error code: %d", result);
        return false;
    }

    // Verify the output file was created
    if (!fs::exists(sqlite_path)) {
        wxLogError("Conversion completed but output file not found");
        return false;
    }

    wxMessageBox("Map data conversion completed successfully!",
                 "Conversion Complete", wxOK | wxICON_INFORMATION, this);

    // Generate and load stylesheet
    std::string stylesheet = GenerateStylesheetForSpatialite(sqlite_path.string());
    if (!stylesheet.empty() && m_renderer) {
        m_osmLoaded = m_renderer->initialize(stylesheet);
    }

    return true;
}

std::string MapPanel::GenerateStylesheetForSpatialite(const std::string& sqlite_path) {
    namespace fs = std::filesystem;

    // Find the template stylesheet using DataDirectoryResolver
    wxString templateWx = DataDirectoryResolver::FindDataFile("map-style/osm-spatialite.xml");
    std::string template_path = templateWx.ToStdString();

    if (template_path.empty()) {
        wxLogError("Could not find osm-spatialite.xml template");
        return "";
    }

    // Read template
    std::ifstream template_file(template_path);
    if (!template_file) {
        wxLogError("Failed to open template: %s", template_path);
        return "";
    }

    std::stringstream buffer;
    buffer << template_file.rdbuf();
    std::string stylesheet_content = buffer.str();

    // Replace placeholder with actual Spatialite path
    size_t pos = 0;
    while ((pos = stylesheet_content.find("%SPATIALITE_FILE%", pos)) != std::string::npos) {
        stylesheet_content.replace(pos, 17, sqlite_path);
        pos += sqlite_path.length();
    }

    // Write to temporary file
    fs::path temp_stylesheet = fs::temp_directory_path() / "garmin_map_spatialite.xml";
    std::ofstream out_file(temp_stylesheet);
    if (!out_file) {
        wxLogError("Failed to create temporary stylesheet");
        return "";
    }

    out_file << stylesheet_content;
    out_file.close();

    return temp_stylesheet.string();
}