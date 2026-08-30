// encoder.cpp
// Voodoo Edge — Encoder controller implementation

#include "voodoo_edge/hardware/encoder.h"

namespace voodoo
{
    namespace hardware
    {

        // Quadrature state transition table
        // Index: old_state << 2 | new_state
        // Value: -1 (CCW), 0 (no change), +1 (CW)
        static const int8_t QUADRATURE_TABLE[16] = {
            0, -1, 1, 0,
            1, 0, 0, -1,
            -1, 0, 0, 1,
            0, 1, -1, 0};

        EncoderController::EncoderController(hal::Gpio &gpio, uint8_t pin_a, uint8_t pin_b)
            : _gpio(gpio), _pin_a(pin_a), _pin_b(pin_b), _position(0), _last_state(0), _callback(nullptr), _user_data(nullptr)
        {
        }

        void EncoderController::begin()
        {
            _gpio.pinMode(_pin_a, hal::Gpio::Mode::InputPullUp);
            _gpio.pinMode(_pin_b, hal::Gpio::Mode::InputPullUp);
            _last_state = read_state();
        }

        void EncoderController::loop()
        {
            uint8_t state = read_state();
            if (state != _last_state)
            {
                uint8_t index = (_last_state << 2) | state;
                int8_t delta = QUADRATURE_TABLE[index & 0x0F];

                if (delta != 0)
                {
                    int32_t old_pos = _position;
                    _position += delta;

                    if (_callback && _position != old_pos)
                    {
                        _callback(_position, _user_data);
                    }
                }

                _last_state = state;
            }
        }

        int32_t EncoderController::position() const
        {
            return _position;
        }

        void EncoderController::reset()
        {
            _position = 0;
        }

        void EncoderController::on_change(EncoderCallback callback, void *user_data)
        {
            _callback = callback;
            _user_data = user_data;
        }

        uint8_t EncoderController::read_state() const
        {
            uint8_t a = static_cast<uint8_t>(_gpio.digitalRead(_pin_a));
            uint8_t b = static_cast<uint8_t>(_gpio.digitalRead(_pin_b));
            return (a << 1) | b;
        }

    } // namespace hardware
} // namespace voodoo
