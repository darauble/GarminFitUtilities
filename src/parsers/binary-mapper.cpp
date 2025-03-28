#include "binary-mapper.hpp"

#include <iostream>
#include <iomanip>
#include <fstream>
#include <cstring>

#include <fit_crc.hpp>

namespace darauble {

BinaryMapper::BinaryMapper(const fs::path& filename, bool _showRaw) :
    binarySize {0}, parsed {false}, showRaw {_showRaw}
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

        if (showRaw) {
            std::cout << "Record Header: " << +recordHeader << ", " << (recordHeader & TYPE_MASK) << std::endl;
        }

        if ((recordHeader & TYPE_MASK) > 0) {
            // Definition message
            FitDefinitionMessage d;
            
            d.offset = recordOffset;
            offset++; // Skip the reserved byte
            d.architecture = read(offset);
            d.globalMessageNumber = readU16(offset, d.architecture);
            d.localMessageNumber = recordHeader & NORMAL_LOCAL_MASK;
            d.fieldCount = read(offset);
            d.messageSize = 0;

            if (showRaw) {
                std::cout << "======================================================" << std::endl;
                std::cout << "Definition Message: " << std::endl
                    << "  Global #" << d.globalMessageNumber << ", "
                    << "   Local #" << d.localMessageNumber << ", "
                    << "    Arch #" << +d.architecture << std::endl
                    ;

                std::cout << " +--------------- Fields " << std::setw(4) << +d.fieldCount << " ------------------+" << std::endl;
                std::cout << " | Number | Size | Arch | Base | Base+ | Offset |" << std::endl;
            }

            for (uint8_t i = 0; i < d.fieldCount; i++) {
                FitFieldDefinition f;

                f.fieldNumber = read(offset);
                f.size = read(offset);
                f.baseType = read(offset);
                f.endianAbility = f.baseType & FIELD_ENDIAN_MASK;
                f.developer = false;
                uint8_t short_base = f.baseType & FIELD_BASE_MASK;
                
                f.offset = 1;

                if (i > 0) {
                    f.offset = d.fields[i - 1].offset + d.fields[i - 1].size;
                }

                if (showRaw) {
                    std::cout << " | " << std::setw(6) << +f.fieldNumber 
                        << " | " << std::setw(4) << +f.size
                        << " | " << std::setw(4) << +f.endianAbility
                        << " | " << std::setw(4) << +short_base
                        << " | " << std::setw(5) << +f.baseType
                        <<" | " << std::setw(6) << +f.offset << " |" << std::endl;
                }

                d.messageSize += f.size;

                d.fields.push_back(f);
            }

            if ((recordHeader & DEV_DATA_MASK) > 0) {
                d.devFieldCount = read(offset);

                if (showRaw) {
                    std::cout << " |------------- Dev Fields " << std::setw(4) << +d.devFieldCount << " ----------------|" << std::endl;
                }

                for (uint8_t i = 0; i < d.devFieldCount; i++) {
                    FitFieldDefinition f;
    
                    f.fieldNumber = read(offset);
                    f.size = read(offset);
                    f.baseType = read(offset);
                    f.endianAbility = f.baseType & FIELD_ENDIAN_MASK;
                    f.developer = true;
                    uint8_t short_base = f.baseType & FIELD_BASE_MASK;
                    
                    f.offset = 1;
    
                    if (i > 0) {
                        f.offset += d.fields[i - 1].size;
                    }

                    if (showRaw) {
                        std::cout << " | " << std::setw(6) << +f.fieldNumber 
                            << " | " << std::setw(4) << +f.size
                            << " | " << std::setw(4) << +f.endianAbility
                            << " | " << std::setw(4) << +short_base
                            << " | " << std::setw(5) << +f.baseType
                            <<" | " << std::setw(6) << +f.offset << " |" << std::endl;
                    }

                    d.messageSize += f.size;
    
                    d.fields.push_back(f);
                }
            }
            if (showRaw) {
                std::cout << " +----------------------------------------------+" << std::endl;
                std::cout << " Total message size: " << +d.messageSize << " bytes" << std::endl << std::endl;
            }

            fitDefinitions.push_back(d);

        } else {
            // Data message
            if (showRaw) {
                std::cout << "======================================================" << std::endl;
                std::cout << "Data Message: local #";
            }

            FitDataMessage m;
            m.offset = recordOffset;
            
            if ((recordHeader & NORMAL_HEADER_MASK) == 0) {
                m.localMessageType = recordHeader & NORMAL_LOCAL_MASK;
                m.compressedTime = 0;
                
                if (showRaw) {
                    std::cout << +m.localMessageType << ", ";
                }
            } else {
                m.localMessageType = (recordHeader & TS_LOCAL_MASK) >> TS_LOCAL_SHIFT;
                m.compressedTime = recordHeader & TS_OFFSET_MASK;
                
                if (showRaw) {
                    std::cout << +m.localMessageType << " compressed" << ", ";
                }
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

            if (showRaw) {
                std::cout << "global #" << d.globalMessageNumber << std::endl;
                std::cout << "| ";
            
                for (auto i = 0; i < d.fields.size(); i++) {
                    for (auto j = 0; j < d.fields[i].size; j++) {
                        std::cout<< std::hex << std::setw(2) << std::setfill('0') <<  +binaryData[recordOffset + j + d.fields[i].offset] << " ";
                    }
                    std::cout << " | ";
                }

                std::cout << std::dec << std::setw(0) << std::setfill(' ') << std::endl << std::endl;
            }

            offset += d.messageSize;

            fitDataMessages.push_back(m);

            if (d.globalMessageNumber == FIT_MESG_NUM_FIELD_DESCRIPTION) {
                if (showRaw) {
                    std::cout << "Parsing developer field description:" << std::endl;
                }

                uint64_t dataOffset;
                fit::Profile::FIELD devFieldDesc;
                uint16_t nativeMesgNum {0};

                for (auto field : d.fields) {
                    dataOffset = m.offset + field.offset;

                    switch(field.fieldNumber) {
                        case 1:
                            // field_definition_number
                            devFieldDesc.num = read(dataOffset);
                            if (showRaw) std::cout << "  num: " << +devFieldDesc.num << std::endl;
                        break;

                        case 3:
                            // field_name
                            devFieldDesc.name = readString(dataOffset, field.size);
                            if (showRaw) std::cout << "  name: " << devFieldDesc.name << std::endl;
                        break;

                        case 6:
                            // scale
                            devFieldDesc.scale = read(dataOffset);

                            if (devFieldDesc.scale == FIT_UINT8_INVALID) {
                                devFieldDesc.scale = 1;
                            }

                            if (showRaw) std::cout << "  scale: " << +devFieldDesc.scale << std::endl;
                        break;
                        
                        case 7:
                            // offset
                            devFieldDesc.offset = readS(dataOffset);

                            if (devFieldDesc.offset == FIT_SINT8_INVALID) {
                                devFieldDesc.offset = 0;
                            }

                            if (showRaw) std::cout << "  offset: " << +devFieldDesc.offset << std::endl;
                        break;

                        case 14:
                            // native_mesg_num
                            nativeMesgNum = readU16(dataOffset, d.architecture);
                            if (showRaw) std::cout << "  native_mesg_num: " << nativeMesgNum << std::endl;
                        break;
                    }
                }

                if (!devFieldMeta.contains(nativeMesgNum)) {
                    devFieldMeta[nativeMesgNum] = std::unordered_map<uint16_t, fit::Profile::FIELD>();
                }

                if (!devFieldMeta[nativeMesgNum].contains(devFieldDesc.num)) {
                    devFieldMeta[nativeMesgNum][devFieldDesc.num] = devFieldDesc;
                } else {
                    std::cerr << "Error: Duplicate developer field description for message #" << nativeMesgNum << ", field #" << +devFieldDesc.num << std::endl;
                }
            }
        }
    }

    dataParsed = true;
}

void BinaryMapper::parse() {
    parseHeader();
    parseData();

    parsed = true;
}

const fit::Profile::FIELD *BinaryMapper::getField(const FitDefinitionMessage& d, FitFieldDefinition &f) {
    if (!f.developer) {
        return fit::Profile::GetField(d.globalMessageNumber, f.fieldNumber);
    } else {
        if (devFieldMeta.contains(d.globalMessageNumber)) {
            if (devFieldMeta[d.globalMessageNumber].contains(f.fieldNumber)) {
                return &devFieldMeta[d.globalMessageNumber][f.fieldNumber];
            }
        }
    }

    return nullptr;
}

int8_t BinaryMapper::readS(uint64_t &offset) {
    return (int8_t)binaryData[offset++];
}

uint8_t BinaryMapper::read(uint64_t &offset) {
    return binaryData[offset++];
}

int16_t BinaryMapper::readS16(uint64_t &offset, uint8_t architecture) {
    uint16_t value = readU16(offset, architecture);
    int16_t sValue;

    memcpy(&sValue, &value, sizeof(sValue));
    
    return sValue;
}

uint16_t BinaryMapper::readU16(uint64_t &offset, uint8_t architecture) {
    uint16_t value = 0;
    
    if (architecture == 0) {
        value = (binaryData[offset + 1] << 8) | binaryData[offset];
    } else {
        value = (binaryData[offset] << 8) | binaryData[offset + 1];
    }

    offset += 2;
    return value;
}

int32_t BinaryMapper::readS32(uint64_t &offset, uint8_t architecture) {
    uint32_t value = readU32(offset, architecture);
    int32_t sValue;

    memcpy(&sValue, &value, sizeof(sValue));
    
    return sValue;
}

uint32_t BinaryMapper::readU32(uint64_t &offset, uint8_t architecture) {
    uint32_t value = 0;
    
    if (architecture == 0) {
        value = (binaryData[offset + 3] << 24) | (binaryData[offset + 2] << 16) | (binaryData[offset + 1] << 8) | binaryData[offset];
    } else {
        value = (binaryData[offset] << 24) | (binaryData[offset + 1] << 16) | (binaryData[offset + 2] << 8) | binaryData[offset + 3];
    }

    offset += 2;
    return value;
}

int64_t BinaryMapper::readS64(uint64_t &offset, uint8_t architecture) {
    uint64_t value = readU64(offset, architecture);
    int64_t sValue;

    memcpy(&sValue, &value, sizeof(sValue));
    
    return sValue;
}

uint64_t BinaryMapper::readU64(uint64_t &offset, uint8_t architecture) {
    uint64_t value = 0;
    
    if (architecture == 0) {
        value = 
            ((uint64_t)binaryData[offset + 7] << 56) | ((uint64_t)binaryData[offset + 6] << 48) | ((uint64_t)binaryData[offset + 5] << 40) | ((uint64_t)binaryData[offset + 4] << 32)
            | (binaryData[offset + 3] << 24) | (binaryData[offset + 2] << 16) | (binaryData[offset + 1] << 8) | binaryData[offset];
    } else {
        value = 
        ((uint64_t)binaryData[offset] << 56) | ((uint64_t)binaryData[offset + 1] << 48) | ((uint64_t)binaryData[offset + 2] << 40) | ((uint64_t)binaryData[offset + 3] << 32)
        | (binaryData[offset + 4] << 24) | (binaryData[offset + 5] << 16) | (binaryData[offset + 6] << 8) | binaryData[offset + 7];
    }

    offset += 2;
    return value;
}

float BinaryMapper::readFloat(uint64_t &offset, uint8_t architecture) {
    uint32_t value = readU32(offset, architecture);
    float fValue;

    memcpy(&fValue, &value, sizeof(fValue));
    
    return fValue;
}

double BinaryMapper::readDouble(uint64_t &offset, uint8_t architecture) {
    uint64_t value = readU64(offset, architecture);
    double dValue;

    memcpy(&dValue, &value, sizeof(dValue));
    
    return dValue;
}

std::string BinaryMapper::readString(uint64_t &offset, uint8_t length) {
    uint64_t stringLength = 0;

    for (stringLength = 0; stringLength < length; stringLength++) {
        if (binaryData[offset + stringLength] == 0) {
            break;
        }
    }

    if (stringLength > length) {
        stringLength = length;
    }

    std::string extractedString(reinterpret_cast<const char*>(&binaryData[offset]), stringLength);
    offset += length;

    return extractedString;
}

std::string BinaryMapper::readDateTime(uint64_t &offset, uint8_t architecture) {
    int64_t garminTs = readU32(offset, architecture);
    std::time_t unixTs {garminTs + 631065600};
    std::tm stdTs = *std::localtime(&unixTs);

    std::ostringstream oss;

    oss << std::put_time(&stdTs, "%Y-%m-%d %H:%M:%S");

    return oss.str();
}

std::string BinaryMapper::readDuration(uint64_t &offset, uint8_t architecture, double scale) {
    uint32_t value = readU32(offset, architecture);
    double totalSeconds = static_cast<double>(value) / scale;
    
    int hours = static_cast<int>(totalSeconds) / 3600;
    int minutes = (static_cast<int>(totalSeconds) % 3600) / 60;
    double seconds = totalSeconds - (hours * 3600 + minutes * 60);

    std::ostringstream oss;
    oss << std::fixed << std::setprecision(2);

    if (hours > 0) {
        oss << hours << ":";
        oss << std::setw(2) << std::setfill('0') << minutes << ":";
    } else if (minutes > 0) {
        oss << minutes << ":";
    }

    oss << std::setw(5) << std::setfill('0') << std::fixed << std::setprecision(2) << seconds;

    return oss.str();
}

void BinaryMapper::write(uint64_t &offset, uint16_t value, uint8_t architecture) {
    if (architecture == 0) {
        binaryData[offset++] = value & 0xFF;
        binaryData[offset++] = (value >> 8) & 0xFF;
    } else {
        binaryData[offset++] = (value >> 8) & 0xFF;
        binaryData[offset++] = value & 0xFF;
    }
}

void BinaryMapper::write(uint64_t &offset, uint32_t value, uint8_t architecture) {
    if (architecture == 0) {
        binaryData[offset++] = value & 0xFF;
        binaryData[offset++] = (value >> 8) & 0xFF;
        binaryData[offset++] = (value >> 16) & 0xFF;
        binaryData[offset++] = (value >> 24) & 0xFF;
    } else {
        binaryData[offset++] = (value >> 24) & 0xFF;
        binaryData[offset++] = (value >> 16) & 0xFF;
        binaryData[offset++] = (value >> 8) & 0xFF;
        binaryData[offset++] = value & 0xFF;
    }
}

void BinaryMapper::write(uint64_t &offset, char* value, size_t length) {
    for (size_t i = 0; i < length; i++) {
        binaryData[offset++] = value[i];
    }
}

uint16_t BinaryMapper::CRC() {
    uint16_t crc {0};

    for (int i = 0; i < binarySize - 2; i++) {
        crc = fit::CRC::Get16(crc, binaryData[i]);
    }

    return crc;
}

void BinaryMapper::writeCRC() {
    uint16_t crc = CRC();
    uint64_t crcOffset = binarySize - 2;

    write(crcOffset, crc, 0);
}

void BinaryMapper::save(const fs::path& filename) {
    std::ofstream outFile(filename.string(), std::ios::binary);
    
    if (!outFile) {
        throw std::runtime_error("BinaryMapper error: cannot open file for writing");
    }

    outFile.write(reinterpret_cast<const char*>(binaryData.get()), binarySize);
    outFile.close();
}

} // namespace darauble
