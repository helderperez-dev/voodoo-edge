// main.cpp
// Voodoo Edge — Motor Control Example
//
// Demonstrates motor and servo control via Voodoo Runtime effects.
// Capabilities:
//   motor.speed  — set motor speed (-1.0 to 1.0)
//   motor.stop   — stop the motor
//   servo.angle  — set servo angle (0-180)

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
#define PIN_MOTOR_DIR 16
#define PIN_MOTOR_PWM 17
#define PIN_SERVO 18
#define PWM_CH_MOTOR 0
#define PWM_CH_SERVO 1

// Globals
static edge::Device device;
static hardware::MotorController *motor = nullptr;
static hardware::ServoController *servo = nullptr;
static voodoo::platform::esp32::MqttTransport *transport = nullptr;

// ---------------------------------------------------------------------------
// Capability handlers
// ---------------------------------------------------------------------------

static bool handle_motor_speed(
    const edge::EdgeMessage &effect,
    JsonObject result,
    void *user_data)
{
    auto *m = static_cast<hardware::MotorController *>(user_data);
    JsonObjectConst payload = effect.payload();
    float speed = payload["speed"] | 0.0f;

    m->set_speed(speed);
    result["speed"] = m->current_speed();
    device.set_state("motor_speed", m->current_speed());
    return true;
}

static bool handle_motor_stop(
    const edge::EdgeMessage &effect,
    JsonObject result,
    void *user_data)
{
    auto *m = static_cast<hardware::MotorController *>(user_data);
    m->stop();
    result["speed"] = 0.0f;
    device.set_state("motor_speed", 0.0f);
    return true;
}

static bool handle_servo_angle(
    const edge::EdgeMessage &effect,
    JsonObject result,
    void *user_data)
{
    auto *s = static_cast<hardware::ServoController *>(user_data);
    JsonObjectConst payload = effect.payload();
    uint16_t angle = payload["angle"] | 90;

    s->set_angle(angle);
    result["angle"] = s->current_angle();
    device.set_state("servo_angle", static_cast<int>(s->current_angle()));
    return true;
}

// ---------------------------------------------------------------------------
// Arduino setup & loop
// ---------------------------------------------------------------------------

void setup()
{
    voodoo::platform::esp32::init();
    auto &plat = voodoo::platform::esp32::platform();
    plat.logging->begin(115200);
    plat.logging->printf("\n[voodoo-edge] motor_control starting\n");

    // Device ID from MAC
    char device_id[32];
    uint8_t mac[6];
    plat.wifi->get_mac_address(mac);
    snprintf(device_id, sizeof(device_id), "motor_%02x%02x%02x", mac[3], mac[4], mac[5]);

    // Hardware
    motor = new hardware::MotorController(*plat.gpio, *plat.pwm,
                                          PIN_MOTOR_DIR, PWM_CH_MOTOR);
    motor->begin();

    servo = new hardware::ServoController(*plat.pwm, PWM_CH_SERVO);
    servo->begin(PIN_SERVO);

    // Device
    device.begin(device_id, "motor-controller");
    device.expose("motor.speed", handle_motor_speed, motor);
    device.expose("motor.stop", handle_motor_stop, motor);
    device.expose("servo.angle", handle_servo_angle, servo);
    device.set_state("motor_speed", 0.0f);
    device.set_state("servo_angle", 90);

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
}

void loop()
{
    device.update();
}
