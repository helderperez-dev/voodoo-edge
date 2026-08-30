// effect_dispatcher.h
// Voodoo Edge — Effect dispatch pipeline
//
// Receives EFFECT messages from the transport, validates them, dispatches
// to the appropriate capability handler, and sends EFFECT_ACK responses.

#ifndef VODOO_CORE_EFFECT_DISPATCHER_H
#define VODOO_CORE_EFFECT_DISPATCHER_H

#include "voodoo_edge/core/protocol.h"
#include "voodoo_edge/core/transport.h"
#include "voodoo_edge/core/capability_registry.h"
#include "voodoo_edge/core/idempotency.h"

namespace voodoo {
namespace edge {

// ---------------------------------------------------------------------------
// EffectDispatcher
// ---------------------------------------------------------------------------
class EffectDispatcher {
public:
    EffectDispatcher(
        Transport& transport,
        CapabilityRegistry& capabilities,
        IdempotencyCache& idempotency,
        const char* device_id
    );

    // Process an incoming EFFECT message (called by transport callback)
    void dispatch(const EdgeMessage& effect);

    // Set the device_id (for re-authentication scenarios)
    void set_device_id(const char* device_id);

private:
    Transport& _transport;
    CapabilityRegistry& _capabilities;
    IdempotencyCache& _idempotency;
    char _device_id[MAX_DEVICE_ID_LEN];

    void send_ack(
        const char* effect_id,
        const char* execution_id,
        EffectAckStatus status,
        const char* error = nullptr
    );
};

} // namespace edge
} // namespace voodoo

#endif // VODOO_CORE_EFFECT_DISPATCHER_H
