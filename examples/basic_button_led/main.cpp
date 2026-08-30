// main.cpp
// Voodoo Edge — Basic Button + LED Example
//
// Demonstrates the voodoo-edge SDK with:
// - WiFi connection
// - MQTT transport to Voodoo Runtime
// - LED control via "led.control" effect
// - Button press events via "button.pressed"
// - Heartbeat and state sync
//
// Uses the high-level Device facade for minimal boilerplate.

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

// Hardware pins
#define PIN_LED 2
#define PIN_BUTTON 0

// ---------------------------------------------------------------------------
// Global state
// ---------------------------------------------------------------------------
static edge::Device device;
static hardware::LedController *led = nullptr;
static hardware::ButtonController *button = nullptr;
static voodoo::platform::esp32::MqttTransport *transport = nullptr;

// ---------------------------------------------------------------------------
// Capability handlers
// ---------------------------------------------------------------------------

// LED control: {"action": "on"} | {"action": "off"} | {"action": "toggle"}
static bool handle_led_control(
    const edge::EdgeMessage &effect,
    JsonObject result,
    void *user_data)
{
    auto *l = static_cast<hardware::LedController *>(user_data);
    JsonObjectConst payload = effect.payload();
    const char *action = payload["action"] | "";

    if (strcmp(action, "on") == 0)
    {
        l->on();
    }
    else if (strcmp(action, "off") == 0)
    {
        l->off();
    }
    else if (strcmp(action, "toggle") == 0)
    {
        l->toggle();
    }
    else
    {
        result["error"] = "unknown action";
        return false;
    }

    result["led_state"] = l->is_on() ? "on" : "off";
    device.set_state("led_state", l->is_on());
    return true;
}

// ---------------------------------------------------------------------------
// Button callback
// ---------------------------------------------------------------------------
static void on_button_press(void *user_data)
{
    auto &plat = voodoo::hal::get_platform();
    plat.logging->printf("[app] button pressed\n");

    JsonDocument payload_doc;
    JsonObject payload = payload_doc.to<JsonObject>();
    payload["pin"] = PIN_BUTTON;

    device.emit("button.pressed", payload);
}

// ---------------------------------------------------------------------------
// Arduino setup
// ---------------------------------------------------------------------------
void setup()
{
    // Initialize ESP32 platform
    voodoo::platform::esp32::init();
    auto &plat = voodoo::platform::esp32::platform();
    plat.logging->begin(115200);
    plat.logging->printf("\n[voodoo-edge] basic_button_led starting\n");

    // Generate device ID from MAC address
    char device_id[32];
    uint8_t mac[6];
    plat.wifi->get_mac_address(mac);
    snprintf(device_id, sizeof(device_id), "esp32_%02x%02x%02x",
             mac[3], mac[4], mac[5]);

    // Initialize hardware
    led = new hardware::LedController(*plat.gpio, PIN_LED);
    led->begin();

    button = new hardware::ButtonController(*plat.gpio, *plat.timer, PIN_BUTTON);
    button->begin();
    button->on_press(on_button_press);

    // Initialize device with facade
    device.begin(device_id, "esp32");
    device.expose("led.control", handle_led_control, led);
    device.set_state("led_state", false);

    // Connect WiFi
    plat.logging->printf("[wifi] connecting to %s\n", WIFI_SSID);
    plat.wifi->begin(WIFI_SSID, WIFI_PASSWORD);

    uint32_t wifi_start = plat.timer->millis();
    while (!plat.wifi->connected())
    {
        if (plat.timer->millis() - wifi_start > 15000)
        {
            plat.logging->printf("[wifi] connection timeout\n");
            break;
        }
        plat.timer->delay(500);
    }

    if (plat.wifi->connected())
    {
        plat.logging->printf("[wifi] connected: %s\n", plat.wifi->local_ip());
    }

    // Initialize MQTT transport and connect
    voodoo::platform::esp32::MqttTransport::Config mqtt_config = {
        .broker_host = MQTT_BROKER_HOST,
        .broker_port = MQTT_BROKER_PORT,
        .device_id = device_id,
        .username = nullptr,
        .password = nullptr,
        .keepalive_s = 60,
        .retry_initial_ms = 1000,
        .retry_max_ms = 30000};

    transport = new voodoo::platform::esp32::MqttTransport(mqtt_config);
    device.connect(*transport);

    plat.logging->printf("[app] ready\n");
}

// ---------------------------------------------------------------------------
// Arduino loop
// ---------------------------------------------------------------------------
void loop()
{
    // Device facade handles: heartbeat, state sync, effect dispatch, reconnection
    device.update();

    // Hardware polling
    button->loop();
}
