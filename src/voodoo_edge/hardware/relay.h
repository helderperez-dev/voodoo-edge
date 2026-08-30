// relay.h
// Voodoo Edge — Relay hardware abstraction
//
// Platform-independent relay controller using GPIO.

#ifndef VODOO_HARDWARE_RELAY_H
#define VODOO_HARDWARE_RELAY_H

#include "voodoo_edge/platform/hal.h"
#include <cstdint>

namespace voodoo
{
    namespace hardware
    {

        class RelayController
        {
        public:
            RelayController(hal::Gpio &gpio, uint8_t pin, bool active_high = true);

            void begin();
            void on();
            void off();
            void toggle();
            bool is_on() const;

        private:
            hal::Gpio &_gpio;
            uint8_t _pin;
            bool _active_high;
            bool _state;
        };

    } // namespace hardware
} // namespace voodoo

#endif // VODOO_HARDWARE_RELAY_H
