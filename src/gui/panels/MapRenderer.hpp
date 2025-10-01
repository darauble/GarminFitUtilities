#pragma once

#include <vector>
#include <string>
#include <memory>
#include "fit.hpp"

#ifdef HAVE_MAPNIK
#include <mapnik/map.hpp>
#include <mapnik/layer.hpp>
#include <mapnik/datasource_cache.hpp>
#include <mapnik/font_engine_freetype.hpp>
#include <mapnik/agg_renderer.hpp>
#include <mapnik/load_map.hpp>
#include <mapnik/image.hpp>
#include <mapnik/image_util.hpp>
#include <mapnik/projection.hpp>
#include <mapnik/box2d.hpp>
#endif

#include <wx/bitmap.h>

// Forward declarations
struct GPSPoint;

class MapnikRenderer {
private:
#ifdef HAVE_MAPNIK
    std::unique_ptr<mapnik::Map> m_map;
    std::string m_stylesheet_path;
    bool m_initialized;
    double m_mercator_minx, m_mercator_miny, m_mercator_maxx, m_mercator_maxy;
#endif
    int m_width, m_height;

public:
    MapnikRenderer(int width, int height);
    ~MapnikRenderer();

    // Setup
    bool initialize(const std::string& stylesheet_path, const std::string& fonts_dir = "");
    bool isValid() const;
    void setSize(int width, int height);

    // View control
    void setBounds(double min_lon, double min_lat, double max_lon, double max_lat);
    void setCenter(double center_lon, double center_lat, double zoom_level);

    // Rendering
    wxBitmap render();
    void renderTrackOverlay(wxBitmap& bitmap, const std::vector<GPSPoint>& track,
                           double min_lon, double min_lat, double max_lon, double max_lat);

private:
    void ensureMapnikInitialized();
};