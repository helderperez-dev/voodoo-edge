// temperature_sensor.cpp
// Voodoo Edge — Temperature sensor implementations

#include "voodoo_edge/hardware/temperature_sensor.h"
#include <cmath>

namespace voodoo
{
    namespace hardware
    {

        // ---------------------------------------------------------------------------
        // NtcThermistor
        // ---------------------------------------------------------------------------

        NtcThermistor::NtcThermistor(hal::Adc &adc, uint8_t pin,
                                     float nominal_r, float nominal_temp,
                                     float b_coefficient, float series_r)
            : _adc(adc), _pin(pin), _nominal_r(nominal_r), _nominal_temp(nominal_temp), _b_coefficient(b_coefficient), _series_r(series_r), _available(false)
        {
        }

        void NtcThermistor::begin()
        {
            _adc.setup(_pin, hal::Adc::Attenuation::DB_11);
            _available = true;
        }

        float NtcThermistor::read_celsius()
        {
            if (!_available)
                return -999.0f;

            uint16_t raw = _adc.read(_pin);
            if (raw == 0 || raw >= 4095)
                return -999.0f;

            // Calculate NTC resistance from voltage divider
            float voltage = static_cast<float>(raw) / 4095.0f;
            float r_ntc = _series_r * voltage / (1.0f - voltage);

            // Steinhart-Hart simplified (B equation)
            float t0_kelvin = _nominal_temp + 273.15f;
            float t_kelvin = 1.0f / (1.0f / t0_kelvin + (1.0f / _b_coefficient) * logf(r_ntc / _nominal_r));

            return t_kelvin - 273.15f;
        }

        bool NtcThermistor::available() const
        {
            return _available;
        }

        // ---------------------------------------------------------------------------
        // I2cTemperatureSensor
        // ---------------------------------------------------------------------------

        I2cTemperatureSensor::I2cTemperatureSensor(hal::I2c &i2c, uint8_t addr,
                                                   uint8_t reg, uint8_t resolution,
                                                   float lsb_temp)
            : _i2c(i2c), _addr(addr), _reg(reg), _resolution(resolution), _lsb_temp(lsb_temp), _available(false)
        {
        }

        void I2cTemperatureSensor::begin()
        {
            // Try to read from the sensor to check availability
            uint8_t buf[2];
            _available = _i2c.write_read(_addr, &_reg, 1, buf, 2);
        }

        float I2cTemperatureSensor::read_celsius()
        {
            if (!_available)
                return -999.0f;

            uint8_t buf[2];
            if (!_i2c.write_read(_addr, &_reg, 1, buf, 2))
            {
                return -999.0f;
            }

            // Combine bytes (MSB first, typical for I2C temp sensors)
            int16_t raw = static_cast<int16_t>((buf[0] << 8) | buf[1]);

            // Shift to match resolution (e.g., 12-bit in upper bits)
            if (_resolution == 12)
            {
                raw >>= 4;
            }

            return static_cast<float>(raw) * _lsb_temp;
        }

        bool I2cTemperatureSensor::available() const
        {
            return _available;
        }

    } // namespace hardware
} // namespace voodoo
