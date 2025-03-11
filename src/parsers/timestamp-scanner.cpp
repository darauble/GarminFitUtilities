#include "timestamp-scanner.hpp"
#include "fit_profile.hpp"
#include "fit_file_id_mesg.hpp"
#include "fit_lap_mesg.hpp"
#include "fit_session_mesg.hpp"

#include <cstdint>

namespace darauble {

void TimestampScanner::record(const FitDefinitionMessage& d, const FitDataMessage& m) {
    for (auto &f : d.fields) {
        if (
            f.fieldNumber == 253 // Universal Timestamp field number
            || (d.globalMessageNumber == FIT_MESG_NUM_FILE_ID && f.fieldNumber == fit::FileIdMesg::FieldDefNum::TimeCreated)
            || (d.globalMessageNumber == FIT_MESG_NUM_SESSION && f.fieldNumber == fit::SessionMesg::FieldDefNum::StartTime)
            || (d.globalMessageNumber == FIT_MESG_NUM_LAP && f.fieldNumber == fit::LapMesg::FieldDefNum::StartTime)
        ) {
            uint64_t offset, recordOffset;
            offset =  recordOffset = m.offset + f.offset;
            uint32_t ts = mapper.readU32(offset, d.architecture);

            if (ts == FIT_UINT32_INVALID) {
                continue;
            }

            timestampIdOffsets.push_back({ d, recordOffset });
        }
    }
}

void TimestampScanner::reset() {
    timestampIdOffsets.clear();
}

} // namespace darauble
