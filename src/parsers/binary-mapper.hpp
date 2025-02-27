#pragma once
#include <cstdint>
#include <vector>
#include <string>
#include <filesystem>

namespace fs = std::filesystem;
namespace darauble {

struct FitFileHeader {
    uint8_t headerSize;      // Typically 12 or 14
    uint8_t protocolVersion;
    uint16_t profileVersion;
    uint32_t dataSize;       // Number of bytes in the data section
    char fileType[4];        // Should be ".FIT"
};

struct FitFieldDefinition {
    uint8_t fieldNumber;
    uint8_t size;    // Number of bytes
    uint8_t endianAbility;
    uint8_t baseType;
    uint16_t offset; // Offset from the record start (first starts at 1, 0 is the reacord header byte)
};

struct FitDefinitionMessage {
    uint64_t offset;
    // uint8_t reserved;          // Must be 0
    uint8_t architecture;      // 0 = little-endian, 1 = big-endian
    uint16_t globalMessageNumber;
    uint16_t localMessageNumber;
    
    uint8_t fieldCount;
    uint8_t devFieldCount;
    
    uint32_t messageSize;
   
    std::vector<FitFieldDefinition> fields;
};

// struct ParsedDefinition {
//     uint16_t globalMessageNumber;
//     std::vector<FitFieldDefinition> fields;
//     uint8_t totalSize;  // Total size of a single Data Message
//     uint8_t developerFieldCount; // Number of developer fields
// };

struct FitDataMessage {
    uint64_t offset; // Starts at header
    uint64_t definitionIndex;
    uint8_t localMessageType;
    uint8_t compressedTime;
};

class BinaryMapper {
private:
    static const uint8_t NORMAL_HEADER_MASK = 0x80;
    static const uint8_t TYPE_MASK = 0x40;
    static const uint8_t DEV_DATA_MASK = 0x20;

    static const uint8_t NORMAL_LOCAL_MASK = 0x0F;
    static const uint8_t TS_LOCAL_MASK = 0x60;
    static const uint8_t TS_LOCAL_SHIFT = 5;
    static const uint8_t TS_OFFSET_MASK = 0x1F;

    static const uint8_t FIELD_ENDIAN_MASK = 0x80;
    static const uint8_t FIELD_BASE_MASK = 0x0F;

    void parseHeader();
    void parseData();
protected:
    std::shared_ptr<uint8_t[]> binaryData;
    size_t binarySize;
    
    FitFileHeader fitHeader;
    std::vector<FitDefinitionMessage> fitDefinitions;
    std::vector<FitDataMessage> fitDataMessages;
    
    bool headerParsed;
    bool dataParsed;
    bool parsed;
public:
    BinaryMapper(const fs::path& filename);
    ~BinaryMapper() = default;

    std::shared_ptr<uint8_t[]> data();
    size_t size();
    bool isParsed();
    void parse();

    const FitFileHeader& header() const { return fitHeader; }
    const std::vector<FitDefinitionMessage>& definitions() const { return fitDefinitions; }
    const std::vector<FitDataMessage>& dataMessages() const { return fitDataMessages; }

    uint8_t read(uint64_t &offset);
    uint16_t read(uint64_t &offset, uint8_t architecture);

    void write(uint64_t &offset, uint16_t value, uint8_t architecture);

    uint16_t CRC();
    void writeCRC();
    void save(const fs::path& filename);
};

} // namespace darauble