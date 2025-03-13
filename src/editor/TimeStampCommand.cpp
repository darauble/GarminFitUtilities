#include "TimeStampCommand.hpp"

#include <chrono>
#include <cstring>
#include <iostream>

#include <fit_profile.hpp>

#include "binary-mapper.hpp"
#include "timestamp-scanner.hpp"

namespace darauble {

void TimeStampCommand::show(int argc, char* argv[]) {
    if (argc == 4 && strcmp(argv[3], "help") == 0) {
        help(argc, argv);
        return;
    }

    if (argc != 4) {
        std::cerr << "Wrong usage of the timestamp command, see help." << std::endl;
        return;
    }

    try {
        BinaryMapper mapper(argv[3]);
        TimestampScanner scanner(mapper);
        scanner.scan();

        for (const auto& ts : scanner.timestampIds()) {
            uint64_t offset = ts.offset;
            int64_t garminTs = mapper.readU32(offset, ts.definition.architecture);
            std::time_t unixTs {garminTs + 631065600};
            std::tm stdTs = *std::localtime(&unixTs);

            auto messageMeta = fit::Profile::GetMesg(ts.definition.globalMessageNumber);

            std::cout << "Message #" << std::setw(3) << ts.definition.globalMessageNumber << " "
                << std::setw(16) << (messageMeta ? messageMeta->name : "")
                << " " << std::setw(16) << ts.fieldName << ":"
                << " Garmin " << garminTs
                << ", Unix " << unixTs
                // << " timestamp " << mapper.readU32(offset, ts.definition.architecture)
                << ", local " << std::put_time(&stdTs, "%Y-%m-%d-%H:%M:%S")
                << " @" << ts.offset << std::endl;
        }
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
    }
}

void TimeStampCommand::set(int argc, char* argv[]) {
    if (argc == 4 && argv[3] == "help") {
        help(argc, argv);
        return;
    }

    if (argc != 6) {
        std::cerr << "Wrong usage of the timestamp command, see help." << std::endl;
        return;
    }


    try {
        BinaryMapper mapper(argv[4]);
        TimestampScanner scanner(mapper);
        scanner.scan();

        int64_t newTs = 0;
        std::time_t unixTs;

        {
            std::tm ts = {};
            std::istringstream ss(argv[3]);
            ss >> std::get_time(&ts, "%Y-%m-%d-%H:%M:%S");
            
            if (ss.fail()) {
                throw std::runtime_error("Failed to parse the timestamp");
            }

            unixTs = std::mktime(&ts);
            newTs = unixTs - 631065600;
        }

        int64_t oldTs = 0;
        int64_t diff = 0;

        for (const auto& ts : scanner.timestampIds()) {
            uint64_t offset = ts.offset;
            int64_t recordTs = mapper.readU32(offset, ts.definition.architecture);

            if (oldTs == 0 || recordTs < oldTs) {
                oldTs = recordTs;
            }
        }

        diff = newTs - oldTs;

        std::cout << "New time: " << argv[3] << ", Unix " << unixTs << ", Garmin " << newTs
            << ", difference from oldest " << diff << " s." << std::endl;

        if (diff == 0) {
            std::cerr << "The new timestamp is the same as the oldest one." << std::endl;
            return;
        }

        // BinaryMapper newMapper(argv[5], mapper);
        uint64_t counter = 0;

        for (const auto& ts : scanner.timestampIds()) {
            uint64_t offset = ts.offset;
            int64_t oldTs = mapper.readU32(offset, ts.definition.architecture);
            int64_t newTs = oldTs + diff;
            
            offset = ts.offset;
            
            mapper.write(offset, (uint32_t)newTs, ts.definition.architecture);
            counter++;
        }

        mapper.writeCRC();
        mapper.save(argv[5]);
        std::cout << "Updated " << counter << " timestamps." << std::endl;
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
    }
    
}

void TimeStampCommand::help(int argc, char* argv[]) {
    std::cout << "Usage:" << std::endl;
    std::cout << "  " << argv[0] << " show timestamp <file name>" << std::endl;
    std::cout << "      show all the messages that have timestamps in them and their offsets" << std::endl;
    std::cout << "  " << argv[0] << " set timestamp <timestamp> <file name> <new file name>" << std::endl;
    std::cout << "      Sets the given time stamp on *earliest* of the file's timestamp." << std::endl;
    std::cout << "      Updates all the other time stamps by difference between oldest original and new timestamp." << std::endl;
    std::cout << "      Writes the changes to a new file (or overwrites the old one if the same path is given)." << std::endl;
    std::cout << "      Time stamp format is YYYY-MM-DD-HH:mm:ss. 24 hours and note the dash between date and time." << std::endl;
}

const std::string TimeStampCommand::description() {
    return "show or modify the starting timestamp of the workout";
}


} // namespace darauble
