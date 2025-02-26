#include "binary-mapper.hpp"

#include <iostream>
#include <iomanip>
#include <fstream>

namespace darauble {

BinaryMapper::BinaryMapper(const fs::path& filename) :
    binarySize {0}, parsed {false}
{
    std::ifstream file(filename, std::ios::binary | std::ios::ate);
    if (!file) {
        throw std::runtime_error("Error: Cannot open file " + filename.string());
    }

    binarySize = file.tellg();  // Get file size
    file.seekg(0, std::ios::beg); // Move back to beginning

    // Allocate memory using shared_ptr with a custom deleter
    binaryData.reset(new uint8_t[binarySize], std::default_delete<uint8_t[]>());

    // Read file contents into the allocated buffer
    file.read(reinterpret_cast<char*>(binaryData.get()), binarySize);
    if (!file) {
        throw std::runtime_error("Error: Failed to read file " + filename.string());
    }

    file.close();
}

std::shared_ptr<uint8_t[]> BinaryMapper::data()  {
    return binaryData;
}

size_t BinaryMapper::size() {
    return binarySize;
}

bool BinaryMapper::isParsed() {
    return parsed;
}

void BinaryMapper::parseHeader() {
    if (binaryData[0] != 12 && binaryData[0] != 14) {
        throw std::runtime_error("Error: Invalid header size");
    }

    if (std::string(reinterpret_cast<char*>(&binaryData[8]), 4) != ".FIT") {
        throw std::runtime_error("Error: not a FIT file");
    }

    fitHeader.headerSize = binaryData[0];
    fitHeader.protocolVersion = binaryData[1];
    fitHeader.profileVersion = (binaryData[3] << 8) | binaryData[2];
    fitHeader.dataSize = (binaryData[7] << 24) | (binaryData[6] << 16) | (binaryData[5] << 8) | binaryData[4];

    headerParsed = true;
}

void BinaryMapper::parseData() {
    if (!headerParsed) {
        throw std::runtime_error("Error: Header must be parsed first");
    }

    uint64_t offset = fitHeader.headerSize;
    
    while (offset < fitHeader.headerSize + fitHeader.dataSize) {
        uint64_t recordOffset = offset;
        uint8_t recordHeader = read(offset);

        std::cout << "Record Header: " << +recordHeader << ", " << (recordHeader & TYPE_MASK) << std::endl;

        if ((recordHeader & TYPE_MASK) > 0) {
            // Definition message
            FitDefinitionMessage d;
            
            d.offset = recordOffset;
            offset++; // Skip the reserved byte
            d.architecture = read(offset);
            d.globalMessageNumber = read(offset, d.architecture);
            d.localMessageNumber = recordHeader & NORMAL_LOCAL_MASK;
            d.fieldCount = read(offset);
            d.messageSize = 0;

            std::cout << "======================================================" << std::endl;
            std::cout << "Definition Message: " << std::endl
                << "  Global #" << d.globalMessageNumber << ", "
                << "   Local #" << d.localMessageNumber << ", "
                << "    Arch #" << +d.architecture << std::endl
                ;

            std::cout << " +----------- Fields " << std::setw(4) << +d.fieldCount << " --------------+" << d.fieldCount << std::endl;
            std::cout << " | Number | Size | Arch | Base | Offset |" << std::endl;

            for (uint8_t i = 0; i < d.fieldCount; i++) {
                FitFieldDefinition f;

                f.fieldNumber = read(offset);
                f.size = read(offset);
                f.baseType = read(offset);

                f.endianAbility = (f.baseType & FIELD_ENDIAN_MASK) > 1;
                f.baseType &= FIELD_BASE_MASK;
                
                f.offset = 1;

                if (i > 0) {
                    f.offset = d.fields[i - 1].offset + d.fields[i - 1].size;
                }

                std::cout << " | " << std::setw(6) << +f.fieldNumber 
                    << " | " << std::setw(4) << +f.size
                    << " | " << std::setw(4) << +f.endianAbility
                    << " | " << std::setw(4) << +f.baseType
                    <<" | " << std::setw(6) << +f.offset << " |" << std::endl;

                d.messageSize += f.size;

                d.fields.push_back(f);
            }

            if ((recordHeader & DEV_DATA_MASK) > 0) {
                d.devFieldCount = read(offset);

                std::cout << " |--------- Dev Fields " << std::setw(4) << d.fieldCount << " --------------|" << d.devFieldCount << std::endl;

                for (uint8_t i = 0; i < d.devFieldCount; i++) {
                    FitFieldDefinition f;
    
                    f.fieldNumber = read(offset);
                    f.size = read(offset);
                    f.baseType = read(offset);
    
                    f.endianAbility = f.baseType & FIELD_ENDIAN_MASK;
                    f.baseType &= FIELD_BASE_MASK;
                    
                    f.offset = 1;
    
                    if (i > 0) {
                        f.offset += d.fields[i - 1].size;
                    }

                    d.messageSize += f.size;
    
                    d.fields.push_back(f);
                }
            }
            std::cout << " +--------------------------------------+" << std::endl;
            std::cout << " Total message size: " << +d.messageSize << " bytes" << std::endl << std::endl;

            fitDefinitions.push_back(d);

        } else {
            // Data message
            std::cout << "======================================================" << std::endl;
            std::cout << "Data Message: local #";

            FitDataMessage m;
            m.offset = recordOffset;
            
            if ((recordHeader & NORMAL_HEADER_MASK) == 0) {
                m.localMessageType = recordHeader & NORMAL_LOCAL_MASK;
                m.compressedTime = 0;
                std::cout << +m.localMessageType << ", ";
            } else {
                m.localMessageType = (recordHeader & TS_LOCAL_MASK) >> TS_LOCAL_SHIFT;
                m.compressedTime = recordHeader & TS_OFFSET_MASK;
                std::cout << +m.localMessageType << " compressed" << ", ";
            }


            bool found = false;
            
            for (auto i = fitDefinitions.size() - 1; i >= 0; i--) {
                if (fitDefinitions[i].localMessageNumber == m.localMessageType) {
                    m.definitionIndex = i;
                    found = true;
                    break;
                }
            }

            if (!found) {
                std::cout << std::endl;
                std::cerr << "Error: Data message encountered without a definition!\n";
                return;
            }

            FitDefinitionMessage & d = fitDefinitions[m.definitionIndex];

            std::cout << "global #" << d.globalMessageNumber << std::endl;

            std::cout << "| ";

            for (auto i = 0; i < d.fields.size(); i++) {
                for (auto j = 0; j < d.fields[i].size; j++) {
                    std::cout<< std::hex << std::setw(2) << std::setfill('0') <<  +binaryData[recordOffset + j + d.fields[i].offset] << " ";
                }
                std::cout << " | ";
            }

            std::cout << std::dec << std::setw(0) << std::setfill(' ') << std::endl << std::endl;

            offset += d.messageSize;
            std::cout<< std::hex << std::setw(2) << std::setfill('0') <<  +binaryData[offset] << " " << std::endl;
            
            std::cout << std::dec << std::setw(0) << std::setfill(' ');

            // break;
        }

        
    }

    dataParsed = true;
}

void BinaryMapper::parse() {
    parseHeader();
    parseData();

    parsed = true;
}

uint8_t BinaryMapper::read(uint64_t &offset) {
    return binaryData[offset++];
}

uint16_t BinaryMapper::read(uint64_t &offset, uint8_t architecture) {
    uint16_t value = 0;
    
    if (architecture == 0) {
        value = (binaryData[offset + 1] << 8) | binaryData[offset];
    } else {
        value = (binaryData[offset] << 8) | binaryData[offset + 1];
    }

    offset += 2;
    return value;
}

} // namespace darauble
