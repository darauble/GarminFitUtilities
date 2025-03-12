#include "print-scanner.hpp"

#include <iomanip>
#include <iostream>
#include <sstream>

#include <fit_profile.hpp>

namespace darauble {

void PrintScanner::printHeader(const FitDefinitionMessage& d) {
    std::ostringstream oss1, oss2;

    for (auto field : d.fields) {
        if (field.baseType != FIT_BASE_TYPE_STRING) {
            oss1 << "+-" << std::setw(FIT_TYPE_WIDTH.at(field.baseType)) << std::setfill('-') << "-" << "-";
            oss2 << "| " << std::setw(FIT_TYPE_WIDTH.at(field.baseType)) << std::setfill(' ') << +field.fieldNumber << " ";
        } else {
            oss1 << "|-" << std::setw(field.size) << std::setfill('-') << "-" << "-";
            oss2 << "| " << std::setw(field.size) << std::setfill(' ') << +field.fieldNumber << " ";
        }
    }

    oss1 << "+";
    oss2 << "|";

    auto messageMeta = fit::Profile::GetMesg(d.globalMessageNumber);

    std::cout << "====  Message #" << d.globalMessageNumber;

    if (messageMeta) {
        std::cout << " (" << messageMeta->name << ")";
    }

    std::cout << "  ====" << std::endl;
    
    std::cout << oss1.str() << std::endl;
    std::cout << oss2.str() << std::endl;
    std::cout << oss1.str() << std::endl;

    lastMessageHeader = oss1.str();
}

void PrintScanner::printMessage(const FitDefinitionMessage& d, const FitDataMessage& m) {
    std::ostringstream oss;

    
    for (auto field : d.fields) {
        uint64_t offset = m.offset + field.offset;

        if (field.baseType != FIT_BASE_TYPE_STRING) {
            oss << "| " << std::setw(FIT_TYPE_WIDTH.at(field.baseType)) << std::setfill(' ');

            switch (field.baseType) {
                case FIT_BASE_TYPE_ENUM:
                case FIT_BASE_TYPE_BYTE:
                case FIT_BASE_TYPE_UINT8:
                case FIT_BASE_TYPE_UINT8Z:
                    oss << +mapper.read(offset);
                    break;
                
                case FIT_BASE_TYPE_SINT8:
                    oss << +mapper.readS(offset);
                    break;
                
                case FIT_BASE_TYPE_UINT16:
                case FIT_BASE_TYPE_UINT16Z:
                    oss << mapper.readU16(offset, d.architecture);
                    break;

                case FIT_BASE_TYPE_SINT16:
                    oss << mapper.readS16(offset, d.architecture);
                    break;

                case FIT_BASE_TYPE_UINT32:
                case FIT_BASE_TYPE_UINT32Z:
                    oss << mapper.readU32(offset, d.architecture);
                    break;

                case FIT_BASE_TYPE_SINT32:
                    oss << mapper.readS32(offset, d.architecture);
                    break;

                case FIT_BASE_TYPE_FLOAT32:
                    oss << mapper.readFloat(offset, d.architecture);
                    break;
                
                case FIT_BASE_TYPE_FLOAT64:
                    oss << mapper.readDouble(offset, d.architecture);
                    break;

                case FIT_BASE_TYPE_UINT64:
                    oss << mapper.readU64(offset, d.architecture);
                    break;

                case FIT_BASE_TYPE_SINT64:
                    oss << mapper.readS64(offset, d.architecture);
                    break;

                default:
                    oss << "???";
                    break;
            }
            oss << " ";
        } else {
            std::string extractedString(reinterpret_cast<const char*>(&mapper.data()[offset]), field.size);
            oss << "| "  << std::setfill(' ') << std::setw(field.size) << extractedString.c_str() << " ";
        }
    }

    oss << "|";

    std::cout << oss.str() << std::endl;
}

void PrintScanner::record(const FitDefinitionMessage& d, const FitDataMessage& m) {
    if (!messageFilter.empty() && messageFilter.find(d.globalMessageNumber) == messageFilter.end()) {
        return;
    }

    if (lastGlobalMessageNumber != d.globalMessageNumber) {
        if (lastMessageHeader.length() > 0) {
            std::cout << lastMessageHeader << std::endl << std::endl;
        }
        printHeader(d);
        lastGlobalMessageNumber = d.globalMessageNumber;
    }

    printMessage(d, m);
}

void PrintScanner::reset() {
    lastMessageHeader = "";
    lastGlobalMessageNumber = -1;
}

} // namespace darauble
