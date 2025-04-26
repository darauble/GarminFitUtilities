#include "GpxCommand.hpp"

#include <cstring>
#include <iostream>

#include <fit_profile.hpp>
#include <fit_record_mesg.hpp>

#include "binary-mapper.hpp"
#include "convert.hpp"
#include "coordinate-replacement-scanner.hpp"
#include "formulae.hpp"
#include "gpx-trkpt.hpp"

namespace darauble {

void GpxCommand::show(int argc, char* argv[]) {
    if (argc == 4 && strcmp(argv[3], "help") == 0) {
        help(argc, argv);
        return;
    }

    if (argc != 5) {
        std::cerr << "Invalid number of arguments." << std::endl << std::endl;
        help(argc, argv);
        return;
    }

    if (strcmp(argv[3], "distance") == 0) {
        auto trackpoints = parsers::ReadTrackpoints(argv[4], true);

        std::cout << "Total points: " << trackpoints.size() << std::endl;
        std::cout << "Total distance: " << trackpoints.at(trackpoints.size() - 1).cumulative_distance << " m" << std::endl;
    } else {
        std::cerr << "Invalid subcommand.[" << argv[3] << "]" << std::endl;
        help(argc, argv);
        return;
    }
}

void GpxCommand::replace(int argc, char* argv[]) {
    if (argc == 4 && strcmp(argv[3], "help") == 0) {
        help(argc, argv);
        return;
    }

    if (argc != 6 && argc != 7) {
        std::cerr << "Invalid number of arguments." << std::endl << std::endl;
        help(argc, argv);
        return;
    }

    bool sessionOnly = false;
    bool simple = true;
    size_t at = 3;

    if (argc == 7) {
        if (strcmp(argv[3], "session") == 0) {
            sessionOnly = true;
        } else if (strcmp(argv[3], "advanced") == 0) {
            simple = false;
        } else {
            std::cerr << "Invalid subcommand [" << argv[3] << "]" << std::endl;
            help(argc, argv);
            return;
        }
        at = 4;
    }


    try {
        BinaryMapper mapper(argv[at]);
        CoordinateReplacementScanner scanner(mapper);
        scanner.scan();

        auto offsets = scanner.getOffsets();
        auto trackpoints = parsers::ReadTrackpoints(argv[at + 1], true);

        if (offsets.size() < 2 || trackpoints.size() < 2) {
            std::cerr << "Not enough records or trackpoints. Should be at least two of each." << std::endl;
            return;
        }

        double fieldScale = 1.0;
        double fieldOffset = 0.0;

        const auto &desc = fit::Profile::GetField(FIT_MESG_NUM_RECORD, fit::RecordMesg::FieldDefNum::Distance);

        if (desc) {
            fieldScale = desc->scale;
            fieldOffset = desc->offset;
        }

        size_t i = 0;
        size_t record_i = 0;
        double bearing = 0.0, rlat, rlon;

        if (!sessionOnly) {
            double lastRecordDistance = 0.0;

            for (const auto& offset : offsets) {
                record_i++;
                uint64_t distanceOffset = offset.distance;
                uint32_t distanceRaw = mapper.readU32(distanceOffset, offset.architecture);
                double distance = static_cast<double>(distanceRaw) / fieldScale - fieldOffset;
                
                int32_t newLat, newLon;

                while (distance > trackpoints.at(i).cumulative_distance && i < trackpoints.size() - 1) {
                    i++;
                    rlat = radiansFromDegrees(trackpoints.at(i-1).lat);
                    rlon = radiansFromDegrees(trackpoints.at(i-1).lon);

                    if (!simple) {
                        bearing = coordinates::bearing_radians(rlat, rlon,
                            radiansFromDegrees(trackpoints.at(i).lat),
                            radiansFromDegrees(trackpoints.at(i).lon));
                        lastRecordDistance = distance;
                    }
                }

                if (simple) {
                    newLat = fromDouble(trackpoints.at(i).lat);
                    newLon = fromDouble(trackpoints.at(i).lon);
                } else {
                    if (distance < trackpoints.at(i).cumulative_distance) {
                        double newLatRad, newLonRad;

                        coordinates::next_point_radians(rlat, rlon, bearing, distance - lastRecordDistance, newLatRad, newLonRad);
                        
                        newLat = int32FromRadians(newLatRad);
                        newLon = int32FromRadians(newLonRad);
                    } else {
                        newLat = fromDouble(trackpoints.at(i).lat);
                        newLon = fromDouble(trackpoints.at(i).lon);
                    }
                }
                    
                uint64_t latOffset = offset.lat;
                uint64_t lonOffset = offset.lon;

                mapper.write(latOffset, newLat, offset.architecture);            
                mapper.write(lonOffset, newLon, offset.architecture);            
            }
        }

        // Replace session start and end lat/lon
        auto sessionOffset = scanner.getSessionOffset();
        uint64_t writeOffset = sessionOffset.startLat;
        mapper.write(writeOffset, fromDouble(trackpoints.at(0).lat), sessionOffset.architecture);

        writeOffset = sessionOffset.startLon;
        mapper.write(writeOffset, fromDouble(trackpoints.at(0).lon), sessionOffset.architecture);

        writeOffset = sessionOffset.endLat;
        mapper.write(writeOffset, fromDouble(trackpoints.at(trackpoints.size() - 1).lat), sessionOffset.architecture);

        writeOffset = sessionOffset.endLon;
        mapper.write(writeOffset, fromDouble(trackpoints.at(trackpoints.size() - 1).lon), sessionOffset.architecture);

        mapper.writeCRC();
        mapper.save(argv[at + 2]);

        std::cout << "Replaced coordinates in " << offsets.size() << " records from " << trackpoints.size() << " GPX trackpoints." << std::endl;
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
    }
}

void GpxCommand::help(int argc, char* argv[]) {
    std::cout << "Usage: " << argv[0] << " show gpx <distance|help> [<filename>]" << std::endl;
    std::cout << "  Show GPX trackpoints with distance calculation." << std::endl;
    std::cout << "Usage: " << argv[0] << " replace gpx [session|advanced] <FIT file> <GPX file> <new FIT file>" << std::endl;
    std::cout << "  Replace coordinates in the FIT file with those from the GPX file." << std::endl;
    std::cout << "  By default GPX points are written by distance approximately, filling all the records." << std::endl;
    std::cout << "  Option \"advanced\" extrapolates additional points if GPX file has less points than the FIT." << std::endl;
    std::cout << "  Option \"session\" updates only session record's start and end points (sometimes useful for different generic location)." << std::endl;
}

const std::string GpxCommand::description() {
    return "display GPX trackpoints with distance calculation";
}

} // namespace darauble