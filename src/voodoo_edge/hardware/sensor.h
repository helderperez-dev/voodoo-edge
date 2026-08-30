// sensor.h
// Voodoo Edge — Generic sensor hardware abstraction
//
// Platform-independent analog sensor reader using ADC.
// Supports calibration offset, scale factor, and smoothing.

#ifndef VODOO_HARDWARE_SENSOR_H
#define VODOO_HARDWARE_SENSOR_H

#include "voodoo_edge/platform/hal.h"
#include <cstdint>

namespace voodoo
{
    namespace hardware
    {

        class SensorReader
        {
        public:
            SensorReader(hal::Adc &adc, uint8_t pin);

            void begin(hal::Adc::Attenuation atten = hal::Adc::Attenuation::DB_11);

            // Read raw ADC value
            uint16_t read_raw();

            // Read calibrated value: (raw - offset) * scale
            float read();

            // Read voltage
            float read_voltage();

            // Calibration: raw_value = (physical_value / scale) + offset
            void set_calibration(float offset, float scale);

            // Moving average smoothing (number of samples, 1 = no smoothing)
            void set_smoothing(uint8_t samples);

            // Get last smoothed reading
            float last_value() const;

        private:
            hal::Adc &_adc;
            uint8_t _pin;
            float _offset;
            float _scale;
            uint8_t _smoothing_samples;
            float _last_value;
            float _smooth_buffer[16]; // max 16 samples
            uint8_t _smooth_index;
            uint8_t _smooth_count;
        };

    } // namespace hardware
} // namespace voodoo

#endif // VODOO_HARDWARE_SENSOR_H
