#include "print-scanner.hpp"

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <format>
#include <sstream>

#include <fit_profile.hpp>

/*
TODO: 
  * +Rodyti datas gražiu formatu (daryti optional ar ne?.. Gal daryti originalą optional su kokiu "raw")
  * Paversti skaičius į kablelinius kai yra daliklis ir postūmis (pvz. svoris. Atsižvelgti į "raw")
  * Paversti trukmę į valandas/minutes/sekundes (paieškoti algoritmo).
  * Padaryti papildomas parinktis kaip filtrą, pvz. offset|raw.
  * +Padaryti optionų struktūrą skaneriui.
  * Rodyti koordinates įprastu skaitomu formatu (tik su optionu!).
*/

namespace darauble {

void PrintScanner::defaultOptions(PrintScannerOptions &o) {
    o.offset = false;
    o.raw = false;
}

void PrintScanner::printHeader(const FitDefinitionMessage& d) {
    std::ostringstream oss1, oss2;
    auto messageMeta = fit::Profile::GetMesg(d.globalMessageNumber);

    for (auto field : d.fields) {
        auto fieldMeta = fit::Profile::GetField(d.globalMessageNumber, field.fieldNumber);
        std::ostringstream ossFieldName;
        ossFieldName << (fieldMeta ? fieldMeta->name : "") << " " << +field.fieldNumber;
        int width {0};

        if (field.baseType != FIT_BASE_TYPE_STRING) {
            if (!options.raw && fieldMeta && fieldMeta->profileType == fit::Profile::Type::DateTime) {
                // Date/Time in ISO is 19 chars
                width = std::max(static_cast<size_t>(ossFieldName.str().length()), static_cast<size_t>(19));
            } else {
                width = std::max(static_cast<size_t>(ossFieldName.str().length()), static_cast<size_t>(FIT_TYPE_WIDTH.at(field.baseType)));
            }
        } else {
            width = std::max(static_cast<size_t>(ossFieldName.str().length()), static_cast<size_t>(field.size));
        }

        if (options.offset) {
            width = std::max(width, 10);
        }
        
        oss1 << "+-" << std::setw(width) << std::setfill('-') << "-" << "-";
        oss2 << "| " << std::setw(width) << std::setfill(' ') << ossFieldName.str() << " ";

        lastFieldWidths.push_back(width);
    }

    oss1 << "+";
    oss2 << "|";

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
    std::ostringstream oss, ossOffset;

    size_t i = 0;

    for (auto field : d.fields) {
        auto fieldMeta = fit::Profile::GetField(d.globalMessageNumber, field.fieldNumber);
        uint64_t offset = m.offset + field.offset;
        uint16_t fieldWidth = lastFieldWidths[i++];
        
        if (options.offset) {
            std::ostringstream ossAtOffset;
            ossAtOffset << "@" << offset;
            ossOffset << "| " << std::setw(fieldWidth) << std::setfill(' ')
                << ossAtOffset.str().c_str() << " ";
        }

        oss << "| ";

        if (field.baseType != FIT_BASE_TYPE_STRING) {
            oss << std::setw(fieldWidth) << std::setfill(' ');
        }
        

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
                if (!options.raw && fieldMeta && fieldMeta->profileType == fit::Profile::Type::DateTime) {
                    oss << mapper.readDateTime(offset, d.architecture);
                } else {
                    oss << mapper.readU32(offset, d.architecture);
                }
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
            
            case FIT_BASE_TYPE_STRING: {
                    std::string extractedString(reinterpret_cast<const char*>(&mapper.data()[offset]), field.size);

                    size_t utf8_length = 0;
                    for (char value : extractedString)
                    {
                        if ((value & 0xc0) != 0x80)
                        {
                            ++utf8_length;
                        }
                    }

                    oss << std::setw(fieldWidth + extractedString.length() - utf8_length) << std::setfill(' ');
                    oss << extractedString.c_str();
                }
                break;

            default:
                oss << "???";
                break;
        }
        oss << " ";
    }

    if (options.offset) {
        ossOffset << "|";
    }

    oss << "|";

    if (options.offset) {
        std::cout << ossOffset.str() << std::endl;
    }
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
        
        lastFieldWidths.clear();
        printHeader(d);
        lastGlobalMessageNumber = d.globalMessageNumber;
    }

    printMessage(d, m);
}

void PrintScanner::reset() {
    lastMessageHeader = "";
    lastGlobalMessageNumber = -1;
}

void PrintScanner::end() {
    if (lastMessageHeader.length() > 0) {
        std::cout << lastMessageHeader << std::endl;
    }
}

} // namespace darauble
