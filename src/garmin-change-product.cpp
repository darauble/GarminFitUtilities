#include <iostream>
#include "binary-mapper.hpp"
#include "product-scanner.hpp"

using namespace darauble;

int main(int argc, char* argv[]) {
    if ((argc != 5) && (argc != 2)) {
        std::cerr << "Usage to modify product: " << argv[0] << " <product number in the file> <product number to replace> input.fit output.fit" << std::endl;
        std::cerr << "     or to scan product: " << argv[0] << " input.fit" << std::endl;
        return 1;
    }

    try {
        if (argc == 2) {
            BinaryMapper mapper(argv[1]);
            ProductScanner scanner(mapper);
            scanner.scan();
            return 0;
        }
        
        BinaryMapper mapper(argv[3]);

        std::cout << "File size: " << mapper.size() << " bytes" << std::endl;

        mapper.parse();

        if (mapper.isParsed()) {
            uint16_t productNumber = std::stoi(argv[1]);
            uint16_t newProductNumber = std::stoi(argv[2]);

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
            mapper.save(argv[4]);
        } else {
            std::cerr << "Error: Failed to parse file" << std::endl;
        }
        
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }

    return 0;
}
