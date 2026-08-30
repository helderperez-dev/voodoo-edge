// servo.h
// Voodoo Edge — Servo motor hardware abstraction
//
// Platform-independent servo controller using PWM.
// Maps angle (0-180°) to appropriate pulse width.

#ifndef VODOO_HARDWARE_SERVO_H
#define VODOO_HARDWARE_SERVO_H

#include "voodoo_edge/platform/hal.h"
#include <cstdint>

namespace voodoo
{
    namespace hardware
    {

        class ServoController
        {
        public:
            ServoController(hal::Pwm &pwm, uint8_t pwm_channel,
                            uint16_t min_pulse_us = 544, uint16_t max_pulse_us = 2400,
                            uint32_t pwm_freq_hz = 50, uint8_t pwm_resolution = 16);

            void begin(uint8_t pin);

            // Set angle in degrees (0-180)
            void set_angle(uint16_t angle_deg);

            // Set angle as float (0.0 - 180.0)
            void set_angle_float(float angle_deg);

            // Detach (stop sending pulses)
            void detach();

            uint16_t current_angle() const;
            bool is_attached() const;

        private:
            hal::Pwm &_pwm;
            uint8_t _pwm_channel;
            uint8_t _pin;
            uint16_t _min_pulse_us;
            uint16_t _max_pulse_us;
            uint32_t _pwm_freq_hz;
            uint8_t _pwm_resolution;
            uint16_t _angle;
            bool _attached;

            uint32_t angle_to_duty(float angle_deg) const;
        };

    } // namespace hardware
} // namespace voodoo

#endif // VODOO_HARDWARE_SERVO_H
