// protocol.h
// Voodoo Edge — voodoo-edge/v1 protocol definitions
//
// Transport-independent message schemas matching the Voodoo Runtime.
// All fields are protocol-level primitives: string, int, bool, object, array.

#ifndef VODOO_CORE_PROTOCOL_H
#define VODOO_CORE_PROTOCOL_H

#include <ArduinoJson.h>
#include <cstdint>
#include <cstddef>

// Protocol constants (can be overridden at build time)
#ifndef VOODOO_EDGE_PROTOCOL_VERSION
#define VOODOO_EDGE_PROTOCOL_VERSION "1"
#endif

#ifndef VOODOO_EDGE_FIRMWARE_VERSION
#define VOODOO_EDGE_FIRMWARE_VERSION "1.0.0"
#endif

#ifndef VOODOO_EDGE_PROTOCOL_NAME
#define VOODOO_EDGE_PROTOCOL_NAME "voodoo-edge"
#endif

// Buffer size limits
#ifndef MAX_DEVICE_ID_LEN
#define MAX_DEVICE_ID_LEN 32
#endif

#ifndef MAX_CREDENTIAL_LEN
#define MAX_CREDENTIAL_LEN 128
#endif

#ifndef MAX_CAPABILITY_NAME_LEN
#define MAX_CAPABILITY_NAME_LEN 64
#endif

#ifndef MAX_EVENT_NAME_LEN
#define MAX_EVENT_NAME_LEN 64
#endif

#ifndef MAX_MESSAGE_BUFFER_SIZE
#define MAX_MESSAGE_BUFFER_SIZE 1024
#endif

#ifndef IDEMPOTENCY_CACHE_SIZE
#define IDEMPOTENCY_CACHE_SIZE 32
#endif

namespace voodoo {
namespace edge {

// ---------------------------------------------------------------------------
// Protocol identity
// ---------------------------------------------------------------------------
constexpr const char* PROTOCOL_NAME = VOODOO_EDGE_PROTOCOL_NAME;
constexpr const char* PROTOCOL_VERSION = VOODOO_EDGE_PROTOCOL_VERSION;

// ---------------------------------------------------------------------------
// Message types — the seven v1 types
// ---------------------------------------------------------------------------
enum class MessageType : uint8_t {
    HELLO = 0,
    AUTH = 1,
    STATE_SYNC = 2,
    EVENT = 3,
    EFFECT = 4,
    EFFECT_ACK = 5,
    HEARTBEAT = 6,
    UNKNOWN = 255
};

const char* message_type_to_string(MessageType type);
MessageType message_type_from_string(const char* str);

// ---------------------------------------------------------------------------
// Effect ACK statuses
// ---------------------------------------------------------------------------
enum class EffectAckStatus : uint8_t {
    ACCEPTED = 0,
    COMPLETED = 1,
    FAILED = 2,
    REJECTED = 3
};

const char* ack_status_to_string(EffectAckStatus status);

// ---------------------------------------------------------------------------
// EdgeMessage — canonical transport-independent envelope
// ---------------------------------------------------------------------------
struct EdgeMessage {
    char version[8];
    MessageType type;
    char message_id[32];
    char device_id[MAX_DEVICE_ID_LEN];
    char timestamp[40];
    JsonDocument payload_doc;  // owns the payload JSON
    char correlation_id[32];
    char trace_id[32];

    EdgeMessage();

    // Serialize to JSON string. Returns bytes written, or 0 on error.
    size_t serialize(char* buffer, size_t buffer_size) const;

    // Deserialize from JSON string. Returns true on success.
    bool deserialize(const char* json, size_t len);

    // Access payload as JsonObject
    JsonObjectConst payload() const;
    JsonObject payload_mutable();
};

// ---------------------------------------------------------------------------
// Message builders — construct protocol-correct envelopes
// ---------------------------------------------------------------------------

// Build a HELLO message (device announcement)
EdgeMessage build_hello(
    const char* device_id,
    const char* device_type,
    const char* firmware_version,
    const char* const* capabilities,
    size_t capability_count
);

// Build an AUTH message (credential presentation)
EdgeMessage build_auth(
    const char* device_id,
    const char* credential
);

// Build an EVENT message (device event publication)
EdgeMessage build_event(
    const char* device_id,
    const char* event_name,
    JsonObjectConst event_payload,
    const char* message_id = nullptr
);

// Build a STATE_SYNC message (state report)
EdgeMessage build_state_sync(
    const char* device_id,
    JsonObjectConst state,
    uint32_t state_version
);

// Build an EFFECT_ACK message (acknowledge an effect)
EdgeMessage build_effect_ack(
    const char* device_id,
    const char* effect_id,
    const char* execution_id,
    EffectAckStatus status,
    const char* error = nullptr
);

// Build a HEARTBEAT message (liveness signal)
EdgeMessage build_heartbeat(
    const char* device_id,
    uint32_t state_version,
    uint32_t uptime_seconds
);

// ---------------------------------------------------------------------------
// Validation
// ---------------------------------------------------------------------------

// Validate protocol version in a message
bool validate_protocol_version(const char* version);

// Validate event name format (dot-namespaced lowercase)
bool validate_event_name(const char* name);

// Validate a message has required fields for its type
bool validate_message(const EdgeMessage& msg);

// Generate a message ID: "msg_" + 20 hex chars
void generate_message_id(char* buffer, size_t size);

} // namespace edge
} // namespace voodoo

#endif // VODOO_CORE_PROTOCOL_H
