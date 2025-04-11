#pragma once
#include <cstdint>
#include <vector>
#include <string>
#include <filesystem>
#include <unordered_map>

#include <fit_profile.hpp>

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
    uint8_t fieldNumber; // Field number as per each message type (fit_<message>_mesg.hpp)
                        // Don't look at fit_profile.hpp! It has bugs as compared to *_mesg.hpp files!
    uint8_t size;    // Number of bytes
    uint8_t endianAbility;
    uint8_t baseType;
    uint16_t offset; // Offset from the record start (first starts at 1, 0 is the reacord header byte)
    bool developer; // Mark if the field is a developer field or not
};

struct FitDefinitionMessage {
    uint64_t offset; // Where in the file the definition starts 
    // uint8_t reserved;          // Must be 0, not required, so not used ATM.
    uint8_t architecture;      // 0 = little-endian, 1 = big-endian
    uint16_t globalMessageNumber; // Given as per Garmin FIT SDK (fit_profile.hpp)
    uint16_t localMessageNumber; // Local message number in the file
    
    uint8_t fieldCount;
    uint8_t devFieldCount;
    
    uint32_t messageSize;
   
    std::vector<FitFieldDefinition> fields;
};

struct FitDataMessage {
    uint64_t offset; // Where in the file the message starts.
    uint64_t definitionIndex; // An index in the vector of FitDefinitionMessage
    uint8_t localMessageType; // Local message type. NOTE: can be "reused"!
    uint8_t compressedTime; // Indication if compressed time (offset from full timestamp) is used (> 0).
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
    std::unordered_map<uint16_t, std::unordered_map<uint16_t, fit::Profile::FIELD>> devFieldMeta;
    
    bool headerParsed;
    bool dataParsed;
    bool parsed;

    bool showRaw;
public:
    BinaryMapper(const fs::path& filename, bool _showRaw = false);
    ~BinaryMapper() = default;

    std::shared_ptr<uint8_t[]> data();
    size_t size();
    bool isParsed();
    void parse();

    const FitFileHeader& header() const { return fitHeader; }
    const std::vector<FitDefinitionMessage>& definitions() const { return fitDefinitions; }
    const std::vector<FitDataMessage>& dataMessages() const { return fitDataMessages; }

    const fit::Profile::FIELD *getField(const FitDefinitionMessage& d, FitFieldDefinition &f);

    int8_t readS(uint64_t &offset);
    uint8_t read(uint64_t &offset);
    int16_t readS16(uint64_t &offset, uint8_t architecture);
    uint16_t readU16(uint64_t &offset, uint8_t architecture);
    int32_t readS32(uint64_t &offset, uint8_t architecture);
    uint32_t readU32(uint64_t &offset, uint8_t architecture);
    int64_t readS64(uint64_t &offset, uint8_t architecture);
    uint64_t readU64(uint64_t &offset, uint8_t architecture);
    float readFloat(uint64_t &offset, uint8_t architecture);
    double readDouble(uint64_t &offset, uint8_t architecture);

    std::string readString(uint64_t &offset, uint8_t length);
    std::string readDateTime(uint64_t &offset, uint8_t architecture);
    std::string readDuration(uint64_t &offset, uint8_t architecture, double scale);

    void write(uint64_t &offset, uint8_t value);
    void write(uint64_t &offset, uint16_t value, uint8_t architecture);
    void write(uint64_t &offset, uint32_t value, uint8_t architecture);
    void write(uint64_t &offset, char* value, size_t length);

    uint16_t CRC();
    void writeCRC();
    void save(const fs::path& filename);
};

} // namespace darauble