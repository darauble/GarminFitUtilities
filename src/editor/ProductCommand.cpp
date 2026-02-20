#include "ProductCommand.hpp"

#include <cstring>
#include <iostream>

#include "binary-mapper.hpp"
#include "product-scanner.hpp"

namespace darauble {

void ProductCommand::show(int argc, char* argv[]) {
    if (argc == 4 && strcmp(argv[3], "help") == 0) {
        help(argc, argv);
        return;
    }

    if (argc != 4) {
        std::cerr << "Wrong usage of the product command, see help." << std::endl;
        return;
    }

    try {
        BinaryMapper mapper(argv[3]);
        ProductScanner scanner(mapper);
        scanner.scan();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
    }
}

void ProductCommand::replace(int argc, char* argv[]) {
    if (argc == 4 && strcmp(argv[3], "help") == 0) {
        help(argc, argv);
        return;
    }

    if (argc != 7) {
        std::cerr << "Wrong usage of the product command, see help." << std::endl;
        return;
    }

    try {
        BinaryMapper mapper(argv[5]);

        std::cout << "File size: " << mapper.size() << " bytes" << std::endl;

        mapper.parse();

        if (mapper.isParsed()) {
            uint16_t productNumber = std::stoi(argv[3]);
            uint16_t newProductNumber = std::stoi(argv[4]);

            ProductScanner scanner(mapper, productNumber);
            scanner.scan();

            std::cout << "File parsed successfully!" << std::endl;

            std::cout << "Header size: " << +mapper.header().headerSize << std::endl;
            std::cout << "Protocol version: " << +mapper.header().protocolVersion << std::endl;
            std::cout << "Profile version: " << mapper.header().profileVersion << std::endl;
            std::cout << "Data size: " << mapper.header().dataSize << " bytes" << std::endl;
            std::cout << "CRC:       " << mapper.CRC() << std::endl;

            std::cout << std::endl << "Modifying the file..." << std::endl;

            for (auto& productId : scanner.productIds()) {
                std::cout << "Modify Product ID at offset " << productId.offset << std::endl;
                uint64_t modificationOffset = productId.offset;
                mapper.write(modificationOffset, newProductNumber, productId.architecture);
            }

            mapper.writeCRC();
            mapper.save(argv[6]);
        } else {
            std::cerr << "Error: Failed to parse file" << std::endl;
        }
        
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
    }
}

void ProductCommand::help(int argc, char* argv[]) {
    std::cout << "Usage:" << std::endl;
    std::cout << "  " << argv[0] << " show product <file name>" << std::endl;
    std::cout << "      show all the messages that have product IDs in them and their offsets" << std::endl;
    std::cout << "  " << argv[0] << " replace product <product number to replace> <new product number> <file name> <new file name>" << std::endl;
    std::cout << "      Replaces the given product number with a new one in the file." << std::endl;
}

const std::string ProductCommand::description() {
    return "show or replace product IDs in the file";
}

} // namespace darauble
