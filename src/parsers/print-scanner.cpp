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
  * +Paversti skaičius į kablelinius kai yra daliklis ir postūmis (pvz. svoris. Atsižvelgti į "raw")
  * +Jei invalid reikšmė, neversti į kablelinius.
  * +Paversti trukmę į valandas/minutes/sekundes (paieškoti algoritmo) - jei matavimo vienetas "s".
  * +Padaryti papildomas parinktis kaip filtrą, pvz. offset|raw.
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
        if (!fieldFilter.empty() && fieldFilter.find(field.fieldNumber) == fieldFilter.end()) {
            continue;
        }

        auto fieldMeta = fit::Profile::GetField(d.globalMessageNumber, field.fieldNumber);
        std::ostringstream ossFieldName;
        ossFieldName << (fieldMeta ? fieldMeta->name : "") << " " << +field.fieldNumber;
        int width {0};

        if (field.baseType != FIT_BASE_TYPE_STRING) {
            if (!options.raw && ((fieldMeta && fieldMeta->profileType == fit::Profile::Type::DateTime) || (field.fieldNumber == 253))) {
                // Date/Time in ISO is 19 chars
                width = std::max(static_cast<size_t>(ossFieldName.str().length()), static_cast<size_t>(19));
            } else if (!options.raw && (fieldMeta && fieldMeta->units == "s" && fieldMeta->profileType == fit::Profile::Type::Uint32)) {
                // Hundreds of hours should fit. like 112:03:14.25
                width = std::max(static_cast<size_t>(ossFieldName.str().length()), static_cast<size_t>(12));
            } else if (!options.raw && (fieldMeta && fieldMeta->scale > 1)) {
                // Scaled fields imply they are of double precision by fit::Profile::FIELD
                width = std::max(static_cast<size_t>(ossFieldName.str().length()),  static_cast<size_t>(FIT_TYPE_WIDTH.at(FIT_BASE_TYPE_FLOAT64)));
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
        if (!fieldFilter.empty() && fieldFilter.find(field.fieldNumber) == fieldFilter.end()) {
            continue;
        }

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
                {
                    uint8_t value = mapper.read(offset);
                    bool valid = ((field.baseType == FIT_BASE_TYPE_UINT8) && (value != FIT_UINT8_INVALID))
                                || ((field.baseType == FIT_BASE_TYPE_UINT8Z) && (value != FIT_UINT8Z_INVALID));
                    

                    if (valid && !options.raw && (fieldMeta && fieldMeta->scale > 1)) {
                        oss << (static_cast<double>(value)) / fieldMeta->scale - fieldMeta->offset;
                    } else {
                        oss << +value;
                    }
                }
                break;
            
            case FIT_BASE_TYPE_SINT8:
                {
                    int8_t value = mapper.readS(offset);

                    if ((value != FIT_SINT8_INVALID) && !options.raw && (fieldMeta && fieldMeta->scale > 1)) {
                        oss << (static_cast<double>(value)) / fieldMeta->scale - fieldMeta->offset;
                    } else {
                        oss << +value;
                    }
                }
                break;
            
            case FIT_BASE_TYPE_UINT16:
            case FIT_BASE_TYPE_UINT16Z:
                {
                    uint16_t value = mapper.readU16(offset, d.architecture);
                    // bool valid = true;
                    bool valid = ((field.baseType == FIT_BASE_TYPE_UINT16) && (value != FIT_UINT16_INVALID))
                                || ((field.baseType == FIT_BASE_TYPE_UINT16Z) && (value != FIT_UINT16Z_INVALID));

                    if (valid && !options.raw && (fieldMeta && fieldMeta->scale > 1)) {
                        oss << (static_cast<double>(value)) / fieldMeta->scale - fieldMeta->offset;
                    } else {
                        oss << value;
                    }
                }
                break;

            case FIT_BASE_TYPE_SINT16:
                {
                    int16_t value = mapper.readS16(offset, d.architecture);

                    if ((value != FIT_SINT16_INVALID) && !options.raw && (fieldMeta && fieldMeta->scale > 1)) {
                        oss << (static_cast<double>(value)) / fieldMeta->scale - fieldMeta->offset;
                    } else {
                        oss << value;
                    }
                }
                break;

            case FIT_BASE_TYPE_UINT32:
            case FIT_BASE_TYPE_UINT32Z:
                {
                    if (!options.raw && ((fieldMeta && fieldMeta->profileType == fit::Profile::Type::DateTime) || (field.fieldNumber == 253))) {
                        oss << mapper.readDateTime(offset, d.architecture);
                    } else if (!options.raw && (fieldMeta && fieldMeta->units == "s" && fieldMeta->profileType == fit::Profile::Type::Uint32)) {
                        oss << mapper.readDuration(offset, d.architecture, fieldMeta->scale);
                    } else {
                        uint32_t value = mapper.readU32(offset, d.architecture);
                        bool valid = ((field.baseType == FIT_BASE_TYPE_UINT32) && (value != FIT_UINT32_INVALID))
                            || ((field.baseType == FIT_BASE_TYPE_UINT32Z) && (value != FIT_UINT32Z_INVALID));

                        if (!options.raw && (fieldMeta && fieldMeta->scale > 1)) {
                            oss << (static_cast<double>(value)) / fieldMeta->scale - fieldMeta->offset;
                        } else {
                            oss << value;
                        }
                    }
                }
                break;

            case FIT_BASE_TYPE_SINT32:
                {
                    int32_t value = mapper.readS32(offset, d.architecture);

                    if ((value != FIT_SINT32_INVALID) && !options.raw && (fieldMeta && fieldMeta->scale > 1)) {
                        oss << (static_cast<double>(value)) / fieldMeta->scale - fieldMeta->offset;
                    } else {
                        oss << value;
                    }
                }
                break;

            case FIT_BASE_TYPE_FLOAT32:
                if (!options.raw && (fieldMeta && fieldMeta->scale > 1)) {
                    oss << (static_cast<double>(mapper.readFloat(offset, d.architecture))) / fieldMeta->scale - fieldMeta->offset;
                } else {
                    oss << mapper.readFloat(offset, d.architecture);
                }
                break;
            
            case FIT_BASE_TYPE_FLOAT64:
                if (!options.raw && (fieldMeta && fieldMeta->scale > 1)) {
                    oss << (static_cast<double>(mapper.readDouble(offset, d.architecture))) / fieldMeta->scale - fieldMeta->offset;
                } else {
                    oss << mapper.readDouble(offset, d.architecture);
                }
                break;

            case FIT_BASE_TYPE_UINT64:
                if (!options.raw && (fieldMeta && fieldMeta->scale > 1)) {
                    oss << (static_cast<double>(mapper.readU64(offset, d.architecture))) / fieldMeta->scale - fieldMeta->offset;
                } else {
                    oss << mapper.readU64(offset, d.architecture);
                }
                break;

            case FIT_BASE_TYPE_SINT64:
                if (!options.raw && (fieldMeta && fieldMeta->scale > 1)) {
                    oss << (static_cast<double>(mapper.readU64(offset, d.architecture))) / fieldMeta->scale - fieldMeta->offset;
                } else {
                    oss << mapper.readS64(offset, d.architecture);
                }
                break;
            
            case FIT_BASE_TYPE_STRING:
                {
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

    if (lastDefinitionIndex != m.definitionIndex) {
        if (lastMessageHeader.length() > 0) {
            std::cout << lastMessageHeader << std::endl << std::endl;
        }
        
        lastFieldWidths.clear();
        printHeader(d);
        lastDefinitionIndex = m.definitionIndex;
    }

    printMessage(d, m);
}

void PrintScanner::reset() {
    lastMessageHeader = "";
    lastDefinitionIndex = -1;
}

void PrintScanner::end() {
    if (lastMessageHeader.length() > 0) {
        std::cout << lastMessageHeader << std::endl;
    }
}

} // namespace darauble
