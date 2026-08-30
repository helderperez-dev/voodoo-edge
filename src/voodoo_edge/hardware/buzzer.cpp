// buzzer.cpp
// Voodoo Edge — Buzzer controller implementation

#include "voodoo_edge/hardware/buzzer.h"

namespace voodoo
{
    namespace hardware
    {

        BuzzerController::BuzzerController(hal::Pwm &pwm, uint8_t pwm_channel)
            : _pwm(pwm), _pwm_channel(pwm_channel), _pin(0), _playing(false), _tone_end_ms(0)
        {
        }

        void BuzzerController::begin(uint8_t pin)
        {
            _pin = pin;
            _pwm.setup(_pwm_channel, 1000, 8);
            _pwm.attach(_pin, _pwm_channel);
            no_tone();
        }

        void BuzzerController::tone(uint32_t freq_hz, uint32_t duration_ms)
        {
            _pwm.setup(_pwm_channel, freq_hz, 8);
            _pwm.write_float(_pwm_channel, 0.5f); // 50% duty for buzzer
            _playing = true;

            if (duration_ms > 0)
            {
                // Store end time — caller must call loop() or no_tone() to stop
                auto &platform = hal::get_platform();
                if (platform.timer)
                {
                    _tone_end_ms = platform.timer->millis() + duration_ms;
                }
            }
            else
            {
                _tone_end_ms = 0; // indefinite
            }
        }

        void BuzzerController::no_tone()
        {
            _pwm.write_float(_pwm_channel, 0.0f);
            _playing = false;
            _tone_end_ms = 0;
        }

        void BuzzerController::beep(uint32_t freq_hz, uint32_t duration_ms)
        {
            tone(freq_hz, duration_ms);
        }

        bool BuzzerController::is_playing() const
        {
            return _playing;
        }

    } // namespace hardware
} // namespace voodoo
