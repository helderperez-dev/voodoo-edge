// hal.cpp
// Voodoo Edge — HAL platform registry

#include "voodoo_edge/platform/hal.h"

namespace voodoo {
namespace hal {

static Platform* _current_platform = nullptr;

Platform& get_platform() {
    // Return a default null platform if none set
    static Platform null_platform = {nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr};
    if (_current_platform) return *_current_platform;
    return null_platform;
}

void set_platform(Platform* platform) {
    _current_platform = platform;
}

} // namespace hal
} // namespace voodoo
