// button.cpp
// Voodoo Edge — Button controller implementation

#include "voodoo_edge/hardware/button.h"

namespace voodoo {
namespace hardware {

ButtonController::ButtonController(hal::Gpio& gpio, hal::Timer& timer, uint8_t pin, uint32_t debounce_ms)
    : _gpio(gpio)
    , _timer(timer)
    , _pin(pin)
    , _debounce_ms(debounce_ms)
    , _last_state(hal::Gpio::Level::HIGH)
    , _debounced_state(hal::Gpio::Level::HIGH)
    , _last_debounce_time(0)
    , _callback(nullptr)
    , _user_data(nullptr)
{
}

void ButtonController::begin() {
    _gpio.pinMode(_pin, hal::Gpio::Mode::INPUT_PULLUP);
    _last_state = _gpio.digitalRead(_pin);
    _debounced_state = _last_state;
}

void ButtonController::loop() {
    hal::Gpio::Level reading = _gpio.digitalRead(_pin);

    if (reading != _last_state) {
        _last_debounce_time = _timer.millis();
    }

    if ((_timer.millis() - _last_debounce_time) > _debounce_ms) {
        if (reading != _debounced_state) {
            _debounced_state = reading;

            // Trigger on press (LOW = pressed for active-LOW button)
            if (_debounced_state == hal::Gpio::Level::LOW && _callback) {
                _callback(_user_data);
            }
        }
    }

    _last_state = reading;
}

void ButtonController::on_press(ButtonPressCallback callback, void* user_data) {
    _callback = callback;
    _user_data = user_data;
}

bool ButtonController::is_pressed() const {
    return _gpio.digitalRead(_pin) == hal::Gpio::Level::LOW;
}

} // namespace hardware
} // namespace voodoo
