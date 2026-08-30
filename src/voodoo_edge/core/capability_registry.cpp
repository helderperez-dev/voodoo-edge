// capability_registry.cpp
// Voodoo Edge — Capability registry implementation

#include "voodoo_edge/core/capability_registry.h"
#include <cstring>

namespace voodoo {
namespace edge {

CapabilityRegistry::CapabilityRegistry() : _count(0) {
    memset(_entries, 0, sizeof(_entries));
    memset(_name_ptrs, 0, sizeof(_name_ptrs));
}

bool CapabilityRegistry::register_capability(const char* name, CapabilityHandler handler, void* user_data) {
    if (!name || !handler) return false;
    if (_count >= MAX_CAPABILITIES) return false;

    // Check for duplicate
    for (size_t i = 0; i < _count; i++) {
        if (strcmp(_entries[i].name, name) == 0) {
            // Update existing entry
            _entries[i].handler = handler;
            _entries[i].user_data = user_data;
            return true;
        }
    }

    // Add new entry
    strncpy(_entries[_count].name, name, MAX_CAPABILITY_NAME_LEN - 1);
    _entries[_count].name[MAX_CAPABILITY_NAME_LEN - 1] = '\0';
    _entries[_count].handler = handler;
    _entries[_count].user_data = user_data;
    _name_ptrs[_count] = _entries[_count].name;
    _name_ptrs[_count + 1] = nullptr;
    _count++;

    return true;
}

CapabilityHandler CapabilityRegistry::find(const char* name) const {
    if (!name) return nullptr;
    for (size_t i = 0; i < _count; i++) {
        if (strcmp(_entries[i].name, name) == 0) {
            return _entries[i].handler;
        }
    }
    return nullptr;
}

void* CapabilityRegistry::find_user_data(const char* name) const {
    if (!name) return nullptr;
    for (size_t i = 0; i < _count; i++) {
        if (strcmp(_entries[i].name, name) == 0) {
            return _entries[i].user_data;
        }
    }
    return nullptr;
}

size_t CapabilityRegistry::count() const {
    return _count;
}

const char* const* CapabilityRegistry::names() const {
    return _name_ptrs;
}

bool CapabilityRegistry::has(const char* name) const {
    return find(name) != nullptr;
}

} // namespace edge
} // namespace voodoo
