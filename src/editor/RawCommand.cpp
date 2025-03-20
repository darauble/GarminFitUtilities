#include "RawCommand.hpp"

#include "binary-mapper.hpp"

#include <cstring>

/*
TODO:
  * Sutvarkyti visus tipus, kad juos būtų galima įrašyti.
*/

namespace darauble {

void RawCommand::set(int argc, char* argv[]) {
    if (argc == 4 && (strcmp(argv[3], "help") == 0)) {
        help(argc, argv);
        return;
    }

    if (argc != 8) {
        std::cerr << "Wrong usage of the raw command (" << argc << "), see help." << std::endl;
        return;
    }

    try {
        BinaryMapper mapper(argv[6]);
        mapper.parse();

        uint64_t offset = std::stoll(argv[4]);
        uint8_t architecture {0};

        for (size_t i = 0; i < mapper.dataMessages().size() - 1; i++) {
            if ((mapper.dataMessages().at(i).offset <= offset) && (mapper.dataMessages().at(i + 1).offset >= offset)) {
                architecture = mapper.definitions().at(mapper.dataMessages().at(i).definitionIndex).architecture;
                break;
            }
        }

        if (strcmp(argv[3], "u16") == 0) {
            uint16_t value = static_cast<uint16_t>(std::stoll(argv[5]));
            mapper.write(offset, value, architecture);
        } else if (strcmp(argv[3], "u32") == 0) {
            uint32_t value = static_cast<uint32_t>(std::stoll(argv[5]));
            mapper.write(offset, value, architecture);
        } else if (strcmp(argv[3], "s") == 0) {
            mapper.write(offset, argv[5], strlen(argv[5]));
        } else {
            std::cerr << "The type " << argv[3] << " is not supported yet!" << std::endl;
            return;
        }

        mapper.writeCRC();
        mapper.save(argv[7]);
        std::cout << "Updated file with value " << argv[5] << "." << std::endl;
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
    }
}

void RawCommand::help(int argc, char* argv[]) {
    std::cout << "Usage:" << std::endl;
    std::cout << "  " << argv[0] << " set raw u8|s8|u16|s16|u32|s32|s <offset> <value> <file name> <new file name>" << std::endl;
    std::cout << "      set a raw value at offset in the file." << std::endl;
    std::cout << "  " << argv[0] << " Types:" << std::endl;
    std::cout << "      u8 - unsigned byte, s8 - signed byte, u16 - unsigned 2 byte, s16 - signed 2 byte," << std::endl;
    std::cout << "      u32 - unsigned 4 byte, s32 - signed 4 byte, s - string." << std::endl;
}

const std::string RawCommand::description() {
    return "set a raw value at a particular offset in the FIT file";
}
    

} // namespace darauble
