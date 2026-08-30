// sensor.cpp
// Voodoo Edge — Sensor reader implementation

#include "voodoo_edge/hardware/sensor.h"
#include <cstring>

namespace voodoo
{
    namespace hardware
    {

        SensorReader::SensorReader(hal::Adc &adc, uint8_t pin)
            : _adc(adc), _pin(pin), _offset(0.0f), _scale(1.0f), _smoothing_samples(1), _last_value(0.0f), _smooth_index(0), _smooth_count(0)
        {
            memset(_smooth_buffer, 0, sizeof(_smooth_buffer));
        }

        void SensorReader::begin(hal::Adc::Attenuation atten)
        {
            _adc.setup(_pin, atten);
        }

        uint16_t SensorReader::read_raw()
        {
            return _adc.read(_pin);
        }

        float SensorReader::read()
        {
            uint16_t raw = read_raw();
            float calibrated = (static_cast<float>(raw) - _offset) * _scale;

            // Apply smoothing
            if (_smoothing_samples <= 1)
            {
                _last_value = calibrated;
                return _last_value;
            }

            _smooth_buffer[_smooth_index] = calibrated;
            _smooth_index = (_smooth_index + 1) % _smoothing_samples;
            if (_smooth_count < _smoothing_samples)
                _smooth_count++;

            // Calculate average
            float sum = 0.0f;
            for (uint8_t i = 0; i < _smooth_count; i++)
            {
                sum += _smooth_buffer[i];
            }
            _last_value = sum / _smooth_count;

            return _last_value;
        }

        float SensorReader::read_voltage()
        {
            return _adc.read_voltage(_pin);
        }

        void SensorReader::set_calibration(float offset, float scale)
        {
            _offset = offset;
            _scale = scale;
        }

        void SensorReader::set_smoothing(uint8_t samples)
        {
            if (samples > 16)
                samples = 16;
            if (samples < 1)
                samples = 1;
            _smoothing_samples = samples;
            _smooth_index = 0;
            _smooth_count = 0;
        }

        float SensorReader::last_value() const
        {
            return _last_value;
        }

    } // namespace hardware
} // namespace voodoo
