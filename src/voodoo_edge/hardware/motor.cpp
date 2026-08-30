// motor.cpp
// Voodoo Edge — Motor controller implementation

#include "voodoo_edge/hardware/motor.h"
#include <cmath>

namespace voodoo
{
    namespace hardware
    {

        MotorController::MotorController(hal::Gpio &gpio, hal::Pwm &pwm,
                                         uint8_t dir_pin, uint8_t pwm_channel,
                                         uint32_t pwm_freq_hz, uint8_t pwm_resolution)
            : _gpio(gpio), _pwm(pwm), _dir_pin(dir_pin), _pwm_channel(pwm_channel), _pwm_freq_hz(pwm_freq_hz), _pwm_resolution(pwm_resolution), _speed(0.0f)
        {
        }

        void MotorController::begin()
        {
            _gpio.pinMode(_dir_pin, hal::Gpio::Mode::Output);
            _gpio.digitalWrite(_dir_pin, hal::Gpio::Level::Low);
            _pwm.setup(_pwm_channel, _pwm_freq_hz, _pwm_resolution);
            stop();
        }

        void MotorController::set_speed(float speed)
        {
            // Clamp to [-1.0, 1.0]
            if (speed > 1.0f)
                speed = 1.0f;
            if (speed < -1.0f)
                speed = -1.0f;

            _speed = speed;

            if (std::abs(speed) < 0.001f)
            {
                stop();
                return;
            }

            // Set direction
            if (speed > 0)
            {
                _gpio.digitalWrite(_dir_pin, hal::Gpio::Level::High);
            }
            else
            {
                _gpio.digitalWrite(_dir_pin, hal::Gpio::Level::Low);
            }

            // Set PWM duty
            float abs_speed = std::abs(speed);
            _pwm.write_float(_pwm_channel, abs_speed);
        }

        void MotorController::stop()
        {
            _speed = 0.0f;
            _pwm.write_float(_pwm_channel, 0.0f);
        }

        void MotorController::brake()
        {
            _speed = 0.0f;
            _gpio.digitalWrite(_dir_pin, hal::Gpio::Level::High);
            _pwm.write_float(_pwm_channel, 1.0f);
        }

        float MotorController::current_speed() const
        {
            return _speed;
        }

    } // namespace hardware
} // namespace voodoo
