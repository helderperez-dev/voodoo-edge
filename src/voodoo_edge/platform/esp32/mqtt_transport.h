// mqtt_transport.h
// Voodoo Edge — MQTT transport for ESP32
//
// Implements the Transport interface using PubSubClient over WiFi.

#ifndef VODOO_PLATFORM_ESP32_MQTT_TRANSPORT_H
#define VODOO_PLATFORM_ESP32_MQTT_TRANSPORT_H

#include "voodoo_edge/core/transport.h"
#include <PubSubClient.h>
#include <WiFiClient.h>

namespace voodoo {
namespace platform {
namespace esp32 {

// MQTT topic constants
constexpr const char* MQTT_TOPIC_PREFIX = "voodoo/v1/devices";

class MqttTransport : public edge::Transport {
public:
    struct Config {
        const char* broker_host;
        uint16_t broker_port;
        const char* device_id;
        const char* username;
        const char* password;
        uint32_t keepalive_s;
        uint32_t retry_initial_ms;
        uint32_t retry_max_ms;
    };

    MqttTransport(const Config& config);

    // Transport interface
    bool begin() override;
    void end() override;
    bool connected() override;
    void loop() override;
    bool publish(const edge::EdgeMessage& msg) override;
    bool subscribe_effects() override;
    void on_effect(edge::EffectCallback callback, void* user_data = nullptr) override;
    edge::TransportKind kind() const override { return edge::TransportKind::MQTT; }

private:
    Config _config;
    WiFiClient _wifi_client;
    PubSubClient _mqtt;
    edge::EffectCallback _effect_callback;
    void* _effect_user_data;
    uint32_t _retry_ms;
    uint32_t _last_connect_attempt;

    bool connect();
    void handle_message(const char* topic, const uint8_t* payload, unsigned int length);

    // Static callback trampoline for PubSubClient
    static MqttTransport* _instance;
    static void static_callback(char* topic, uint8_t* payload, unsigned int length);
};

} // namespace esp32
} // namespace platform
} // namespace voodoo

#endif // VODOO_PLATFORM_ESP32_MQTT_TRANSPORT_H
