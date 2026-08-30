// test_capability_registry.cpp
// Voodoo Edge — Capability registry unit tests

#include <unity.h>
#include "voodoo_edge/core/capability_registry.h"

using namespace voodoo::edge;

static bool handler_called = false;
static bool test_handler(const EdgeMessage& effect, JsonObject result, void* user_data) {
    handler_called = true;
    result["ok"] = true;
    return true;
}

static bool test_handler2(const EdgeMessage& effect, JsonObject result, void* user_data) {
    return false;
}

void test_register_and_find() {
    CapabilityRegistry registry;
    TEST_ASSERT_TRUE(registry.register_capability("led.control", test_handler));
    TEST_ASSERT_EQUAL(1, registry.count());
    TEST_ASSERT_NOT_NULL(registry.find("led.control"));
    TEST_ASSERT_NULL(registry.find("nonexistent"));
}

void test_duplicate_registration() {
    CapabilityRegistry registry;
    TEST_ASSERT_TRUE(registry.register_capability("led.control", test_handler));
    TEST_ASSERT_TRUE(registry.register_capability("led.control", test_handler2));
    TEST_ASSERT_EQUAL(1, registry.count());
}

void test_max_capabilities() {
    CapabilityRegistry registry;
    for (int i = 0; i < MAX_CAPABILITIES; i++) {
        char name[32];
        snprintf(name, sizeof(name), "cap_%d", i);
        TEST_ASSERT_TRUE(registry.register_capability(name, test_handler));
    }
    TEST_ASSERT_FALSE(registry.register_capability("cap_overflow", test_handler));
}

void test_names_array() {
    CapabilityRegistry registry;
    registry.register_capability("led.control", test_handler);
    registry.register_capability("button.read", test_handler);

    const char* const* names = registry.names();
    TEST_ASSERT_NOT_NULL(names);
    TEST_ASSERT_EQUAL_STRING("led.control", names[0]);
    TEST_ASSERT_EQUAL_STRING("button.read", names[1]);
    TEST_ASSERT_NULL(names[2]);
}

void test_has() {
    CapabilityRegistry registry;
    registry.register_capability("led.control", test_handler);
    TEST_ASSERT_TRUE(registry.has("led.control"));
    TEST_ASSERT_FALSE(registry.has("nonexistent"));
}

void test_user_data() {
    int data = 42;
    CapabilityRegistry registry;
    registry.register_capability("led.control", test_handler, &data);
    TEST_ASSERT_EQUAL_PTR(&data, registry.find_user_data("led.control"));
}


