// esp32_platform.cpp
// Voodoo Edge — ESP32 platform implementation

#include "voodoo_edge/platform/esp32/esp32_platform.h"
#include <Arduino.h>
#include <WiFi.h>
#include <cstring>
#include <cstdarg>

// ESP32 LEDC for PWM
#if defined(ESP32)
#include <driver/ledc.h>
#endif

namespace voodoo
{
    namespace platform
    {
        namespace esp32
        {

            // ---------------------------------------------------------------------------
            // ESP32 GPIO
            // ---------------------------------------------------------------------------
            class Esp32Gpio : public hal::Gpio
            {
            public:
                void pinMode(uint8_t pin, Mode mode) override
                {
                    ::pinMode(pin, static_cast<uint8_t>(mode));
                }

                void digitalWrite(uint8_t pin, Level level) override
                {
                    ::digitalWrite(pin, static_cast<uint8_t>(level));
                }

                Level digitalRead(uint8_t pin) override
                {
                    return static_cast<Level>(::digitalRead(pin));
                }
            };

            // ---------------------------------------------------------------------------
            // ESP32 WiFi
            // ---------------------------------------------------------------------------
            class Esp32Wifi : public hal::Wifi
            {
            public:
                bool begin(const char *ssid, const char *password) override
                {
                    WiFi.mode(WIFI_STA);
                    WiFi.begin(ssid, password);
                    return true;
                }

                void disconnect() override
                {
                    WiFi.disconnect();
                }

                Status status() override
                {
                    wl_status_t s = WiFi.status();
                    switch (s)
                    {
                    case WL_IDLE_STATUS:
                        return Status::IDLE;
                    case WL_CONNECTED:
                        return Status::CONNECTED;
                    case WL_CONNECT_FAILED:
                        return Status::FAILED;
                    case WL_DISCONNECTED:
                        return Status::DISCONNECTED;
                    default:
                        return Status::CONNECTING;
                    }
                }

                bool connected() override
                {
                    return WiFi.status() == WL_CONNECTED;
                }

                const char *local_ip() override
                {
                    static char ip_str[16];
                    IPAddress ip = WiFi.localIP();
                    snprintf(ip_str, sizeof(ip_str), "%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);
                    return ip_str;
                }

                int8_t rssi() override
                {
                    return WiFi.RSSI();
                }

                void get_mac_address(uint8_t mac[6]) override
                {
                    WiFi.macAddress(mac);
                }
            };

            // ---------------------------------------------------------------------------
            // ESP32 PWM (LEDC)
            // ---------------------------------------------------------------------------
            class Esp32Pwm : public hal::Pwm
            {
            public:
                bool setup(uint8_t channel, uint32_t freq_hz, uint8_t resolution_bits) override
                {
#if defined(ESP32)
                    ledc_timer_config_t timer_conf = {};
                    timer_conf.speed_mode = LEDC_LOW_SPEED_MODE;
                    timer_conf.timer_num = static_cast<ledc_timer_t>(channel % 4);
                    timer_conf.duty_resolution = static_cast<ledc_timer_bit_t>(resolution_bits);
                    timer_conf.freq_hz = freq_hz;
                    timer_conf.clk_cfg = LEDC_AUTO_CLK;
                    return ledc_timer_config(&timer_conf) == ESP_OK;
#else
                    return false;
#endif
                }

                void attach(uint8_t pin, uint8_t channel) override
                {
#if defined(ESP32)
                    ledc_channel_config_t chan_conf = {};
                    chan_conf.speed_mode = LEDC_LOW_SPEED_MODE;
                    chan_conf.channel = static_cast<ledc_channel_t>(channel % 8);
                    chan_conf.timer_sel = static_cast<ledc_timer_t>(channel % 4);
                    chan_conf.gpio_num = pin;
                    chan_conf.duty = 0;
                    chan_conf.hpoint = 0;
                    ledc_channel_config(&chan_conf);
#endif
                }

                void write(uint8_t channel, uint32_t duty) override
                {
#if defined(ESP32)
                    ledc_set_duty(LEDC_LOW_SPEED_MODE, static_cast<ledc_channel_t>(channel % 8), duty);
                    ledc_update_duty(LEDC_LOW_SPEED_MODE, static_cast<ledc_channel_t>(channel % 8));
#endif
                }

                void write_float(uint8_t channel, float duty) override
                {
#if defined(ESP32)
                    // Assume 8-bit resolution by default for float API
                    uint32_t max_duty = 255;
                    uint32_t d = static_cast<uint32_t>(duty * max_duty);
                    if (d > max_duty)
                        d = max_duty;
                    write(channel, d);
#endif
                }

                void stop(uint8_t channel) override
                {
#if defined(ESP32)
                    ledc_stop(LEDC_LOW_SPEED_MODE, static_cast<ledc_channel_t>(channel % 8), 0);
#endif
                }
            };

            // ---------------------------------------------------------------------------
            // ESP32 ADC
            // ---------------------------------------------------------------------------
            class Esp32Adc : public hal::Adc
            {
            public:
                void setup(uint8_t pin, Attenuation atten) override
                {
#if defined(ESP32)
                    analogSetAttenuation(static_cast<adc_attenuation_t>(atten));
                    analogReadResolution(12);
#endif
                }

                uint16_t read(uint8_t pin) override
                {
                    return static_cast<uint16_t>(analogRead(pin));
                }

                float read_voltage(uint8_t pin) override
                {
                    uint16_t raw = read(pin);
                    // ESP32 ADC is 12-bit (0-4095), reference ~3.3V
                    return (raw / 4095.0f) * 3.3f;
                }
            };

            // ---------------------------------------------------------------------------
            // ESP32 I2C (Wire)
            // ---------------------------------------------------------------------------
            class Esp32I2c : public hal::I2c
            {
            public:
                bool begin(int sda_pin, int scl_pin, uint32_t freq_hz) override
                {
                    Wire.begin(sda_pin, scl_pin, freq_hz);
                    return true;
                }

                void end() override
                {
                    Wire.end();
                }

                bool write(uint8_t addr, const uint8_t *data, size_t len) override
                {
                    Wire.beginTransmission(addr);
                    Wire.write(data, len);
                    return Wire.endTransmission() == 0;
                }

                bool read(uint8_t addr, uint8_t *buffer, size_t len) override
                {
                    Wire.requestFrom(addr, static_cast<uint8_t>(len));
                    size_t i = 0;
                    while (Wire.available() && i < len)
                    {
                        buffer[i++] = Wire.read();
                    }
                    return i == len;
                }

                bool write_read(uint8_t addr, const uint8_t *write_data, size_t write_len,
                                uint8_t *read_buffer, size_t read_len) override
                {
                    Wire.beginTransmission(addr);
                    Wire.write(write_data, write_len);
                    if (Wire.endTransmission(false) != 0)
                        return false;

                    Wire.requestFrom(addr, static_cast<uint8_t>(read_len));
                    size_t i = 0;
                    while (Wire.available() && i < read_len)
                    {
                        read_buffer[i++] = Wire.read();
                    }
                    return i == read_len;
                }
            };

            // ---------------------------------------------------------------------------
            // ESP32 SPI
            // ---------------------------------------------------------------------------
            class Esp32Spi : public hal::Spi
            {
            public:
                bool begin(int sck_pin, int miso_pin, int mosi_pin, int cs_pin) override
                {
                    _spi.begin(sck_pin, miso_pin, mosi_pin, cs_pin);
                    _cs_pin = cs_pin;
                    return true;
                }

                void end() override
                {
                    _spi.end();
                }

                bool transfer(const uint8_t *tx, uint8_t *rx, size_t len) override
                {
                    _spi.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE0));
                    for (size_t i = 0; i < len; i++)
                    {
                        uint8_t r = _spi.transfer(tx ? tx[i] : 0);
                        if (rx)
                            rx[i] = r;
                    }
                    _spi.endTransaction();
                    return true;
                }

                void select(int cs_pin) override
                {
                    ::digitalWrite(cs_pin, LOW);
                }

                void deselect(int cs_pin) override
                {
                    ::digitalWrite(cs_pin, HIGH);
                }

            private:
                SPIClass _spi = SPI;
                int _cs_pin = -1;
            };

            // ---------------------------------------------------------------------------
            // ESP32 Random
            // ---------------------------------------------------------------------------
            class Esp32Random : public hal::Random
            {
            public:
                uint32_t generate() override
                {
                    return esp_random();
                }

                void fill(uint8_t *buffer, size_t length) override
                {
                    for (size_t i = 0; i < length; i += 4)
                    {
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
            class Esp32Timer : public hal::Timer
            {
            public:
                uint32_t millis() override
                {
                    return ::millis();
                }

                void delay(uint32_t ms) override
                {
                    ::delay(ms);
                }
            };

            // ---------------------------------------------------------------------------
            // ESP32 Logging
            // ---------------------------------------------------------------------------
            class Esp32Logging : public hal::Logging
            {
            public:
                void begin(uint32_t baud_rate) override
                {
                    Serial.begin(baud_rate);
                }

                void printf(const char *format, ...) override
                {
                    va_list args;
                    va_start(args, format);
                    char buf[256];
                    vsnprintf(buf, sizeof(buf), format, args);
                    va_end(args);
                    Serial.print(buf);
                }

                void println(const char *str) override
                {
                    Serial.println(str);
                }
            };

            // ---------------------------------------------------------------------------
            // Platform instances
            // ---------------------------------------------------------------------------
            static Esp32Gpio _gpio;
            static Esp32Pwm _pwm;
            static Esp32Adc _adc;
            static Esp32I2c _i2c;
            static Esp32Spi _spi;
            static Esp32Wifi _wifi;
            static Esp32Random _random;
            static Esp32Timer _timer;
            static Esp32Logging _logging;

            static hal::Platform _esp32_platform = {
                &_gpio,
                &_pwm,
                &_adc,
                &_i2c,
                &_spi,
                &_wifi,
                nullptr, // mqtt — set by transport
                nullptr, // http — set by transport
                &_random,
                &_timer,
                &_logging};

            void init()
            {
                hal::set_platform(&_esp32_platform);
            }

            hal::Platform &platform()
            {
                return _esp32_platform;
            }

        } // namespace esp32
    } // namespace platform
} // namespace voodoo
