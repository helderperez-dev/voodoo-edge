// idempotency.h
// Voodoo Edge — Idempotency cache
//
// Bounded circular buffer that tracks recently processed effect IDs.
// Prevents duplicate processing of effects that may be delivered more than once.

#ifndef VODOO_CORE_IDEMPOTENCY_H
#define VODOO_CORE_IDEMPOTENCY_H

#include <cstdint>
#include <cstddef>

#ifndef IDEMPOTENCY_CACHE_SIZE
#define IDEMPOTENCY_CACHE_SIZE 32
#endif

namespace voodoo {
namespace edge {

class IdempotencyCache {
public:
    IdempotencyCache();

    // Check if an effect ID has been seen
    bool has_seen(const char* effect_id) const;

    // Mark an effect ID as seen
    void mark_seen(const char* effect_id);

    // Clear the cache
    void clear();

    // Get the number of entries
    size_t count() const;

private:
    struct Entry {
        char id[32];
        bool used;
    };

    Entry _entries[IDEMPOTENCY_CACHE_SIZE];
    size_t _write_pos;
    size_t _count;
};

} // namespace edge
} // namespace voodoo

#endif // VODOO_CORE_IDEMPOTENCY_H
