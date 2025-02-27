#include "product-scanner.hpp"

#include <iostream>
#include <cstdint>

#include <fit_file_id_mesg.hpp>
#include <fit_device_info_mesg.hpp>

namespace darauble {

void ProductScanner::record(const FitDefinitionMessage& d, const FitDataMessage& m) {
    if (d.globalMessageNumber == FIT_MESG_NUM_FILE_ID) {
        for (auto &f : d.fields) {
            if (f.fieldNumber == fit::FileIdMesg::FieldDefNum::Product) {
                uint64_t offset, recordOffset;
                offset =  recordOffset = m.offset + f.offset;
                uint16_t productId = mapper.read(offset, d.architecture);

                if (productId == FIT_UINT16_INVALID) {
                    continue;
                }

                if (searchProductId == FIT_UINT16_INVALID || productId == searchProductId) {
                    std::cout << "File ID product: " << productId << " at offset " << recordOffset << std::endl;
                    productIdOffsets.push_back({d.architecture, recordOffset});
                }
            }
        }
    } else if (d.globalMessageNumber == FIT_MESG_NUM_DEVICE_INFO) {
        for (auto &f : d.fields) {
            if (f.fieldNumber == fit::DeviceInfoMesg::FieldDefNum::Product) {
                uint64_t offset, recordOffset;
                offset =  recordOffset = m.offset + f.offset;
                uint16_t productId = mapper.read(offset, d.architecture);

                if (productId == FIT_UINT16_INVALID) {
                    continue;
                }

                if (searchProductId == FIT_UINT16_INVALID || productId == searchProductId) {
                    std::cout << "Device Info product: " << productId << " at offset " << recordOffset << std::endl;
                    productIdOffsets.push_back({d.architecture, recordOffset});
                }
            }
        }
    }
}

void ProductScanner::reset() {
    std::cout << "ProductScanner::reset" << std::endl;
    productIdOffsets.clear();
}

} // namespace darauble
