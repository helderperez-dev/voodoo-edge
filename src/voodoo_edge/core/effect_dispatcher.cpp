// effect_dispatcher.cpp
// Voodoo Edge — Effect dispatch implementation

#include "voodoo_edge/core/effect_dispatcher.h"
#include "voodoo_edge/platform/hal.h"
#include <cstring>
#include <cstdio>

namespace voodoo {
namespace edge {

EffectDispatcher::EffectDispatcher(
    Transport& transport,
    CapabilityRegistry& capabilities,
    IdempotencyCache& idempotency,
    const char* device_id
)
    : _transport(transport)
    , _capabilities(capabilities)
    , _idempotency(idempotency)
{
    strncpy(_device_id, device_id, sizeof(_device_id) - 1);
    _device_id[sizeof(_device_id) - 1] = '\0';
}

void EffectDispatcher::set_device_id(const char* device_id) {
    strncpy(_device_id, device_id, sizeof(_device_id) - 1);
    _device_id[sizeof(_device_id) - 1] = '\0';
}

void EffectDispatcher::dispatch(const EdgeMessage& effect) {
    auto& log = hal::get_platform().logging;

    // Step 1: Validate envelope
    if (effect.type != MessageType::EFFECT) {
        if (log) log->printf("[dispatcher] not an effect message\n");
        return;
    }

    // Step 2: Validate device_id
    if (strcmp(effect.device_id, _device_id) != 0) {
        if (log) log->printf("[dispatcher] device_id mismatch: %s vs %s\n",
            effect.device_id, _device_id);
        return;
    }

    // Step 3: Extract capability name
    JsonObjectConst payload = effect.payload();
    const char* capability = payload["capability"] | "";
    if (capability[0] == '\0') {
        if (log) log->printf("[dispatcher] missing capability\n");
        send_ack(effect.message_id, "", EffectAckStatus::REJECTED, "missing capability");
        return;
    }

    // Step 4: Validate capability exists
    CapabilityHandler handler = _capabilities.find(capability);
    if (!handler) {
        if (log) log->printf("[dispatcher] unknown capability: %s\n", capability);
        send_ack(effect.message_id, "", EffectAckStatus::REJECTED, "unknown capability");
        return;
    }

    // Step 5: Check idempotency
    if (_idempotency.has_seen(effect.message_id)) {
        if (log) log->printf("[dispatcher] duplicate effect: %s\n", effect.message_id);
        send_ack(effect.message_id, "", EffectAckStatus::COMPLETED, nullptr);
        return;
    }

    // Step 6: Execute handler
    const char* execution_id = payload["execution_id"] | "";
    JsonDocument result_doc;
    JsonObject result = result_doc.to<JsonObject>();

    bool success = handler(effect, result, _capabilities.find_user_data(capability));

    // Step 7: Mark as seen
    _idempotency.mark_seen(effect.message_id);

    // Step 8: Send ACK
    if (success) {
        send_ack(effect.message_id, execution_id, EffectAckStatus::COMPLETED);
    } else {
        send_ack(effect.message_id, execution_id, EffectAckStatus::FAILED, "handler failed");
    }
}

void EffectDispatcher::send_ack(
    const char* effect_id,
    const char* execution_id,
    EffectAckStatus status,
    const char* error
) {
    EdgeMessage ack = build_effect_ack(_device_id, effect_id, execution_id, status, error);
    _transport.publish(ack);
}

} // namespace edge
} // namespace voodoo
