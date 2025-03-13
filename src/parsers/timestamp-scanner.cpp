#include "timestamp-scanner.hpp"

#include <fit_profile.hpp>

#include <cstdint>

namespace darauble {

void TimestampScanner::record(const FitDefinitionMessage& d, const FitDataMessage& m) {
    for (auto &f : d.fields) {
        auto fieldMeta = fit::Profile::GetField(d.globalMessageNumber, f.fieldNumber);

        if (fieldMeta && fieldMeta->profileType == fit::Profile::Type::DateTime) {
            uint64_t offset, recordOffset;
            offset =  recordOffset = m.offset + f.offset;
            uint32_t ts = mapper.readU32(offset, d.architecture);

            if (ts == FIT_UINT32_INVALID) {
                continue;
            }

            timestampIdOffsets.push_back({ d, fieldMeta->name, recordOffset });
        }
    }
}

void TimestampScanner::reset() {
    timestampIdOffsets.clear();
}

} // namespace darauble
