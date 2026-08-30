// device_state.cpp
// Voodoo Edge — Device state implementation

#include "voodoo_edge/core/device_state.h"

namespace voodoo {
namespace edge {

DeviceState::DeviceState() : _version(0), _synced_version(0) {
    _doc.to<JsonObject>();
}

void DeviceState::set(const char* key, const char* value) {
    _doc[key] = value;
    _version++;
}

void DeviceState::set(const char* key, int value) {
    _doc[key] = value;
    _version++;
}

void DeviceState::set(const char* key, float value) {
    _doc[key] = value;
    _version++;
}

void DeviceState::set(const char* key, bool value) {
    _doc[key] = value;
    _version++;
}

JsonObjectConst DeviceState::state() const {
    return _doc.as<JsonObjectConst>();
}

uint32_t DeviceState::version() const {
    return _version;
}

bool DeviceState::has_changed() const {
    return _version != _synced_version;
}

void DeviceState::mark_synced() {
    _synced_version = _version;
}

EdgeMessage DeviceState::build_sync_message(const char* device_id) const {
    return build_state_sync(device_id, state(), _version);
}

} // namespace edge
} // namespace voodoo
