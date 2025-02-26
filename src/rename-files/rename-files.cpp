#include <iostream>
#include <fstream>
#include <iomanip>
#include <chrono>

#include <fit_date_time.hpp>
#include <fit_decode.hpp>
#include <fit_mesg_broadcaster.hpp>

#include "exceptions.hpp"
#include "rename-files.hpp"

namespace darauble {


void Listener::OnMesg(fit::FileIdMesg& mesg) {
        fit::DateTime created {mesg.GetTimeCreated()};
        fileCreated = created.GetTimeT();
        
        if (mesg.GetType() != FIT_FILE_ACTIVITY) {
            throw NotActivityException("File is not of activity type");
        }

        throw StopParsingException("Date found!");
    }

FitFileHandler::FitFileHandler() {
    broadcaster.AddListener((fit::FileIdMesgListener&)listener);
}

void FitFileHandler::handle(const fs::path& filename) {
    std::cout << "Fit file: " << filename.string() << std::endl;

    std::ifstream file(filename.string(), std::ios::in | std::ios::binary);

    if (!file.is_open()) {
        std::cerr << "  Failed to open FIT file." << std::endl;
        return;
    }

    bool parsed {false};

    if (!decode.CheckIntegrity(file))
    {
        std::cerr << "  FIT file integrity failed. Attempting to decode..." << std::endl;
    }

    try {
        decode.Read(file, broadcaster);
    } catch (const StopParsingException& e) {
        parsed = true;
    } catch (const NotActivityException& e) {
        std::cerr << "  File is not an activity, skipping." << std::endl;
        goto except;
    } catch (const fit::RuntimeException& e) {
        std::cerr << "  Exception decoding file: " << e.what() << std::endl;
        goto except;
    } catch (const std::exception& e) {
        std::cerr << "  Exception decoding file: " << e.what() << std::endl;
        goto except;
    }

except:
    if (parsed) {
        std::tm ts = *std::localtime(&fileCreated);

        std::ostringstream oss;
        oss << std::put_time(&ts, "%Y-%m-%d-%H-%M-%S");

        std::cout << "  File created at: " << fileCreated << ", " << oss.str() << std::endl;
        fs::path newName = filename.parent_path() / oss.str();
        newName += ".fit";
        
        if (!fs::exists(newName)) {
            std::cout <<  "  Renaming to: " << newName.string() << std::endl;
            fs:rename(filename, newName);
        }

    }

    file.close();
}

} // namespace darauble