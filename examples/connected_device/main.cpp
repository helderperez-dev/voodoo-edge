// main.cpp
// Voodoo Edge — Connected Device Example
//
// A comprehensive example showing multiple capabilities on one device:
//   - LED control (on/off/toggle)
//   - Relay control (on/off)
//   - Button events
//   - Temperature monitoring
//   - Buzzer alerts
//   - Device restart
//
// This demonstrates how a real product (e.g. vacuum former controller)
// would use Voodoo Edge.

#include <Arduino.h>
#include <voodoo_edge.h>

using namespace voodoo;

// Configuration
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
#define PIN_RELAY 4
#define PIN_NTC 34
#define PIN_BUZZER 25
#define PWM_CH_BUZZER 2

// Globals
static edge::Device device;
static hardware::LedController *led = nullptr;
static hardware::ButtonController *button = nullptr;
static hardware::RelayController *relay = nullptr;
static hardware::NtcThermistor *temp_sensor = nullptr;
static hardware::BuzzerController *buzzer = nullptr;
static voodoo::platform::esp32::MqttTransport *transport = nullptr;

// ---------------------------------------------------------------------------
// Capability handlers
// ---------------------------------------------------------------------------

static bool handle_led(const edge::EdgeMessage &effect, JsonObject result, void *ud)
{
    auto *l = static_cast<hardware::LedController *>(ud);
    JsonObjectConst p = effect.payload();
    const char *action = p["action"] | "";
    if (strcmp(action, "on") == 0)
        l->on();
    else if (strcmp(action, "off") == 0)
        l->off();
    else if (strcmp(action, "toggle") == 0)
        l->toggle();
    else
    {
        result["error"] = "unknown action";
        return false;
    }
    result["state"] = l->is_on();
    device.set_state("led", l->is_on());
    return true;
}

static bool handle_relay(const edge::EdgeMessage &effect, JsonObject result, void *ud)
{
    auto *r = static_cast<hardware::RelayController *>(ud);
    JsonObjectConst p = effect.payload();
    const char *action = p["action"] | "";
    if (strcmp(action, "on") == 0)
        r->on();
    else if (strcmp(action, "off") == 0)
        r->off();
    else if (strcmp(action, "toggle") == 0)
        r->toggle();
    else
    {
        result["error"] = "unknown action";
        return false;
    }
    result["state"] = r->is_on();
    device.set_state("relay", r->is_on());
    return true;
}

static bool handle_buzzer(const edge::EdgeMessage &effect, JsonObject result, void *ud)
{
    auto *b = static_cast<hardware::BuzzerController *>(ud);
    JsonObjectConst p = effect.payload();
    uint32_t freq = p["frequency"] | 1000;
    uint32_t duration = p["duration_ms"] | 200;
    b->beep(freq, duration);
    result["playing"] = true;
    return true;
}

static bool handle_restart(const edge::EdgeMessage &effect, JsonObject result, void *ud)
{
    result["status"] = "restarting";
    auto &plat = voodoo::hal::get_platform();
    plat.timer->delay(100);
    ESP.restart();
    return true;
}

// ---------------------------------------------------------------------------
// Button callback
// ---------------------------------------------------------------------------

static void on_button_press(void *)
{
    JsonDocument doc;
    JsonObject p = doc.to<JsonObject>();
    p["pin"] = PIN_BUTTON;
    device.emit("button.pressed", p);
}

// ---------------------------------------------------------------------------
// Arduino setup & loop
// ---------------------------------------------------------------------------

void setup()
{
    voodoo::platform::esp32::init();
    auto &plat = voodoo::platform::esp32::platform();
    plat.logging->begin(115200);
    plat.logging->printf("\n[voodoo-edge] connected_device starting\n");

    // Device ID
    char device_id[32];
    uint8_t mac[6];
    plat.wifi->get_mac_address(mac);
    snprintf(device_id, sizeof(device_id), "device_%02x%02x%02x", mac[3], mac[4], mac[5]);

    // Hardware
    led = new hardware::LedController(*plat.gpio, PIN_LED);
    led->begin();

    button = new hardware::ButtonController(*plat.gpio, *plat.timer, PIN_BUTTON);
    button->begin();
    button->on_press(on_button_press);

    relay = new hardware::RelayController(*plat.gpio, PIN_RELAY);
    relay->begin();

    temp_sensor = new hardware::NtcThermistor(*plat.adc, PIN_NTC);
    temp_sensor->begin();

    buzzer = new hardware::BuzzerController(*plat.pwm, PWM_CH_BUZZER);
    buzzer->begin(PIN_BUZZER);

    // Device
    device.begin(device_id, "connected-device");
    device.expose("led.control", handle_led, led);
    device.expose("relay.control", handle_relay, relay);
    device.expose("buzzer.alert", handle_buzzer, buzzer);
    device.expose("device.restart", handle_restart, nullptr);

    device.set_state("led", false);
    device.set_state("relay", false);
    device.set_state("temperature", 0.0f);

    // WiFi
    plat.wifi->begin(WIFI_SSID, WIFI_PASSWORD);
    uint32_t t0 = plat.timer->millis();
    while (!plat.wifi->connected() && plat.timer->millis() - t0 < 15000)
    {
        plat.timer->delay(500);
    }

    // MQTT
    voodoo::platform::esp32::MqttTransport::Config cfg = {
        .broker_host = MQTT_BROKER_HOST,
        .broker_port = MQTT_BROKER_PORT,
        .device_id = device_id,
        .username = nullptr,
        .password = nullptr,
        .keepalive_s = 60,
        .retry_initial_ms = 1000,
        .retry_max_ms = 30000};
    transport = new voodoo::platform::esp32::MqttTransport(cfg);
    device.connect(*transport);

    // Startup beep
    buzzer->beep(2000, 100);
}

void loop()
{
    device.update();
    button->loop();

    // Periodic temperature reading
    static uint32_t last_temp = 0;
    auto &plat = voodoo::platform::esp32::platform();
    uint32_t now = plat.timer->millis();
    if ((now - last_temp) >= 10000)
    {
        last_temp = now;
        float temp = temp_sensor->read_celsius();
        device.set_state("temperature", temp);
    }
}
