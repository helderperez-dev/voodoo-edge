// protocol.cpp
// Voodoo Edge — Protocol implementation

#include "voodoo_edge/core/protocol.h"
#include "voodoo_edge/platform/hal.h"
#include <cstring>
#include <cstdio>

namespace voodoo {
namespace edge {

// ---------------------------------------------------------------------------
// Message type conversion
// ---------------------------------------------------------------------------

const char* message_type_to_string(MessageType type) {
    switch (type) {
        case MessageType::HELLO:      return "hello";
        case MessageType::AUTH:       return "auth";
        case MessageType::STATE_SYNC: return "state_sync";
        case MessageType::EVENT:      return "event";
        case MessageType::EFFECT:     return "effect";
        case MessageType::EFFECT_ACK: return "effect_ack";
        case MessageType::HEARTBEAT:  return "heartbeat";
        default:                      return "unknown";
    }
}

MessageType message_type_from_string(const char* str) {
    if (strcmp(str, "hello") == 0) return MessageType::HELLO;
    if (strcmp(str, "auth") == 0) return MessageType::AUTH;
    if (strcmp(str, "state_sync") == 0) return MessageType::STATE_SYNC;
    if (strcmp(str, "event") == 0) return MessageType::EVENT;
    if (strcmp(str, "effect") == 0) return MessageType::EFFECT;
    if (strcmp(str, "effect_ack") == 0) return MessageType::EFFECT_ACK;
    if (strcmp(str, "heartbeat") == 0) return MessageType::HEARTBEAT;
    return MessageType::UNKNOWN;
}

const char* ack_status_to_string(EffectAckStatus status) {
    switch (status) {
        case EffectAckStatus::ACCEPTED:  return "accepted";
        case EffectAckStatus::COMPLETED: return "completed";
        case EffectAckStatus::FAILED:    return "failed";
        case EffectAckStatus::REJECTED:  return "rejected";
        default:                         return "unknown";
    }
}

// ---------------------------------------------------------------------------
// EdgeMessage
// ---------------------------------------------------------------------------

EdgeMessage::EdgeMessage() : type(MessageType::UNKNOWN) {
    strncpy(version, PROTOCOL_VERSION, sizeof(version) - 1);
    version[sizeof(version) - 1] = '\0';
    message_id[0] = '\0';
    device_id[0] = '\0';
    timestamp[0] = '\0';
    correlation_id[0] = '\0';
    trace_id[0] = '\0';
    generate_message_id(message_id, sizeof(message_id));

    // Set timestamp using platform timer
    auto& platform = hal::get_platform();
    if (platform.timer) {
        uint32_t ms = platform.timer->millis();
        snprintf(timestamp, sizeof(timestamp), "%lu", ms);
    }
}

size_t EdgeMessage::serialize(char* buffer, size_t buffer_size) const {
    JsonDocument doc;
    doc["version"] = version;
    doc["type"] = message_type_to_string(type);
    doc["message_id"] = message_id;
    doc["device_id"] = device_id;
    doc["timestamp"] = timestamp;

    // Copy payload
    if (!payload_doc.isNull()) {
        doc["payload"] = payload_doc.as<JsonObjectConst>();
    } else {
        doc["payload"].to<JsonObject>();
    }

    if (correlation_id[0] != '\0') {
        doc["correlation_id"] = correlation_id;
    }
    if (trace_id[0] != '\0') {
        doc["trace_id"] = trace_id;
    }

    size_t written = serializeJson(doc, buffer, buffer_size);
    return written;
}

bool EdgeMessage::deserialize(const char* json, size_t len) {
    DeserializationError err = deserializeJson(payload_doc, json, len);
    if (err) return false;

    // Extract envelope fields
    const char* v = payload_doc["version"] | "";
    strncpy(version, v, sizeof(version) - 1);

    const char* t = payload_doc["type"] | "";
    type = message_type_from_string(t);

    const char* mid = payload_doc["message_id"] | "";
    strncpy(message_id, mid, sizeof(message_id) - 1);

    const char* did = payload_doc["device_id"] | "";
    strncpy(device_id, did, sizeof(device_id) - 1);

    const char* ts = payload_doc["timestamp"] | "";
    strncpy(timestamp, ts, sizeof(timestamp) - 1);

    const char* cid = payload_doc["correlation_id"] | "";
    strncpy(correlation_id, cid, sizeof(correlation_id) - 1);

    const char* tid = payload_doc["trace_id"] | "";
    strncpy(trace_id, tid, sizeof(trace_id) - 1);

    return true;
}

JsonObjectConst EdgeMessage::payload() const {
    return payload_doc["payload"].as<JsonObjectConst>();
}

JsonObject EdgeMessage::payload_mutable() {
    if (payload_doc["payload"].isNull()) {
        return payload_doc["payload"].to<JsonObject>();
    }
    return payload_doc["payload"].as<JsonObject>();
}

// ---------------------------------------------------------------------------
// Message builders
// ---------------------------------------------------------------------------

EdgeMessage build_hello(
    const char* device_id,
    const char* device_type,
    const char* firmware_version,
    const char* const* capabilities,
    size_t capability_count
) {
    EdgeMessage msg;
    msg.type = MessageType::HELLO;
    strncpy(msg.device_id, device_id, sizeof(msg.device_id) - 1);

    JsonObject payload = msg.payload_mutable();
    payload["device_id"] = device_id;
    payload["device_type"] = device_type;
    payload["protocol_version"] = PROTOCOL_VERSION;
    payload["firmware_version"] = firmware_version;

    JsonArray caps = payload["capabilities"].to<JsonArray>();
    for (size_t i = 0; i < capability_count; i++) {
        caps.add(capabilities[i]);
    }

    return msg;
}

EdgeMessage build_auth(
    const char* device_id,
    const char* credential
) {
    EdgeMessage msg;
    msg.type = MessageType::AUTH;
    strncpy(msg.device_id, device_id, sizeof(msg.device_id) - 1);

    JsonObject payload = msg.payload_mutable();
    payload["device_id"] = device_id;
    payload["credential"] = credential;
    payload["protocol_version"] = PROTOCOL_VERSION;

    return msg;
}

EdgeMessage build_event(
    const char* device_id,
    const char* event_name,
    JsonObjectConst event_payload,
    const char* message_id
) {
    EdgeMessage msg;
    msg.type = MessageType::EVENT;
    strncpy(msg.device_id, device_id, sizeof(msg.device_id) - 1);
    if (message_id) {
        strncpy(msg.message_id, message_id, sizeof(msg.message_id) - 1);
    }

    JsonObject payload = msg.payload_mutable();
    payload["event_name"] = event_name;
    if (!event_payload.isNull()) {
        payload["event_payload"] = event_payload;
    } else {
        payload["event_payload"].to<JsonObject>();
    }

    return msg;
}

EdgeMessage build_state_sync(
    const char* device_id,
    JsonObjectConst state,
    uint32_t state_version
) {
    EdgeMessage msg;
    msg.type = MessageType::STATE_SYNC;
    strncpy(msg.device_id, device_id, sizeof(msg.device_id) - 1);

    JsonObject payload = msg.payload_mutable();
    payload["state_version"] = state_version;
    if (!state.isNull()) {
        payload["state"] = state;
    } else {
        payload["state"].to<JsonObject>();
    }

    return msg;
}

EdgeMessage build_effect_ack(
    const char* device_id,
    const char* effect_id,
    const char* execution_id,
    EffectAckStatus status,
    const char* error
) {
    EdgeMessage msg;
    msg.type = MessageType::EFFECT_ACK;
    strncpy(msg.device_id, device_id, sizeof(msg.device_id) - 1);

    JsonObject payload = msg.payload_mutable();
    payload["effect_id"] = effect_id;
    if (execution_id && execution_id[0] != '\0') {
        payload["execution_id"] = execution_id;
    }
    payload["status"] = ack_status_to_string(status);
    if (error) {
        payload["error"] = error;
    }

    return msg;
}

EdgeMessage build_heartbeat(
    const char* device_id,
    uint32_t state_version,
    uint32_t uptime_seconds
) {
    EdgeMessage msg;
    msg.type = MessageType::HEARTBEAT;
    strncpy(msg.device_id, device_id, sizeof(msg.device_id) - 1);

    JsonObject payload = msg.payload_mutable();
    payload["state_version"] = state_version;
    payload["uptime_seconds"] = uptime_seconds;

    return msg;
}

// ---------------------------------------------------------------------------
// Validation
// ---------------------------------------------------------------------------

bool validate_protocol_version(const char* version) {
    return version && strcmp(version, PROTOCOL_VERSION) == 0;
}

bool validate_event_name(const char* name) {
    if (!name || name[0] == '\0') return false;

    // Must contain at least one dot
    const char* dot = strchr(name, '.');
    if (!dot || dot == name) return false;

    // Must not start or end with dot
    size_t len = strlen(name);
    if (name[len - 1] == '.') return false;

    // Must be lowercase alphanumeric with dots and underscores
    for (size_t i = 0; i < len; i++) {
        char c = name[i];
        if (!((c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '.' || c == '_')) {
            return false;
        }
    }

    return true;
}

bool validate_message(const EdgeMessage& msg) {
    if (msg.type == MessageType::UNKNOWN) return false;
    if (msg.device_id[0] == '\0') return false;
    if (msg.message_id[0] == '\0') return false;
    return true;
}

void generate_message_id(char* buffer, size_t size) {
    if (size < 25) return;  // "msg_" + 20 hex + null

    auto& platform = hal::get_platform();
    buffer[0] = 'm';
    buffer[1] = 's';
    buffer[2] = 'g';
    buffer[3] = '_';

    // Generate 20 hex characters (80 bits of randomness)
    uint8_t random_bytes[10];
    if (platform.random) {
        platform.random->fill(random_bytes, sizeof(random_bytes));
    } else {
        // Fallback: use timer-based pseudo-random with persistent state
        static uint32_t seed = 0;
        if (seed == 0) {
            seed = platform.timer ? platform.timer->millis() : 12345;
            if (seed == 0) seed = 1;
        }
        for (size_t i = 0; i < sizeof(random_bytes); i++) {
            seed = seed * 1103515245 + 12345;
            random_bytes[i] = (seed >> 16) & 0xFF;
        }
    }

    for (size_t i = 0; i < 10; i++) {
        snprintf(buffer + 4 + i * 2, 3, "%02x", random_bytes[i]);
    }
    buffer[24] = '\0';
}

} // namespace edge
} // namespace voodoo
