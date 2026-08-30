// event_publisher.cpp
// Voodoo Edge — Event publisher implementation

#include "voodoo_edge/core/event_publisher.h"
#include "voodoo_edge/platform/hal.h"
#include <cstring>

namespace voodoo {
namespace edge {

EventPublisher::EventPublisher(Transport& transport, const char* device_id)
    : _transport(transport)
{
    strncpy(_device_id, device_id, sizeof(_device_id) - 1);
    _device_id[sizeof(_device_id) - 1] = '\0';
}

void EventPublisher::set_device_id(const char* device_id) {
    strncpy(_device_id, device_id, sizeof(_device_id) - 1);
    _device_id[sizeof(_device_id) - 1] = '\0';
}

bool EventPublisher::publish(const char* event_name, JsonObjectConst payload) {
    auto& log = hal::get_platform().logging;

    // Validate event name
    if (!validate_event_name(event_name)) {
        if (log) log->printf("[events] invalid event name: %s\n", event_name);
        return false;
    }

    // Build and publish
    EdgeMessage msg = build_event(_device_id, event_name, payload);
    bool ok = _transport.publish(msg);

    if (log && !ok) {
        log->printf("[events] failed to publish: %s\n", event_name);
    }

    return ok;
}

} // namespace edge
} // namespace voodoo
