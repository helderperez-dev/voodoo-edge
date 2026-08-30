// led.h
// Voodoo Edge — LED hardware abstraction
//
// Platform-independent LED controller using the HAL GPIO interface.

#ifndef VODOO_HARDWARE_LED_H
#define VODOO_HARDWARE_LED_H

#include "voodoo_edge/platform/hal.h"
#include <cstdint>

namespace voodoo {
namespace hardware {

class LedController {
public:
    LedController(hal::Gpio& gpio, uint8_t pin);

    void begin();
    void on();
    void off();
    void toggle();
    bool is_on() const;

private:
    hal::Gpio& _gpio;
    uint8_t _pin;
    bool _state;
};

} // namespace hardware
} // namespace voodoo

#endif // VODOO_HARDWARE_LED_H
