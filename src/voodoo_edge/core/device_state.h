// device_state.h
// Voodoo Edge — Device state tracking
//
// Maintains a JSON document representing the device's current state.
// Tracks a monotonic state version for synchronization.

#ifndef VODOO_CORE_DEVICE_STATE_H
#define VODOO_CORE_DEVICE_STATE_H

#include "voodoo_edge/core/protocol.h"
#include <cstdint>

namespace voodoo {
namespace edge {

class DeviceState {
public:
    DeviceState();

    // Set a state field
    void set(const char* key, const char* value);
    void set(const char* key, int value);
    void set(const char* key, float value);
    void set(const char* key, bool value);

    // Get the current state as a JsonObjectConst
    JsonObjectConst state() const;

    // Get the current state version (monotonic)
    uint32_t version() const;

    // Check if state has changed since last sync
    bool has_changed() const;

    // Mark state as synced (resets changed flag)
    void mark_synced();

    // Build a STATE_SYNC message
    EdgeMessage build_sync_message(const char* device_id) const;

private:
    JsonDocument _doc;
    uint32_t _version;
    uint32_t _synced_version;
};

} // namespace edge
} // namespace voodoo

#endif // VODOO_CORE_DEVICE_STATE_H
