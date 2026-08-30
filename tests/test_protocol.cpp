// test_protocol.cpp
// Voodoo Edge — Protocol unit tests

#include <unity.h>
#include "voodoo_edge/core/protocol.h"

using namespace voodoo::edge;

void test_message_type_conversion() {
    TEST_ASSERT_EQUAL_STRING("hello", message_type_to_string(MessageType::HELLO));
    TEST_ASSERT_EQUAL_STRING("auth", message_type_to_string(MessageType::AUTH));
    TEST_ASSERT_EQUAL_STRING("state_sync", message_type_to_string(MessageType::STATE_SYNC));
    TEST_ASSERT_EQUAL_STRING("event", message_type_to_string(MessageType::EVENT));
    TEST_ASSERT_EQUAL_STRING("effect", message_type_to_string(MessageType::EFFECT));
    TEST_ASSERT_EQUAL_STRING("effect_ack", message_type_to_string(MessageType::EFFECT_ACK));
    TEST_ASSERT_EQUAL_STRING("heartbeat", message_type_to_string(MessageType::HEARTBEAT));
    TEST_ASSERT_EQUAL_STRING("unknown", message_type_to_string(MessageType::UNKNOWN));
}

void test_message_type_from_string() {
    TEST_ASSERT_EQUAL(MessageType::HELLO, message_type_from_string("hello"));
    TEST_ASSERT_EQUAL(MessageType::AUTH, message_type_from_string("auth"));
    TEST_ASSERT_EQUAL(MessageType::STATE_SYNC, message_type_from_string("state_sync"));
    TEST_ASSERT_EQUAL(MessageType::EVENT, message_type_from_string("event"));
    TEST_ASSERT_EQUAL(MessageType::EFFECT, message_type_from_string("effect"));
    TEST_ASSERT_EQUAL(MessageType::EFFECT_ACK, message_type_from_string("effect_ack"));
    TEST_ASSERT_EQUAL(MessageType::HEARTBEAT, message_type_from_string("heartbeat"));
    TEST_ASSERT_EQUAL(MessageType::UNKNOWN, message_type_from_string("bogus"));
}

void test_validate_event_name() {
    TEST_ASSERT_TRUE(validate_event_name("button.pressed"));
    TEST_ASSERT_TRUE(validate_event_name("device.state_changed"));
    TEST_ASSERT_TRUE(validate_event_name("temperature.changed"));

    TEST_ASSERT_FALSE(validate_event_name(""));
    TEST_ASSERT_FALSE(validate_event_name(nullptr));
    TEST_ASSERT_FALSE(validate_event_name("no_dot"));
    TEST_ASSERT_FALSE(validate_event_name(".starts_with_dot"));
    TEST_ASSERT_FALSE(validate_event_name("ends_with_dot."));
    TEST_ASSERT_FALSE(validate_event_name("has.UPPERCASE"));
    TEST_ASSERT_FALSE(validate_event_name("has space.test"));
}

void test_validate_protocol_version() {
    TEST_ASSERT_TRUE(validate_protocol_version("1"));
    TEST_ASSERT_FALSE(validate_protocol_version("2"));
    TEST_ASSERT_FALSE(validate_protocol_version(nullptr));
    TEST_ASSERT_FALSE(validate_protocol_version(""));
}

void test_edge_message_serialize_deserialize() {
    EdgeMessage msg;
    msg.type = MessageType::EVENT;
    strncpy(msg.device_id, "test_device", sizeof(msg.device_id) - 1);
    strncpy(msg.message_id, "msg_test123", sizeof(msg.message_id) - 1);

    JsonObject payload = msg.payload_mutable();
    payload["event_name"] = "button.pressed";

    char buffer[512];
    size_t len = msg.serialize(buffer, sizeof(buffer));
    TEST_ASSERT_GREATER_THAN(0, len);

    EdgeMessage msg2;
    TEST_ASSERT_TRUE(msg2.deserialize(buffer, len));
    TEST_ASSERT_EQUAL(MessageType::EVENT, msg2.type);
    TEST_ASSERT_EQUAL_STRING("test_device", msg2.device_id);
    TEST_ASSERT_EQUAL_STRING("msg_test123", msg2.message_id);
}

void test_build_hello() {
    const char* caps[] = {"led.control", "button.read"};
    EdgeMessage msg = build_hello("dev1", "esp32", "1.0.0", caps, 2);

    TEST_ASSERT_EQUAL(MessageType::HELLO, msg.type);
    TEST_ASSERT_EQUAL_STRING("dev1", msg.device_id);

    JsonObjectConst payload = msg.payload();
    TEST_ASSERT_EQUAL_STRING("esp32", payload["device_type"]);
    TEST_ASSERT_EQUAL_STRING("1.0.0", payload["firmware_version"]);
    TEST_ASSERT_EQUAL(2, payload["capabilities"].size());
}

void test_build_event() {
    JsonDocument payload_doc;
    JsonObject payload = payload_doc.to<JsonObject>();
    payload["pin"] = 0;

    EdgeMessage msg = build_event("dev1", "button.pressed", payload);
    TEST_ASSERT_EQUAL(MessageType::EVENT, msg.type);
    TEST_ASSERT_EQUAL_STRING("dev1", msg.device_id);

    JsonObjectConst p = msg.payload();
    TEST_ASSERT_EQUAL_STRING("button.pressed", p["event_name"]);
    TEST_ASSERT_EQUAL(0, p["event_payload"]["pin"]);
}

void test_build_effect_ack() {
    EdgeMessage msg = build_effect_ack("dev1", "eff123", "exec456", EffectAckStatus::COMPLETED);
    TEST_ASSERT_EQUAL(MessageType::EFFECT_ACK, msg.type);

    JsonObjectConst payload = msg.payload();
    TEST_ASSERT_EQUAL_STRING("eff123", payload["effect_id"]);
    TEST_ASSERT_EQUAL_STRING("completed", payload["status"]);
}

void test_build_heartbeat() {
    EdgeMessage msg = build_heartbeat("dev1", 42, 3600);
    TEST_ASSERT_EQUAL(MessageType::HEARTBEAT, msg.type);

    JsonObjectConst payload = msg.payload();
    TEST_ASSERT_EQUAL(42, payload["state_version"]);
    TEST_ASSERT_EQUAL(3600, payload["uptime_seconds"]);
}

void test_generate_message_id() {
    char id1[32], id2[32];
    generate_message_id(id1, sizeof(id1));
    generate_message_id(id2, sizeof(id2));

    // Should start with "msg_"
    TEST_ASSERT_EQUAL_STRING("msg_", id1);  // prefix check
    TEST_ASSERT_EQUAL(24, strlen(id1));      // "msg_" + 20 hex

    // Two IDs should be different (with high probability)
    TEST_ASSERT_FALSE(strcmp(id1, id2) == 0);
}

void setup() {
    UNITY_BEGIN();
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
    UNITY_END();
}

void loop() {}
