#include <iostream>
#include <string>
#include "rename-files.hpp"

constexpr auto VERSION = "1.0.0";

using namespace darauble;

int main(int argc, char *argv[]) {
    std::cout << "Rename Garmin FIT files with a creation date utility version " << VERSION << std::endl;

    if (argc != 2) {
        std::cerr << "Provide a file name or a directory for the renaming operation" << std::endl;
        return -1;
    }

    try {
        FitFileHandler handler;
        DirectoryScanner scanner {handler, { ".fit" }};
        scanner.scan(argv[1]);
    } catch (const std::exception& e) {
        std::cerr << "Error scanning directory/reading the file: " << e.what() << std::endl;
        return -2;
    }
}
