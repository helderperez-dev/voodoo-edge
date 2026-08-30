// mqtt_transport.cpp
// Voodoo Edge — MQTT transport implementation

#ifndef VOODOO_EDGE_NATIVE_TEST

#include "voodoo_edge/platform/esp32/mqtt_transport.h"
#include "voodoo_edge/platform/hal.h"
#include <cstring>
#include <cstdio>

namespace voodoo {
namespace platform {
namespace esp32 {

MqttTransport* MqttTransport::_instance = nullptr;

MqttTransport::MqttTransport(const Config& config)
    : _config(config)
    , _effect_callback(nullptr)
    , _effect_user_data(nullptr)
    , _retry_ms(config.retry_initial_ms)
    , _last_connect_attempt(0)
{
    _mqtt.setClient(_wifi_client);
    _mqtt.setServer(config.broker_host, config.broker_port);
    _mqtt.setBufferSize(1024);
    _instance = this;
    _mqtt.setCallback(static_callback);
}

bool MqttTransport::begin() {
    return connect();
}

void MqttTransport::end() {
    _mqtt.disconnect();
}

bool MqttTransport::connected() {
    return _mqtt.connected();
}

void MqttTransport::loop() {
    if (!_mqtt.connected()) {
        auto& platform = hal::get_platform();
        uint32_t now = platform.timer->millis();
        if (now - _last_connect_attempt >= _retry_ms) {
            _last_connect_attempt = now;
            if (connect()) {
                _retry_ms = _config.retry_initial_ms;
            } else {
                _retry_ms = (_retry_ms * 2 < _config.retry_max_ms)
                    ? _retry_ms * 2
                    : _config.retry_max_ms;
            }
        }
        return;
    }
    _mqtt.loop();
}

bool MqttTransport::publish(const edge::EdgeMessage& msg) {
    if (!_mqtt.connected()) return false;

    char buffer[MAX_MESSAGE_BUFFER_SIZE];
    size_t len = msg.serialize(buffer, sizeof(buffer));
    if (len == 0) return false;

    // Determine topic based on message type
    char topic[128];
    const char* kind = "events";
    switch (msg.type) {
        case edge::MessageType::HELLO:      kind = "hello"; break;
        case edge::MessageType::AUTH:       kind = "auth"; break;
        case edge::MessageType::STATE_SYNC: kind = "state"; break;
        case edge::MessageType::EVENT:      kind = "events"; break;
        case edge::MessageType::EFFECT_ACK: kind = "ack"; break;
        case edge::MessageType::HEARTBEAT:  kind = "heartbeat"; break;
        default: break;
    }
    snprintf(topic, sizeof(topic), "%s/%s/%s", MQTT_TOPIC_PREFIX, _config.device_id, kind);

    return _mqtt.publish(topic, buffer, len);
}

bool MqttTransport::subscribe_effects() {
    if (!_mqtt.connected()) return false;

    char topic[128];
    snprintf(topic, sizeof(topic), "%s/%s/effects", MQTT_TOPIC_PREFIX, _config.device_id);
    return _mqtt.subscribe(topic, 1);
}

void MqttTransport::on_effect(edge::EffectCallback callback, void* user_data) {
    _effect_callback = callback;
    _effect_user_data = user_data;
}

bool MqttTransport::connect() {
    auto& platform = hal::get_platform();
    if (platform.logging) {
        platform.logging->printf("[mqtt] connecting to %s:%d\n",
            _config.broker_host, _config.broker_port);
    }

    bool ok = _mqtt.connect(
        _config.device_id,
        _config.username,
        _config.password
    );

    if (ok) {
        if (platform.logging) {
            platform.logging->printf("[mqtt] connected\n");
        }
        subscribe_effects();
    }

    return ok;
}

void MqttTransport::handle_message(const char* topic, const uint8_t* payload, unsigned int length) {
    if (!_effect_callback) return;

    edge::EdgeMessage msg;
    if (!msg.deserialize(reinterpret_cast<const char*>(payload), length)) {
        return;
    }

    if (msg.type == edge::MessageType::EFFECT) {
        _effect_callback(msg, _effect_user_data);
    }
}

void MqttTransport::static_callback(char* topic, uint8_t* payload, unsigned int length) {
    if (_instance) {
        _instance->handle_message(topic, payload, length);
    }
}

} // namespace esp32
} // namespace platform
} // namespace voodoo

#endif // VOODOO_EDGE_NATIVE_TEST
