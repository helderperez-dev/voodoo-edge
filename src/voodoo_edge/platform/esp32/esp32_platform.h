// esp32_platform.h
// Voodoo Edge — ESP32 platform HAL implementation
//
// Provides concrete implementations of all HAL interfaces for ESP32.

#ifndef VODOO_PLATFORM_ESP32_H
#define VODOO_PLATFORM_ESP32_H

#include "voodoo_edge/platform/hal.h"

namespace voodoo {
namespace platform {
namespace esp32 {

// Initialize the ESP32 platform and register it as the active platform
void init();

// Get the ESP32 platform instance
hal::Platform& platform();

} // namespace esp32
} // namespace platform
} // namespace voodoo

#endif // VODOO_PLATFORM_ESP32_H
