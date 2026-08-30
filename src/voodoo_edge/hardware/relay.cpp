// relay.cpp
// Voodoo Edge — Relay controller implementation

#include "voodoo_edge/hardware/relay.h"

namespace voodoo
{
    namespace hardware
    {

        RelayController::RelayController(hal::Gpio &gpio, uint8_t pin, bool active_high)
            : _gpio(gpio), _pin(pin), _active_high(active_high), _state(false)
        {
        }

        void RelayController::begin()
        {
            _gpio.pinMode(_pin, hal::Gpio::Mode::OUTPUT);
            off();
        }

        void RelayController::on()
        {
            _gpio.digitalWrite(_pin, _active_high ? hal::Gpio::Level::HIGH : hal::Gpio::Level::LOW);
            _state = true;
        }

        void RelayController::off()
        {
            _gpio.digitalWrite(_pin, _active_high ? hal::Gpio::Level::LOW : hal::Gpio::Level::HIGH);
            _state = false;
        }

        void RelayController::toggle()
        {
            if (_state)
                off();
            else
                on();
        }

        bool RelayController::is_on() const
        {
            return _state;
        }

    } // namespace hardware
} // namespace voodoo
