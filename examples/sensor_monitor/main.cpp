// main.cpp
// Voodoo Edge — Sensor Monitor Example
//
// Demonstrates temperature sensor reading and periodic state reporting.
// Capabilities:
//   sensor.read  — force an immediate sensor read
//
// Events:
//   temperature.changed — emitted when temperature changes by > 0.5°C

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

// Hardware
#define PIN_NTC 34
#define READ_INTERVAL_MS 5000
#define CHANGE_THRESHOLD 0.5f

// Globals
static edge::Device device;
static hardware::NtcThermistor *temp_sensor = nullptr;
static hardware::SensorReader *analog_sensor = nullptr;
static platform::esp32::MqttTransport *transport = nullptr;
static float last_temp = -999.0f;
static uint32_t last_read = 0;

// ---------------------------------------------------------------------------
// Capability handlers
// ---------------------------------------------------------------------------

static bool handle_sensor_read(
    const edge::EdgeMessage &effect,
    JsonObject result,
    void *user_data)
{
    float temp = temp_sensor->read_celsius();
    float voltage = analog_sensor->read_voltage();

    result["temperature_c"] = temp;
    result["voltage"] = voltage;
    device.set_state("temperature", temp);
    return true;
}

// ---------------------------------------------------------------------------
// Arduino setup & loop
// ---------------------------------------------------------------------------

void setup()
{
    platform::esp32::init();
    auto &platform = platform::esp32::platform();
    platform.logging->begin(115200);
    platform.logging->printf("\n[voodoo-edge] sensor_monitor starting\n");

    // Device ID
    char device_id[32];
    uint8_t mac[6];
    platform.wifi->get_mac_address(mac);
    snprintf(device_id, sizeof(device_id), "sensor_%02x%02x%02x", mac[3], mac[4], mac[5]);

    // Sensors
    analog_sensor = new hardware::SensorReader(*platform.adc, PIN_NTC);
    analog_sensor->begin();
    analog_sensor->set_smoothing(8);

    temp_sensor = new hardware::NtcThermistor(*platform.adc, PIN_NTC);
    temp_sensor->begin();

    // Device
    device.begin(device_id, "sensor-monitor");
    device.expose("sensor.read", handle_sensor_read, nullptr);
    device.set_state("temperature", 0.0f);

    // WiFi
    platform.wifi->begin(WIFI_SSID, WIFI_PASSWORD);
    uint32_t t0 = platform.timer->millis();
    while (!platform.wifi->connected() && platform.timer->millis() - t0 < 15000)
    {
        platform.timer->delay(500);
    }

    // MQTT
    platform::esp32::MqttTransport::Config cfg = {
        .broker_host = MQTT_BROKER_HOST,
        .broker_port = MQTT_BROKER_PORT,
        .device_id = device_id,
        .username = nullptr,
        .password = nullptr,
        .keepalive_s = 60,
        .retry_initial_ms = 1000,
        .retry_max_ms = 30000};
    transport = new platform::esp32::MqttTransport(cfg);
    device.connect(*transport);
}

void loop()
{
    device.update();

    auto &platform = platform::esp32::platform();
    uint32_t now = platform.timer->millis();

    if ((now - last_read) >= READ_INTERVAL_MS)
    {
        last_read = now;

        float temp = temp_sensor->read_celsius();
        device.set_state("temperature", temp);

        // Emit event if temperature changed significantly
        if (last_temp > -900.0f && fabsf(temp - last_temp) > CHANGE_THRESHOLD)
        {
            JsonDocument payload_doc;
            JsonObject payload = payload_doc.to<JsonObject>();
            payload["temperature_c"] = temp;
            payload["previous_c"] = last_temp;
            payload["delta"] = temp - last_temp;
            device.emit("temperature.changed", payload);
        }

        last_temp = temp;
    }
}
