#pragma once

#include "binary-scanner.hpp"
#include <unordered_set>
#include <map>
#include <string>
#include <vector>

#include <fit.hpp>

namespace darauble {

class PrintScanner : public BinaryScanner {
protected:
    int32_t lastGlobalMessageNumber;
    std::string lastMessageHeader;
    std::vector<uint16_t> lastFieldWidths;
    std::unordered_set<uint8_t> messageFilter;
    std::unordered_set<uint8_t> fieldFilter;

    void printHeader(const FitDefinitionMessage& d);
    void printMessage(const FitDefinitionMessage& d, const FitDataMessage& m);
public:
    const std::map<uint8_t, uint32_t> FIT_TYPE_WIDTH = {
        {FIT_BASE_TYPE_ENUM, 3},
        {FIT_BASE_TYPE_SINT8, 4},
        {FIT_BASE_TYPE_UINT8, 3},
        {FIT_BASE_TYPE_SINT16, 6},
        {FIT_BASE_TYPE_UINT16, 5},
        {FIT_BASE_TYPE_SINT32, 11},
        {FIT_BASE_TYPE_UINT32, 10},
        {FIT_BASE_TYPE_STRING, 0},
        {FIT_BASE_TYPE_FLOAT32, 11},
        {FIT_BASE_TYPE_FLOAT64, 31},
        {FIT_BASE_TYPE_UINT8Z, 3},
        {FIT_BASE_TYPE_UINT16Z, 5},
        {FIT_BASE_TYPE_UINT32Z, 10},
        {FIT_BASE_TYPE_BYTE, 3},
        {FIT_BASE_TYPE_SINT64, 20},
        {FIT_BASE_TYPE_UINT64, 20},
        {FIT_BASE_TYPE_UINT64Z, 20},
    };

    PrintScanner(BinaryMapper& _mapper, const std::unordered_set<uint8_t>& _messageFilter, const std::unordered_set<uint8_t>& _fieldFilter) :
        BinaryScanner(_mapper), messageFilter {_messageFilter}, fieldFilter {_fieldFilter}, lastGlobalMessageNumber {-1}, lastMessageHeader {""}
    {}

    virtual void reset() override;
    virtual void record(const FitDefinitionMessage& d, const FitDataMessage& m) override;
    virtual void end() override;

    // const std::unordered_set<uint8_t>& messageFilter() const {
    //     return messageFilter;
    // }

    // const std::unordered_set<uint8_t>& fieldFilter() const {
    //     return fieldFilter;
    // }
};

} // namespace darauble
