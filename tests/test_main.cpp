// test_main.cpp
// Voodoo Edge — Unity test runner for native tests

#include <unity.h>

// Forward declarations of test functions from test_protocol.cpp
extern void test_message_type_conversion();
extern void test_message_type_from_string();
extern void test_validate_event_name();
extern void test_validate_protocol_version();
extern void test_edge_message_serialize_deserialize();
extern void test_build_hello();
extern void test_build_event();
extern void test_build_effect_ack();
extern void test_build_heartbeat();
extern void test_generate_message_id();

// Forward declarations from test_capability_registry.cpp
extern void test_register_and_find();
extern void test_duplicate_registration();
extern void test_max_capabilities();
extern void test_names_array();
extern void test_has();
extern void test_user_data();

// Forward declarations from test_device_state.cpp
extern void test_initial_state();
extern void test_set_increments_version();
extern void test_mark_synced();
extern void test_state_json();
extern void test_build_sync_message();

// Forward declarations from test_idempotency.cpp
extern void test_basic_seen();
extern void test_duplicate_mark();
extern void test_circular_eviction();
extern void test_clear();
extern void test_null_handling();

void setUp(void) {}
void tearDown(void) {}

int main(void) {
    UNITY_BEGIN();

    // Protocol tests
    RUN_TEST(test_message_type_conversion);
    RUN_TEST(test_message_type_from_string);
    RUN_TEST(test_validate_event_name);
    RUN_TEST(test_validate_protocol_version);
    RUN_TEST(test_edge_message_serialize_deserialize);
    RUN_TEST(test_build_hello);
    RUN_TEST(test_build_event);
    RUN_TEST(test_build_effect_ack);
    RUN_TEST(test_build_heartbeat);
    RUN_TEST(test_generate_message_id);

    // Capability registry tests
    RUN_TEST(test_register_and_find);
    RUN_TEST(test_duplicate_registration);
    RUN_TEST(test_max_capabilities);
    RUN_TEST(test_names_array);
    RUN_TEST(test_has);
    RUN_TEST(test_user_data);

    // Device state tests
    RUN_TEST(test_initial_state);
    RUN_TEST(test_set_increments_version);
    RUN_TEST(test_mark_synced);
    RUN_TEST(test_state_json);
    RUN_TEST(test_build_sync_message);

    // Idempotency tests
    RUN_TEST(test_basic_seen);
    RUN_TEST(test_duplicate_mark);
    RUN_TEST(test_circular_eviction);
    RUN_TEST(test_clear);
    RUN_TEST(test_null_handling);

    return UNITY_END();
}
