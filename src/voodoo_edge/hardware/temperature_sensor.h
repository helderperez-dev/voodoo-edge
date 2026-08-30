// temperature_sensor.h
// Voodoo Edge — Temperature sensor hardware abstraction
//
// Platform-independent temperature reading.
// Supports NTC thermistors (via ADC) and I2C digital sensors.

#ifndef VODOO_HARDWARE_TEMPERATURE_SENSOR_H
#define VODOO_HARDWARE_TEMPERATURE_SENSOR_H

#include "voodoo_edge/platform/hal.h"
#include <cstdint>

namespace voodoo
{
    namespace hardware
    {

        // ---------------------------------------------------------------------------
        // TemperatureSensor — abstract interface
        // ---------------------------------------------------------------------------
        class TemperatureSensor
        {
        public:
            virtual ~TemperatureSensor() = default;

            virtual void begin() = 0;
            virtual float read_celsius() = 0;
            virtual bool available() const = 0;
        };

        // ---------------------------------------------------------------------------
        // NtcThermistor — analog NTC thermistor via ADC
        // ---------------------------------------------------------------------------
        // Uses Steinhart-Hart simplified equation:
        //   T = 1 / (1/T0 + (1/B) * ln(R/R0))
        class NtcThermistor : public TemperatureSensor
        {
        public:
            // adc: ADC interface, pin: analog pin
            // nominal_r: resistance at nominal_temp (ohms), nominal_temp: reference temp (°C)
            // b_coefficient: B value from datasheet, series_r: series resistor (ohms)
            NtcThermistor(hal::Adc &adc, uint8_t pin,
                          float nominal_r = 10000.0f, float nominal_temp = 25.0f,
                          float b_coefficient = 3950.0f, float series_r = 10000.0f);

            void begin() override;
            float read_celsius() override;
            bool available() const override;

        private:
            hal::Adc &_adc;
            uint8_t _pin;
            float _nominal_r;
            float _nominal_temp;
            float _b_coefficient;
            float _series_r;
            bool _available;
        };

        // ---------------------------------------------------------------------------
        // I2cTemperatureSensor — generic I2C temperature sensor
        // ---------------------------------------------------------------------------
        // For sensors like TMP102, LM75, etc. that report temperature as a
        // 12-bit or 16-bit signed value in a register.
        class I2cTemperatureSensor : public TemperatureSensor
        {
        public:
            // addr: I2C address, reg: temperature register, resolution: bits (12 or 16)
            // lsb_temp: temperature per LSB (e.g. 0.0625 for 12-bit TMP102)
            I2cTemperatureSensor(hal::I2c &i2c, uint8_t addr,
                                 uint8_t reg = 0x00, uint8_t resolution = 12,
                                 float lsb_temp = 0.0625f);

            void begin() override;
            float read_celsius() override;
            bool available() const override;

        private:
            hal::I2c &_i2c;
            uint8_t _addr;
            uint8_t _reg;
            uint8_t _resolution;
            float _lsb_temp;
            bool _available;
        };

    } // namespace hardware
} // namespace voodoo

#endif // VODOO_HARDWARE_TEMPERATURE_SENSOR_H
