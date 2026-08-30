// test_device_state.cpp
// Voodoo Edge — Device state unit tests

#include <unity.h>
#include "voodoo_edge/core/device_state.h"

using namespace voodoo::edge;

void test_initial_state() {
    DeviceState state;
    TEST_ASSERT_EQUAL(0, state.version());
    TEST_ASSERT_FALSE(state.has_changed());
}

void test_set_increments_version() {
    DeviceState state;
    state.set("led_state", true);
    TEST_ASSERT_EQUAL(1, state.version());
    TEST_ASSERT_TRUE(state.has_changed());

    state.set("temperature", 25);
    TEST_ASSERT_EQUAL(2, state.version());
}

void test_mark_synced() {
    DeviceState state;
    state.set("led_state", true);
    TEST_ASSERT_TRUE(state.has_changed());

    state.mark_synced();
    TEST_ASSERT_FALSE(state.has_changed());

    state.set("temperature", 25);
    TEST_ASSERT_TRUE(state.has_changed());
}

void test_state_json() {
    DeviceState state;
    state.set("led_state", true);
    state.set("temperature", 25);
    state.set("firmware_version", "1.0.0");

    JsonObjectConst s = state.state();
    TEST_ASSERT_TRUE(s["led_state"]);
    TEST_ASSERT_EQUAL(25, s["temperature"]);
    TEST_ASSERT_EQUAL_STRING("1.0.0", s["firmware_version"]);
}

void test_build_sync_message() {
    DeviceState state;
    state.set("led_state", true);

    EdgeMessage msg = state.build_sync_message("dev1");
    TEST_ASSERT_EQUAL(MessageType::STATE_SYNC, msg.type);
    TEST_ASSERT_EQUAL_STRING("dev1", msg.device_id);

    JsonObjectConst payload = msg.payload();
    TEST_ASSERT_EQUAL(1, payload["state_version"]);
}

void setup() {
    UNITY_BEGIN();
    RUN_TEST(test_initial_state);
    RUN_TEST(test_set_increments_version);
    RUN_TEST(test_mark_synced);
    RUN_TEST(test_state_json);
    RUN_TEST(test_build_sync_message);
    UNITY_END();
}

void loop() {}
