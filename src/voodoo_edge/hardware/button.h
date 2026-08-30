// button.h
// Voodoo Edge — Button hardware abstraction
//
// Platform-independent debounced button using the HAL GPIO and Timer interfaces.

#ifndef VODOO_HARDWARE_BUTTON_H
#define VODOO_HARDWARE_BUTTON_H

#include "voodoo_edge/platform/hal.h"
#include <cstdint>

namespace voodoo {
namespace hardware {

typedef void (*ButtonPressCallback)(void* user_data);

class ButtonController {
public:
    ButtonController(hal::Gpio& gpio, hal::Timer& timer, uint8_t pin, uint32_t debounce_ms = 50);

    void begin();
    void loop();
    void on_press(ButtonPressCallback callback, void* user_data = nullptr);
    bool is_pressed() const;

private:
    hal::Gpio& _gpio;
    hal::Timer& _timer;
    uint8_t _pin;
    uint32_t _debounce_ms;
    hal::Gpio::Level _last_state;
    hal::Gpio::Level _debounced_state;
    uint32_t _last_debounce_time;
    ButtonPressCallback _callback;
    void* _user_data;
};

} // namespace hardware
} // namespace voodoo

#endif // VODOO_HARDWARE_BUTTON_H
