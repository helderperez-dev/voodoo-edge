// hal.h
// Voodoo Edge — Hardware Abstraction Layer interfaces
//
// Platform implementations must provide concrete implementations of these
// interfaces. This allows the core protocol layer to remain platform-independent.

#ifndef VODOO_HAL_H
#define VODOO_HAL_H

#include <cstdint>
#include <cstddef>

namespace voodoo
{
    namespace hal
    {

        // ---------------------------------------------------------------------------
        // Gpio — Digital I/O abstraction
        // ---------------------------------------------------------------------------
        class Gpio
        {
        public:
            virtual ~Gpio() = default;

            enum class Mode : uint8_t
            {
                Input = 0,
                Output = 1,
                InputPullUp = 2,
                InputPullDown = 3
            };

            enum class Level : uint8_t
            {
                Low = 0,
                High = 1
            };

            virtual void pinMode(uint8_t pin, Mode mode) = 0;
            virtual void digitalWrite(uint8_t pin, Level level) = 0;
            virtual Level digitalRead(uint8_t pin) = 0;
        };

        // ---------------------------------------------------------------------------
        // Pwm — PWM output abstraction
        // ---------------------------------------------------------------------------
        class Pwm
        {
        public:
            virtual ~Pwm() = default;

            // Configure a PWM channel with frequency and resolution
            virtual bool setup(uint8_t channel, uint32_t freq_hz, uint8_t resolution_bits) = 0;

            // Attach a pin to a configured channel
            virtual void attach(uint8_t pin, uint8_t channel) = 0;

            // Set duty cycle (0 .. 2^resolution_bits - 1)
            virtual void write(uint8_t channel, uint32_t duty) = 0;

            // Set duty as a float fraction 0.0 .. 1.0
            virtual void write_float(uint8_t channel, float duty) = 0;

            // Stop PWM on a channel
            virtual void stop(uint8_t channel) = 0;
        };

        // ---------------------------------------------------------------------------
        // Adc — Analog-to-digital converter abstraction
        // ---------------------------------------------------------------------------
        class Adc
        {
        public:
            virtual ~Adc() = default;

            enum class Attenuation : uint8_t
            {
                DB_0 = 0,   // 0-1.1V
                DB_2_5 = 1, // 0-1.5V
                DB_6 = 2,   // 0-2.2V
                DB_11 = 3   // 0-3.3V
            };

            virtual void setup(uint8_t pin, Attenuation atten = Attenuation::DB_11) = 0;
            virtual uint16_t read(uint8_t pin) = 0;      // raw ADC value
            virtual float read_voltage(uint8_t pin) = 0; // converted voltage
        };

        // ---------------------------------------------------------------------------
        // I2c — I2C bus abstraction
        // ---------------------------------------------------------------------------
        class I2c
        {
        public:
            virtual ~I2c() = default;

            virtual bool begin(int sda_pin, int scl_pin, uint32_t freq_hz = 100000) = 0;
            virtual void end() = 0;

            // Write bytes to a device at address
            virtual bool write(uint8_t addr, const uint8_t *data, size_t len) = 0;

            // Read bytes from a device at address
            virtual bool read(uint8_t addr, uint8_t *buffer, size_t len) = 0;

            // Write then read (common register pattern)
            virtual bool write_read(uint8_t addr, const uint8_t *write_data, size_t write_len,
                                    uint8_t *read_buffer, size_t read_len) = 0;
        };

        // ---------------------------------------------------------------------------
        // Spi — SPI bus abstraction
        // ---------------------------------------------------------------------------
        class Spi
        {
        public:
            virtual ~Spi() = default;

            virtual bool begin(int sck_pin, int miso_pin, int mosi_pin, int cs_pin = -1) = 0;
            virtual void end() = 0;

            // Transfer bytes (full-duplex). tx and rx may alias.
            virtual bool transfer(const uint8_t *tx, uint8_t *rx, size_t len) = 0;

            // Chip-select control
            virtual void select(int cs_pin) = 0;
            virtual void deselect(int cs_pin) = 0;
        };

        // ---------------------------------------------------------------------------
        // Wifi — Network abstraction
        // ---------------------------------------------------------------------------
        class Wifi
        {
        public:
            virtual ~Wifi() = default;

            enum class Status : uint8_t
            {
                IDLE = 0,
                CONNECTING = 1,
                CONNECTED = 2,
                DISCONNECTED = 3,
                FAILED = 4
            };

            virtual bool begin(const char *ssid, const char *password) = 0;
            virtual void disconnect() = 0;
            virtual Status status() = 0;
            virtual bool connected() = 0;
            virtual const char *local_ip() = 0;
            virtual int8_t rssi() = 0;
            virtual void get_mac_address(uint8_t mac[6]) = 0;
        };

        // ---------------------------------------------------------------------------
        // Mqtt — MQTT client abstraction
        // ---------------------------------------------------------------------------
        class Mqtt
        {
        public:
            virtual ~Mqtt() = default;

            typedef void (*MessageCallback)(const char *topic, const uint8_t *payload, size_t length, void *user_data);

            virtual bool begin(const char *host, uint16_t port, const char *username = nullptr, const char *password = nullptr) = 0;
            virtual void end() = 0;
            virtual bool connected() = 0;
            virtual bool connect(const char *client_id) = 0;
            virtual void disconnect() = 0;
            virtual bool publish(const char *topic, const char *payload, size_t length, uint8_t qos = 0) = 0;
            virtual bool subscribe(const char *topic, uint8_t qos = 0) = 0;
            virtual void on_message(MessageCallback callback, void *user_data = nullptr) = 0;
            virtual void loop() = 0;
        };

        // ---------------------------------------------------------------------------
        // Http — HTTP client abstraction
        // ---------------------------------------------------------------------------
        class Http
        {
        public:
            virtual ~Http() = default;

            virtual bool begin(const char *url) = 0;
            virtual void end() = 0;
            virtual void set_header(const char *name, const char *value) = 0;
            virtual void set_timeout(uint32_t timeout_ms) = 0;
            virtual int post(const char *payload, size_t length) = 0;
            virtual int get() = 0;
            virtual const char *response_body() = 0;
            virtual int status_code() = 0;
        };

        // ---------------------------------------------------------------------------
        // Random — Random number generation
        // ---------------------------------------------------------------------------
        class Random
        {
        public:
            virtual ~Random() = default;

            virtual uint32_t generate() = 0;
            virtual void fill(uint8_t *buffer, size_t length) = 0;
        };

        // ---------------------------------------------------------------------------
        // Timer — Millisecond timer
        // ---------------------------------------------------------------------------
        class Timer
        {
        public:
            virtual ~Timer() = default;

            virtual uint32_t millis() = 0;
            virtual void delay(uint32_t ms) = 0;
        };

        // ---------------------------------------------------------------------------
        // Logging — Serial/logging output
        // ---------------------------------------------------------------------------
        class Logging
        {
        public:
            virtual ~Logging() = default;

            virtual void begin(uint32_t baud_rate) = 0;
            virtual void printf(const char *format, ...) = 0;
            virtual void println(const char *str) = 0;
        };

        // ---------------------------------------------------------------------------
        // Platform — Aggregates all HAL interfaces for a platform
        // ---------------------------------------------------------------------------
        struct Platform
        {
            Gpio *gpio;
            Pwm *pwm;
            Adc *adc;
            I2c *i2c;
            Spi *spi;
            Wifi *wifi;
            Mqtt *mqtt;
            Http *http;
            Random *random;
            Timer *timer;
            Logging *logging;
        };

        // Get the current platform (set by platform initialization)
        Platform &get_platform();

        // Set the current platform (called by platform implementations)
        void set_platform(Platform *platform);

    } // namespace hal
} // namespace voodoo

#endif // VODOO_HAL_H
