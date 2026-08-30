// main.cpp
// Voodoo Edge — Basic Button + LED Example
//
// Demonstrates the voodoo-edge SDK with:
// - WiFi connection
// - MQTT transport to Voodoo Runtime
// - LED control via "led.control" effect
// - Button press events via "button.pressed"
// - Heartbeat and state sync

#include <Arduino.h>
#include <voodoo_edge.h>

using namespace voodoo;

// ---------------------------------------------------------------------------
// Configuration — replace with your values
// ---------------------------------------------------------------------------
// In production, use a secrets.h file (see secrets.example.h)
#ifndef WIFI_SSID
#define WIFI_SSID "your-wifi-ssid"
#endif

#ifndef WIFI_PASSWORD
#define WIFI_PASSWORD "your-wifi-password"
#endif

#ifndef MQTT_BROKER_HOST
#define MQTT_BROKER_HOST "192.168.1.100"
#endif

#ifndef MQTT_BROKER_PORT
#define MQTT_BROKER_PORT 1883
#endif

#ifndef DEVICE_TYPE
#define DEVICE_TYPE "esp32"
#endif

#ifndef FIRMWARE_VERSION
#define FIRMWARE_VERSION "1.0.0"
#endif

// Hardware pins
#define PIN_LED 2
#define PIN_BUTTON 0

// Timing
#define HEARTBEAT_INTERVAL_MS 30000
#define STATE_SYNC_INTERVAL_MS 60000

// ---------------------------------------------------------------------------
// Global state
// ---------------------------------------------------------------------------
static hal::Platform* platform = nullptr;
static hardware::LedController* led = nullptr;
static hardware::ButtonController* button = nullptr;
static platform::esp32::MqttTransport* transport = nullptr;
static edge::CapabilityRegistry* capabilities = nullptr;
static edge::EffectDispatcher* dispatcher = nullptr;
static edge::EventPublisher* events = nullptr;
static edge::DeviceState* state = nullptr;
static edge::IdempotencyCache* idempotency = nullptr;

static char device_id[MAX_DEVICE_ID_LEN] = {0};
static uint32_t last_heartbeat = 0;
static uint32_t last_state_sync = 0;

// ---------------------------------------------------------------------------
// Capability handlers
// ---------------------------------------------------------------------------

// LED control: {"action": "on"} | {"action": "off"} | {"action": "toggle"}
static bool handle_led_control(
    const edge::EdgeMessage& effect,
    JsonObject result,
    void* user_data
) {
    auto* l = static_cast<hardware::LedController*>(user_data);
    JsonObjectConst payload = effect.payload();
    const char* action = payload["action"] | "";

    if (strcmp(action, "on") == 0) {
        l->on();
    } else if (strcmp(action, "off") == 0) {
        l->off();
    } else if (strcmp(action, "toggle") == 0) {
        l->toggle();
    } else {
        result["error"] = "unknown action";
        return false;
    }

    result["led_state"] = l->is_on() ? "on" : "off";
    state->set("led_state", l->is_on());
    return true;
}

// Device restart: immediate restart
static bool handle_device_restart(
    const edge::EdgeMessage& effect,
    JsonObject result,
    void* user_data
) {
    platform->logging->printf("[app] restart requested\n");
    result["status"] = "restarting";
    platform->timer->delay(100);
    ESP.restart();
    return true;  // unreachable
}

// ---------------------------------------------------------------------------
// Button callback
// ---------------------------------------------------------------------------
static void on_button_press(void* user_data) {
    platform->logging->printf("[app] button pressed\n");

    JsonDocument payload_doc;
    JsonObject payload = payload_doc.to<JsonObject>();
    payload["pin"] = PIN_BUTTON;
    payload["duration_ms"] = 0;

    events->publish("button.pressed", payload);
}

// ---------------------------------------------------------------------------
// Arduino setup
// ---------------------------------------------------------------------------
void setup() {
    // Initialize ESP32 platform
    platform::esp32::init();
    platform = &platform::esp32::platform();
    platform->logging->begin(115200);
    platform->logging->printf("\n[voodoo-edge] starting\n");

    // Generate device ID from MAC address
    uint8_t mac[6];
    platform->wifi->get_mac_address(mac);
    snprintf(device_id, sizeof(device_id), "esp32_%02x%02x%02x",
        mac[3], mac[4], mac[5]);
    platform->logging->printf("[voodoo-edge] device_id: %s\n", device_id);

    // Initialize hardware
    led = new hardware::LedController(*platform->gpio, PIN_LED);
    led->begin();

    button = new hardware::ButtonController(*platform->gpio, *platform->timer, PIN_BUTTON);
    button->begin();
    button->on_press(on_button_press);

    // Initialize core
    capabilities = new edge::CapabilityRegistry();
    capabilities->register_capability("led.control", handle_led_control, led);
    capabilities->register_capability("device.restart", handle_device_restart, nullptr);

    idempotency = new edge::IdempotencyCache();
    state = new edge::DeviceState();
    state->set("led_state", false);
    state->set("firmware_version", FIRMWARE_VERSION);

    // Connect WiFi
    platform->logging->printf("[wifi] connecting to %s\n", WIFI_SSID);
    platform->wifi->begin(WIFI_SSID, WIFI_PASSWORD);

    uint32_t wifi_start = platform->timer->millis();
    while (!platform->wifi->connected()) {
        if (platform->timer->millis() - wifi_start > 15000) {
            platform->logging->printf("[wifi] connection timeout\n");
            break;
        }
        platform->timer->delay(500);
    }

    if (platform->wifi->connected()) {
        platform->logging->printf("[wifi] connected: %s\n", platform->wifi->local_ip());
    }

    // Initialize MQTT transport
    platform::esp32::MqttTransport::Config mqtt_config = {
        .broker_host = MQTT_BROKER_HOST,
        .broker_port = MQTT_BROKER_PORT,
        .device_id = device_id,
        .username = nullptr,
        .password = nullptr,
        .keepalive_s = 60,
        .retry_initial_ms = 1000,
        .retry_max_ms = 30000
    };

    transport = new platform::esp32::MqttTransport(mqtt_config);
    transport->on_effect([](const edge::EdgeMessage& effect, void* ud) {
        static_cast<edge::EffectDispatcher*>(ud)->dispatch(effect);
    });

    dispatcher = new edge::EffectDispatcher(*transport, *capabilities, *idempotency, device_id);
    events = new edge::EventPublisher(*transport, device_id);

    // Connect to broker
    transport->begin();

    // Send HELLO
    const char* cap_names[] = {"led.control", "device.restart"};
    edge::EdgeMessage hello = edge::build_hello(
        device_id, DEVICE_TYPE, FIRMWARE_VERSION, cap_names, 2
    );
    transport->publish(hello);

    // Send initial state
    edge::EdgeMessage state_msg = state->build_sync_message(device_id);
    transport->publish(state_msg);
    state->mark_synced();

    // Send ready event
    events->publish("device.ready");

    platform->logging->printf("[voodoo-edge] ready\n");
}

// ---------------------------------------------------------------------------
// Arduino loop
// ---------------------------------------------------------------------------
void loop() {
    // Process transport messages
    transport->loop();

    // Process button
    button->loop();

    // Heartbeat
    uint32_t now = platform->timer->millis();
    if (now - last_heartbeat >= HEARTBEAT_INTERVAL_MS) {
        last_heartbeat = now;
        uint32_t uptime = now / 1000;
        edge::EdgeMessage hb = edge::build_heartbeat(device_id, state->version(), uptime);
        transport->publish(hb);
    }

    // State sync
    if (state->has_changed() && (now - last_state_sync >= STATE_SYNC_INTERVAL_MS)) {
        last_state_sync = now;
        edge::EdgeMessage state_msg = state->build_sync_message(device_id);
        transport->publish(state_msg);
        state->mark_synced();
    }
}
