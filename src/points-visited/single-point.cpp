#include <map>
#include <fstream>

#include "binary-mapper.hpp"
#include "coordinates-scanner.hpp"
#include "single-point.hpp"
#include "exceptions.hpp"

namespace darauble {

static std::map<std::string, FIT_SPORT> sport_map = {
    {"running", FIT_SPORT_RUNNING},
    {"cycling", FIT_SPORT_CYCLING},
    {"hiking", FIT_SPORT_HIKING},
    {"walking", FIT_SPORT_WALKING},
    {"swimming", FIT_SPORT_SWIMMING},
    {"fitness_equipment", FIT_SPORT_FITNESS_EQUIPMENT},
    {"golf", FIT_SPORT_GOLF},
    {"rowing", FIT_SPORT_ROWING},
    {"multisport", FIT_SPORT_MULTISPORT},
    {"paddling", FIT_SPORT_PADDLING},
    {"flying", FIT_SPORT_FLYING},
    {"e_biking", FIT_SPORT_E_BIKING},
    {"motorcycling", FIT_SPORT_MOTORCYCLING},
    {"boating", FIT_SPORT_BOATING},
    {"driving", FIT_SPORT_DRIVING},
    {"all", FIT_SPORT_ALL}
};

void SinglePointHandler::parse(const fs::path& filepath, std::vector<int32_t>& la, std::vector<int32_t>& lo) {
    BinaryMapper mapper {filepath};
    CoordinatesScanner scanner {mapper, sport, la, lo};
    scanner.scan();
}

uint32_t SinglePointHandler::search(std::vector<int32_t>& la, std::vector<int32_t>& lo) {
    uint32_t found {0};

    for (size_t i {0}; i < lo.size() - 1; i++) {
        if ((la.at(i) == FIT_SINT32_INVALID) || (lo.at(i) == FIT_SINT32_INVALID)
            || (la.at(i + 1) == FIT_SINT32_INVALID) || (lo.at(i + 1) == FIT_SINT32_INVALID)) {
            // std::cout << "One point is invalid, skipping." << std::endl;
            continue;
        }

        BoundingBox vector_box {la.at(i), lo.at(i), la.at(i + 1), lo.at(i + 1)};

        if (box.intersect(vector_box)) {
            // std::cout << "Boxes at " << i << vector_box << " intersect!" << std::endl;
            found++;
        }
    }

    return found;
}

SinglePointHandler::SinglePointHandler(SingleSummary& _summary, std::string _sport, BoundingBox _box) :
    summary {_summary}, sport{FIT_SPORT_ALL}, box {_box}
{
    if (sport_map.find(_sport) != sport_map.end()) {
        sport = sport_map[_sport];
    }
}

void SinglePointHandler::handle(const fs::path& filename) {
    summary.incrementTotalFiles();

    std::vector<int32_t> la, lo;

    try {
        auto start = std::chrono::high_resolution_clock::now();

        parse(filename, la, lo);

        auto end = std::chrono::high_resolution_clock::now();

        std::cout << "File " << filename << " parsed, read " << lo.size() << " points in " << (double) std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count() / 1000000000 << " s" << std::endl;

        if (la.size() != lo.size()) {
            std::cout << "Something's really very wrong!" << std::endl;
            return;
        }
        
        start = std::chrono::high_resolution_clock::now();
        uint32_t found = search(la, lo);

        end = std::chrono::high_resolution_clock::now();

        std::cout << "Found intersection(s): " << found << ". Searched for " << (double) std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count() / 1000000000 << " s" << std::endl;

        summary.incrementParsedFiles();
        summary.incrementFilteredFiles();

        if (found > 0) {
            summary.incrementTotalVisits();
        }

    } catch (const WrongSportException& e) {
        summary.incrementParsedFiles();
        std::cerr << e.what() << std::endl;
    } catch (...)
    {
        std::cerr << "Exception decoding file" << std::endl;
    }
}

} // namespace darauble