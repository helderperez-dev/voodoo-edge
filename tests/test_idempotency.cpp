// test_idempotency.cpp
// Voodoo Edge — Idempotency cache unit tests

#include <unity.h>
#include "voodoo_edge/core/idempotency.h"

using namespace voodoo::edge;

void test_basic_seen() {
    IdempotencyCache cache;
    TEST_ASSERT_FALSE(cache.has_seen("msg_abc123"));
    cache.mark_seen("msg_abc123");
    TEST_ASSERT_TRUE(cache.has_seen("msg_abc123"));
    TEST_ASSERT_FALSE(cache.has_seen("msg_def456"));
}

void test_duplicate_mark() {
    IdempotencyCache cache;
    cache.mark_seen("msg_abc123");
    cache.mark_seen("msg_abc123");
    TEST_ASSERT_EQUAL(1, cache.count());
}

void test_circular_eviction() {
    IdempotencyCache cache;
    char id[32];
    for (size_t i = 0; i < IDEMPOTENCY_CACHE_SIZE + 5; i++) {
        snprintf(id, sizeof(id), "msg_%04zu", i);
        cache.mark_seen(id);
    }
    // Oldest entries should be evicted
    TEST_ASSERT_FALSE(cache.has_seen("msg_0000"));
    TEST_ASSERT_FALSE(cache.has_seen("msg_0004"));
    // Recent entries should still be there
    TEST_ASSERT_TRUE(cache.has_seen("msg_0034"));
    TEST_ASSERT_TRUE(cache.has_seen("msg_0036"));
}

void test_clear() {
    IdempotencyCache cache;
    cache.mark_seen("msg_abc123");
    cache.mark_seen("msg_def456");
    TEST_ASSERT_EQUAL(2, cache.count());
    cache.clear();
    TEST_ASSERT_EQUAL(0, cache.count());
    TEST_ASSERT_FALSE(cache.has_seen("msg_abc123"));
}

void test_null_handling() {
    IdempotencyCache cache;
    TEST_ASSERT_FALSE(cache.has_seen(nullptr));
    cache.mark_seen(nullptr);  // should not crash
    TEST_ASSERT_EQUAL(0, cache.count());
}

void setup() {
    UNITY_BEGIN();
    RUN_TEST(test_basic_seen);
    RUN_TEST(test_duplicate_mark);
    RUN_TEST(test_circular_eviction);
    RUN_TEST(test_clear);
    RUN_TEST(test_null_handling);
    UNITY_END();
}

void loop() {}
