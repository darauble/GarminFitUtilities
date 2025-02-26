#include <iostream>
#include "rename-files.hpp"

using namespace darauble;

int main(int argc, char *argv[]) {
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
