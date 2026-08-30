// transport.h
// Voodoo Edge — Abstract transport interface
//
// Transport implementations (MQTT, HTTP, etc.) must implement this interface.
// The transport is responsible for connecting to the Voodoo Runtime and
// delivering/receiving protocol messages.

#ifndef VODOO_CORE_TRANSPORT_H
#define VODOO_CORE_TRANSPORT_H

#include "voodoo_edge/core/protocol.h"

namespace voodoo {
namespace edge {

// ---------------------------------------------------------------------------
// Transport kind
// ---------------------------------------------------------------------------
enum class TransportKind : uint8_t {
    MQTT = 0,
    HTTP = 1
};

// ---------------------------------------------------------------------------
// Effect callback — invoked when an EFFECT message arrives
// ---------------------------------------------------------------------------
typedef void (*EffectCallback)(const EdgeMessage& effect, void* user_data);

// ---------------------------------------------------------------------------
// Transport — abstract interface
// ---------------------------------------------------------------------------
class Transport {
public:
    virtual ~Transport() = default;

    // Lifecycle
    virtual bool begin() = 0;
    virtual void end() = 0;
    virtual bool connected() = 0;
    virtual void loop() = 0;

    // Publish a message to the runtime
    virtual bool publish(const EdgeMessage& msg) = 0;

    // Subscribe to effect messages from the runtime
    virtual bool subscribe_effects() = 0;

    // Register callback for incoming effects
    virtual void on_effect(EffectCallback callback, void* user_data = nullptr) = 0;

    // Transport type
    virtual TransportKind kind() const = 0;
};

} // namespace edge
} // namespace voodoo

#endif // VODOO_CORE_TRANSPORT_H
