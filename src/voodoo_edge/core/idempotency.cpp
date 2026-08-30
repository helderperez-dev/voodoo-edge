// idempotency.cpp
// Voodoo Edge — Idempotency cache implementation

#include "voodoo_edge/core/idempotency.h"
#include <cstring>

namespace voodoo {
namespace edge {

IdempotencyCache::IdempotencyCache() : _write_pos(0), _count(0) {
    memset(_entries, 0, sizeof(_entries));
}

bool IdempotencyCache::has_seen(const char* effect_id) const {
    if (!effect_id) return false;
    for (size_t i = 0; i < _count; i++) {
        if (_entries[i].used && strcmp(_entries[i].id, effect_id) == 0) {
            return true;
        }
    }
    return false;
}

void IdempotencyCache::mark_seen(const char* effect_id) {
    if (!effect_id) return;

    // Check if already present
    if (has_seen(effect_id)) return;

    // Add to circular buffer
    strncpy(_entries[_write_pos].id, effect_id, sizeof(_entries[0].id) - 1);
    _entries[_write_pos].id[sizeof(_entries[0].id) - 1] = '\0';
    _entries[_write_pos].used = true;

    _write_pos = (_write_pos + 1) % IDEMPOTENCY_CACHE_SIZE;
    if (_count < IDEMPOTENCY_CACHE_SIZE) _count++;
}

void IdempotencyCache::clear() {
    memset(_entries, 0, sizeof(_entries));
    _write_pos = 0;
    _count = 0;
}

size_t IdempotencyCache::count() const {
    return _count;
}

} // namespace edge
} // namespace voodoo
