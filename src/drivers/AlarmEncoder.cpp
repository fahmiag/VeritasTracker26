#include "AlarmEncoder.h"

namespace {
void Put32(uint8_t* output, uint32_t value) {
    for (uint8_t i = 0; i < 4; ++i) output[i] = uint8_t(value >> (8 * i));
}
}
size_t AlarmEncoder::Encode(const AlarmManager::Event& event,
                            uint8_t version, uint8_t emergencyType, uint8_t tamperType,
                            uint8_t* output, size_t capacity) {
    if (!output || capacity < Size || emergencyType == tamperType) return 0;
    if (event.kind != AlarmManager::Kind::Emergency &&
        event.kind != AlarmManager::Kind::Tamper) return 0;
    output[0] = version;
    output[1] = event.kind == AlarmManager::Kind::Emergency ? emergencyType : tamperType;
    Put32(output + 2, event.id);
    Put32(output + 6, event.uptimeMs);
    output[10] = event.status;
    output[11] = 0;
    return Size;
}
