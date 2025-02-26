#include <iostream>
#include "binary-mapper.hpp"

using namespace darauble;

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <filename>" << std::endl;
        return 1;
    }

    try {
        BinaryMapper mapper(argv[1]);

        std::cout << "File size: " << mapper.size() << " bytes" << std::endl;

        mapper.parse();

        if (mapper.isParsed()) {
            std::cout << "File parsed successfully!" << std::endl;

            std::cout << "Header size: " << +mapper.header().headerSize << std::endl;
            std::cout << "Protocol version: " << +mapper.header().protocolVersion << std::endl;
            std::cout << "Profile version: " << mapper.header().profileVersion << std::endl;
            std::cout << "Data size: " << mapper.header().dataSize << " bytes" << std::endl;
        } else {
            std::cerr << "Error: Failed to parse file" << std::endl;
        }
        
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }

    return 0;
}
