// servo.cpp
// Voodoo Edge — Servo controller implementation

#include "voodoo_edge/hardware/servo.h"
#include <cmath>

namespace voodoo
{
    namespace hardware
    {

        ServoController::ServoController(hal::Pwm &pwm, uint8_t pwm_channel,
                                         uint16_t min_pulse_us, uint16_t max_pulse_us,
                                         uint32_t pwm_freq_hz, uint8_t pwm_resolution)
            : _pwm(pwm), _pwm_channel(pwm_channel), _pin(0), _min_pulse_us(min_pulse_us), _max_pulse_us(max_pulse_us), _pwm_freq_hz(pwm_freq_hz), _pwm_resolution(pwm_resolution), _angle(90), _attached(false)
        {
        }

        void ServoController::begin(uint8_t pin)
        {
            _pin = pin;
            _pwm.setup(_pwm_channel, _pwm_freq_hz, _pwm_resolution);
            _pwm.attach(_pin, _pwm_channel);
            _attached = true;
            set_angle(90); // center
        }

        void ServoController::set_angle(uint16_t angle_deg)
        {
            if (angle_deg > 180)
                angle_deg = 180;
            set_angle_float(static_cast<float>(angle_deg));
        }

        void ServoController::set_angle_float(float angle_deg)
        {
            if (angle_deg < 0.0f)
                angle_deg = 0.0f;
            if (angle_deg > 180.0f)
                angle_deg = 180.0f;

            _angle = static_cast<uint16_t>(angle_deg);
            _pwm.write(_pwm_channel, angle_to_duty(angle_deg));
        }

        void ServoController::detach()
        {
            _pwm.stop(_pwm_channel);
            _attached = false;
        }

        uint16_t ServoController::current_angle() const
        {
            return _angle;
        }

        bool ServoController::is_attached() const
        {
            return _attached;
        }

        uint32_t ServoController::angle_to_duty(float angle_deg) const
        {
            // Map angle to pulse width in microseconds
            float pulse_us = _min_pulse_us + (angle_deg / 180.0f) * (_max_pulse_us - _min_pulse_us);

            // Calculate period in microseconds
            float period_us = 1000000.0f / _pwm_freq_hz;

            // Calculate duty as fraction of max duty
            float max_duty = static_cast<float>((1 << _pwm_resolution) - 1);
            float duty = (pulse_us / period_us) * max_duty;

            return static_cast<uint32_t>(duty);
        }

    } // namespace hardware
} // namespace voodoo
