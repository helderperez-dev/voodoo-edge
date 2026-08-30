// esp32_platform.cpp
// Voodoo Edge — ESP32 platform implementation

#include "voodoo_edge/platform/esp32/esp32_platform.h"
#include <Arduino.h>
#include <WiFi.h>
#include <cstring>
#include <cstdarg>

namespace voodoo {
namespace platform {
namespace esp32 {

// ---------------------------------------------------------------------------
// ESP32 GPIO
// ---------------------------------------------------------------------------
class Esp32Gpio : public hal::Gpio {
public:
    void pinMode(uint8_t pin, Mode mode) override {
        ::pinMode(pin, static_cast<uint8_t>(mode));
    }

    void digitalWrite(uint8_t pin, Level level) override {
        ::digitalWrite(pin, static_cast<uint8_t>(level));
    }

    Level digitalRead(uint8_t pin) override {
        return static_cast<Level>(::digitalRead(pin));
    }
};

// ---------------------------------------------------------------------------
// ESP32 WiFi
// ---------------------------------------------------------------------------
class Esp32Wifi : public hal::Wifi {
public:
    bool begin(const char* ssid, const char* password) override {
        WiFi.mode(WIFI_STA);
        WiFi.begin(ssid, password);
        return true;
    }

    void disconnect() override {
        WiFi.disconnect();
    }

    Status status() override {
        wl_status_t s = WiFi.status();
        switch (s) {
            case WL_IDLE_STATUS: return Status::IDLE;
            case WL_CONNECTED: return Status::CONNECTED;
            case WL_CONNECT_FAILED: return Status::FAILED;
            case WL_DISCONNECTED: return Status::DISCONNECTED;
            default: return Status::CONNECTING;
        }
    }

    bool connected() override {
        return WiFi.status() == WL_CONNECTED;
    }

    const char* local_ip() override {
        static char ip_str[16];
        IPAddress ip = WiFi.localIP();
        snprintf(ip_str, sizeof(ip_str), "%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);
        return ip_str;
    }

    int8_t rssi() override {
        return WiFi.RSSI();
    }

    void get_mac_address(uint8_t mac[6]) override {
        WiFi.macAddress(mac);
    }
};

// ---------------------------------------------------------------------------
// ESP32 Random
// ---------------------------------------------------------------------------
class Esp32Random : public hal::Random {
public:
    uint32_t generate() override {
        return esp_random();
    }

    void fill(uint8_t* buffer, size_t length) override {
        for (size_t i = 0; i < length; i += 4) {
            uint32_t r = esp_random();
            size_t remaining = length - i;
            size_t to_copy = remaining < 4 ? remaining : 4;
            memcpy(buffer + i, &r, to_copy);
        }
    }
};

// ---------------------------------------------------------------------------
// ESP32 Timer
// ---------------------------------------------------------------------------
class Esp32Timer : public hal::Timer {
public:
    uint32_t millis() override {
        return ::millis();
    }

    void delay(uint32_t ms) override {
        ::delay(ms);
    }
};

// ---------------------------------------------------------------------------
// ESP32 Logging
// ---------------------------------------------------------------------------
class Esp32Logging : public hal::Logging {
public:
    void begin(uint32_t baud_rate) override {
        Serial.begin(baud_rate);
    }

    void printf(const char* format, ...) override {
        va_list args;
        va_start(args, format);
        char buf[256];
        vsnprintf(buf, sizeof(buf), format, args);
        va_end(args);
        Serial.print(buf);
    }

    void println(const char* str) override {
        Serial.println(str);
    }
};

// ---------------------------------------------------------------------------
// Platform instances
// ---------------------------------------------------------------------------
static Esp32Gpio _gpio;
static Esp32Wifi _wifi;
static Esp32Random _random;
static Esp32Timer _timer;
static Esp32Logging _logging;

static hal::Platform _esp32_platform = {
    &_gpio,
    &_wifi,
    nullptr,  // mqtt — set by transport
    nullptr,  // http — set by transport
    &_random,
    &_timer,
    &_logging
};

void init() {
    hal::set_platform(&_esp32_platform);
}

hal::Platform& platform() {
    return _esp32_platform;
}

} // namespace esp32
} // namespace platform
} // namespace voodoo
