#include <cmath>
#include <iostream>
#include <iomanip>

#include "bounding-box.hpp"
#include "command-args.hpp"
#include "convert.hpp"
#include "single-point.hpp"
#include "single-summary.hpp"

constexpr auto VERSION = "1.0.0";

using namespace darauble;

int main(int argc, char *argv[]) {
    std::cout << "Points visited in Garmin FIT files version " << VERSION << std::endl;

    CommandArgsParser cargs;

    std::cout << std::setprecision(18) << std::defaultfloat;

    cargs.define('h', "help", "Show help");
    cargs.define('l', "latitude", "Set the latitude of the point to search, e.g. 54.92848", std::nan(""));
    cargs.define('o', "longitude", "Set the longitude of the point to search, e.g. 23.75036", std::nan(""));
    cargs.define('i', "input", "Set the path to a directory to search or a signle file to parse", "");
    cargs.define('d', "distance", "Distance in meters to the searching square side, default 15", 15);
    cargs.define('s', "sport", "Read only files with the given sport: running, cycling, hiking, walking, fitness_equipment etc. Default \"all\"", "all");

    cargs.parse(argc, argv);

    if (cargs["help"].b()) {
        cargs.showHelp();
        return 0;
    }

    if (std::isnan(cargs["latitude"].d()) || std::isnan(cargs["longitude"].d())) {
        std::cerr << "Please specify latitude and longitude" << std::endl;
        cargs.showHelp();
        return 0;
    }

    if (cargs["input"].s().size() == 0) {
        std::cerr << "Please specify input directory or file" << std::endl;
        cargs.showHelp();
        return 0;
    }

    std::cout << "Parsed latittude: " << cargs["latitude"].d() << std::endl;

    double lat = cargs["latitude"].d();      // Latitude in degrees,
    double lon = cargs["longitude"].d();     // Longitude in degrees
    double distance_m = cargs["distance"].i();

    double top_lat, left_lon, bottom_lat, right_lon;
    calculate_square(lat, lon, distance_m, top_lat, left_lon, bottom_lat, right_lon);

    BoundingBox search_box {top_lat, left_lon, bottom_lat, right_lon};

    std::cout << "Searching for bounding box "
        << "{ " << top_lat << ", " << left_lon << ", " << bottom_lat << ", " << right_lon << " }, "
        << search_box << "..." << std::endl;
    
    try {
        SingleSummary summary;
        SinglePointHandler handler(summary, cargs["sport"], search_box);
        DirectoryScanner scanner {handler, { ".fit" }};

        scanner.scan(cargs["input"].s());
        std::cout << summary;
    } catch (const std::exception& e) {
        std::cerr << "Error scanning directory/reading the file: " << e.what() << std::endl;
        return -2;
    }
}
