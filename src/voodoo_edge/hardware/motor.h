// motor.h
// Voodoo Edge — Motor hardware abstraction
//
// Platform-independent DC motor controller using PWM and GPIO.
// Supports direction control via H-bridge and speed via PWM.

#ifndef VODOO_HARDWARE_MOTOR_H
#define VODOO_HARDWARE_MOTOR_H

#include "voodoo_edge/platform/hal.h"
#include <cstdint>

namespace voodoo
{
    namespace hardware
    {

        class MotorController
        {
        public:
            // Two-pin H-bridge motor (direction + PWM speed)
            MotorController(hal::Gpio &gpio, hal::Pwm &pwm,
                            uint8_t dir_pin, uint8_t pwm_channel,
                            uint32_t pwm_freq_hz = 1000, uint8_t pwm_resolution = 8);

            void begin();

            // Speed: -1.0 (full reverse) to +1.0 (full forward), 0 = stop
            void set_speed(float speed);

            // Immediate stop (coast)
            void stop();

            // Active brake (both outputs driven)
            void brake();

            float current_speed() const;

        private:
            hal::Gpio &_gpio;
            hal::Pwm &_pwm;
            uint8_t _dir_pin;
            uint8_t _pwm_channel;
            uint32_t _pwm_freq_hz;
            uint8_t _pwm_resolution;
            float _speed;
        };

    } // namespace hardware
} // namespace voodoo

#endif // VODOO_HARDWARE_MOTOR_H
