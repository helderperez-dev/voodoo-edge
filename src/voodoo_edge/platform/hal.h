// hal.h
// Voodoo Edge — Hardware Abstraction Layer interfaces
//
// Platform implementations must provide concrete implementations of these
// interfaces. This allows the core protocol layer to remain platform-independent.

#ifndef VODOO_HAL_H
#define VODOO_HAL_H

#include <cstdint>
#include <cstddef>

namespace voodoo {
namespace hal {

// ---------------------------------------------------------------------------
// Gpio — Digital I/O abstraction
// ---------------------------------------------------------------------------
class Gpio {
public:
    virtual ~Gpio() = default;

    enum class Mode : uint8_t {
        INPUT = 0,
        OUTPUT = 1,
        INPUT_PULLUP = 2,
        INPUT_PULLDOWN = 3
    };

    enum class Level : uint8_t {
        LOW = 0,
        HIGH = 1
    };

    virtual void pinMode(uint8_t pin, Mode mode) = 0;
    virtual void digitalWrite(uint8_t pin, Level level) = 0;
    virtual Level digitalRead(uint8_t pin) = 0;
};

// ---------------------------------------------------------------------------
// Wifi — Network abstraction
// ---------------------------------------------------------------------------
class Wifi {
public:
    virtual ~Wifi() = default;

    enum class Status : uint8_t {
        IDLE = 0,
        CONNECTING = 1,
        CONNECTED = 2,
        DISCONNECTED = 3,
        FAILED = 4
    };

    virtual bool begin(const char* ssid, const char* password) = 0;
    virtual void disconnect() = 0;
    virtual Status status() = 0;
    virtual bool connected() = 0;
    virtual const char* local_ip() = 0;
    virtual int8_t rssi() = 0;
    virtual void get_mac_address(uint8_t mac[6]) = 0;
};

// ---------------------------------------------------------------------------
// Mqtt — MQTT client abstraction
// ---------------------------------------------------------------------------
class Mqtt {
public:
    virtual ~Mqtt() = default;

    typedef void (*MessageCallback)(const char* topic, const uint8_t* payload, size_t length, void* user_data);

    virtual bool begin(const char* host, uint16_t port, const char* username = nullptr, const char* password = nullptr) = 0;
    virtual void end() = 0;
    virtual bool connected() = 0;
    virtual bool connect(const char* client_id) = 0;
    virtual void disconnect() = 0;
    virtual bool publish(const char* topic, const char* payload, size_t length, uint8_t qos = 0) = 0;
    virtual bool subscribe(const char* topic, uint8_t qos = 0) = 0;
    virtual void on_message(MessageCallback callback, void* user_data = nullptr) = 0;
    virtual void loop() = 0;
};

// ---------------------------------------------------------------------------
// Http — HTTP client abstraction
// ---------------------------------------------------------------------------
class Http {
public:
    virtual ~Http() = default;

    virtual bool begin(const char* url) = 0;
    virtual void end() = 0;
    virtual void set_header(const char* name, const char* value) = 0;
    virtual void set_timeout(uint32_t timeout_ms) = 0;
    virtual int post(const char* payload, size_t length) = 0;
    virtual int get() = 0;
    virtual const char* response_body() = 0;
    virtual int status_code() = 0;
};

// ---------------------------------------------------------------------------
// Random — Random number generation
// ---------------------------------------------------------------------------
class Random {
public:
    virtual ~Random() = default;

    virtual uint32_t generate() = 0;
    virtual void fill(uint8_t* buffer, size_t length) = 0;
};

// ---------------------------------------------------------------------------
// Timer — Millisecond timer
// ---------------------------------------------------------------------------
class Timer {
public:
    virtual ~Timer() = default;

    virtual uint32_t millis() = 0;
    virtual void delay(uint32_t ms) = 0;
};

// ---------------------------------------------------------------------------
// Logging — Serial/logging output
// ---------------------------------------------------------------------------
class Logging {
public:
    virtual ~Logging() = default;

    virtual void begin(uint32_t baud_rate) = 0;
    virtual void printf(const char* format, ...) = 0;
    virtual void println(const char* str) = 0;
};

// ---------------------------------------------------------------------------
// Platform — Aggregates all HAL interfaces for a platform
// ---------------------------------------------------------------------------
struct Platform {
    Gpio* gpio;
    Wifi* wifi;
    Mqtt* mqtt;
    Http* http;
    Random* random;
    Timer* timer;
    Logging* logging;
};

// Get the current platform (set by platform initialization)
Platform& get_platform();

// Set the current platform (called by platform implementations)
void set_platform(Platform* platform);

} // namespace hal
} // namespace voodoo

#endif // VODOO_HAL_H
