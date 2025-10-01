#include "MapRenderer.hpp"
#include "MapPanel.hpp"
#include <wx/wx.h>
#include <wx/rawbmp.h>
#include <wx/dcmemory.h>
#include <cmath>
#include <cstdio>

#ifdef HAVE_MAPNIK
#include <mapnik/version.hpp>
#endif

MapnikRenderer::MapnikRenderer(int width, int height)
    : m_width(width), m_height(height)
#ifdef HAVE_MAPNIK
      , m_initialized(false)
      , m_mercator_minx(0), m_mercator_miny(0), m_mercator_maxx(0), m_mercator_maxy(0)
#endif
{
    ensureMapnikInitialized();
}

MapnikRenderer::~MapnikRenderer() {
}

void MapnikRenderer::ensureMapnikInitialized() {
#ifdef HAVE_MAPNIK
    static bool mapnik_registered = false;
    if (!mapnik_registered) {
        // Register datasource plugins
        mapnik::datasource_cache::instance().register_datasources("/usr/lib/mapnik/3.1/input/");

        // Register fonts
        mapnik::freetype_engine::register_fonts("/usr/share/fonts/", true);

        printf("Mapnik initialized - version %d.%d.%d\n",
               MAPNIK_MAJOR_VERSION, MAPNIK_MINOR_VERSION, MAPNIK_PATCH_VERSION);

        mapnik_registered = true;
    }
#endif
}

bool MapnikRenderer::initialize(const std::string& stylesheet_path, const std::string& fonts_dir) {
#ifdef HAVE_MAPNIK
    try {
        m_stylesheet_path = stylesheet_path;
        m_map = std::make_unique<mapnik::Map>(m_width, m_height);

        mapnik::load_map(*m_map, stylesheet_path);

        m_initialized = true;
        return true;
    } catch (const std::exception& e) {
        printf("ERROR loading Mapnik stylesheet: %s\n", e.what());
        fprintf(stderr, "Mapnik error details: %s\n", e.what());
        fflush(stdout);
        fflush(stderr);
        m_initialized = false;
        return false;
    }
#else
    printf("Mapnik support not compiled in\n");
    return false;
#endif
}

bool MapnikRenderer::isValid() const {
#ifdef HAVE_MAPNIK
    return m_initialized && m_map != nullptr;
#else
    return false;
#endif
}

void MapnikRenderer::setSize(int width, int height) {
    m_width = width;
    m_height = height;
#ifdef HAVE_MAPNIK
    if (m_map) {
        m_map->resize(width, height);
    }
#endif
}

void MapnikRenderer::setBounds(double min_lon, double min_lat, double max_lon, double max_lat) {
#ifdef HAVE_MAPNIK
    if (!m_map) return;

    try {
        // The map is in Web Mercator projection, so we need to convert lat/lon to Mercator
        // Web Mercator formulas
        double min_x = min_lon * 20037508.34 / 180.0;
        double max_x = max_lon * 20037508.34 / 180.0;

        double min_y = log(tan((90.0 + min_lat) * M_PI / 360.0)) / (M_PI / 180.0) * 20037508.34 / 180.0;
        double max_y = log(tan((90.0 + max_lat) * M_PI / 360.0)) / (M_PI / 180.0) * 20037508.34 / 180.0;

        mapnik::box2d<double> bbox(min_x, min_y, max_x, max_y);
        m_map->zoom_to_box(bbox);

        // Store the actual Mercator bounds for track overlay rendering
        m_mercator_minx = m_map->get_current_extent().minx();
        m_mercator_miny = m_map->get_current_extent().miny();
        m_mercator_maxx = m_map->get_current_extent().maxx();
        m_mercator_maxy = m_map->get_current_extent().maxy();
    } catch (const std::exception& e) {
        printf("Error setting bounds: %s\n", e.what());
    }
#endif
}

void MapnikRenderer::setCenter(double center_lon, double center_lat, double zoom_level) {
#ifdef HAVE_MAPNIK
    if (!m_map) return;

    try {
        // Calculate bounds based on zoom level
        // Zoom level is scale factor (pixels per meter in Web Mercator)
        // Higher zoom = smaller area

        // Convert zoom to approximate degrees
        double degrees_per_pixel = 360.0 / (256.0 * zoom_level);
        double half_width_degrees = (m_width / 2.0) * degrees_per_pixel;
        double half_height_degrees = (m_height / 2.0) * degrees_per_pixel;

        double min_lon = center_lon - half_width_degrees;
        double max_lon = center_lon + half_width_degrees;
        double min_lat = center_lat - half_height_degrees;
        double max_lat = center_lat + half_height_degrees;

        mapnik::box2d<double> bbox(min_lon, min_lat, max_lon, max_lat);
        m_map->zoom_to_box(bbox);
    } catch (const std::exception& e) {
        printf("Error setting center: %s\n", e.what());
    }
#endif
}

wxBitmap MapnikRenderer::render() {
#ifdef HAVE_MAPNIK
    if (!isValid()) {
        // Return empty bitmap
        wxBitmap bitmap(m_width, m_height);
        wxMemoryDC dc(bitmap);
        dc.SetBackground(*wxLIGHT_GREY_BRUSH);
        dc.Clear();
        dc.SetTextForeground(*wxBLACK);
        dc.DrawText("Mapnik not initialized", 10, 10);
        dc.SelectObject(wxNullBitmap);
        return bitmap;
    }

    try {
        // Render to Mapnik image
        mapnik::image_rgba8 image(m_width, m_height);
        mapnik::agg_renderer<mapnik::image_rgba8> renderer(*m_map, image);
        renderer.apply();

        // Convert Mapnik image to wxBitmap
        const unsigned char* mapnik_data = image.bytes();
        wxBitmap bitmap(m_width, m_height, 32);
        wxAlphaPixelData pixelData(bitmap);

        if (!pixelData) {
            wxBitmap error_bitmap(m_width, m_height);
            wxMemoryDC dc(error_bitmap);
            dc.SetBackground(*wxRED_BRUSH);
            dc.Clear();
            dc.SelectObject(wxNullBitmap);
            return error_bitmap;
        }

        wxAlphaPixelData::Iterator dst(pixelData);

        for (int y = 0; y < m_height; ++y) {
            dst.MoveTo(pixelData, 0, y);
            for (int x = 0; x < m_width; ++x) {
                const unsigned char* pixel = mapnik_data + (y * m_width + x) * 4;
                dst.Red() = pixel[0];
                dst.Green() = pixel[1];
                dst.Blue() = pixel[2];
                dst.Alpha() = pixel[3];
                ++dst;
            }
        }

        return bitmap;
    } catch (const std::exception& e) {
        printf("Error rendering map: %s\n", e.what());

        // Return error bitmap
        wxBitmap bitmap(m_width, m_height);
        wxMemoryDC dc(bitmap);
        dc.SetBackground(*wxLIGHT_GREY_BRUSH);
        dc.Clear();
        dc.SetTextForeground(*wxRED);
        dc.DrawText("Rendering error", 10, 10);
        dc.DrawText(wxString::FromUTF8(e.what()), 10, 30);
        dc.SelectObject(wxNullBitmap);
        return bitmap;
    }
#else
    wxBitmap bitmap(m_width, m_height);
    wxMemoryDC dc(bitmap);
    dc.SetBackground(*wxLIGHT_GREY_BRUSH);
    dc.Clear();
    dc.SetTextForeground(*wxBLACK);
    dc.DrawText("Mapnik support not compiled", 10, 10);
    dc.SelectObject(wxNullBitmap);
    return bitmap;
#endif
}

void MapnikRenderer::renderTrackOverlay(wxBitmap& bitmap, const std::vector<GPSPoint>& track,
                                        double min_lon, double min_lat, double max_lon, double max_lat) {
    if (track.empty()) return;

    wxMemoryDC dc(bitmap);

#ifdef HAVE_MAPNIK
    // Use the stored Mercator bounds from the map (set by setBounds)
    double min_x = m_mercator_minx;
    double max_x = m_mercator_maxx;
    double min_y = m_mercator_miny;
    double max_y = m_mercator_maxy;
#else
    // Fallback if no Mapnik: use simple lat/lon bounds
    double min_x = min_lon;
    double max_x = max_lon;
    double min_y = min_lat;
    double max_y = max_lat;
#endif

    // Calculate scale for Mercator coordinate conversion
    double x_range = max_x - min_x;
    double y_range = max_y - min_y;

    if (x_range <= 0 || y_range <= 0) return;

    double scale_x = m_width / x_range;
    double scale_y = m_height / y_range;

    // Draw track
    dc.SetPen(wxPen(*wxRED, 3));

    for (size_t i = 1; i < track.size(); ++i) {
        // Convert each GPS point to Web Mercator
        double x1_merc = track[i-1].longitude * 20037508.34 / 180.0;
        double y1_merc = log(tan((90.0 + track[i-1].latitude) * M_PI / 360.0)) / (M_PI / 180.0) * 20037508.34 / 180.0;

        double x2_merc = track[i].longitude * 20037508.34 / 180.0;
        double y2_merc = log(tan((90.0 + track[i].latitude) * M_PI / 360.0)) / (M_PI / 180.0) * 20037508.34 / 180.0;

        // Convert to screen coordinates
        int x1 = static_cast<int>((x1_merc - min_x) * scale_x);
        int y1 = static_cast<int>(m_height - ((y1_merc - min_y) * scale_y));

        int x2 = static_cast<int>((x2_merc - min_x) * scale_x);
        int y2 = static_cast<int>(m_height - ((y2_merc - min_y) * scale_y));

        dc.DrawLine(x1, y1, x2, y2);
    }

    // Draw start/end markers
    if (!track.empty()) {
        // Convert start point to Mercator
        double start_x_merc = track.front().longitude * 20037508.34 / 180.0;
        double start_y_merc = log(tan((90.0 + track.front().latitude) * M_PI / 360.0)) / (M_PI / 180.0) * 20037508.34 / 180.0;

        // Convert end point to Mercator
        double end_x_merc = track.back().longitude * 20037508.34 / 180.0;
        double end_y_merc = log(tan((90.0 + track.back().latitude) * M_PI / 360.0)) / (M_PI / 180.0) * 20037508.34 / 180.0;

        // Convert to screen coordinates
        int start_x = static_cast<int>((start_x_merc - min_x) * scale_x);
        int start_y = static_cast<int>(m_height - ((start_y_merc - min_y) * scale_y));

        int end_x = static_cast<int>((end_x_merc - min_x) * scale_x);
        int end_y = static_cast<int>(m_height - ((end_y_merc - min_y) * scale_y));

        dc.SetBrush(*wxGREEN_BRUSH);
        dc.DrawCircle(start_x, start_y, 6);

        dc.SetBrush(*wxRED_BRUSH);
        dc.DrawCircle(end_x, end_y, 6);
    }

    dc.SelectObject(wxNullBitmap);
}