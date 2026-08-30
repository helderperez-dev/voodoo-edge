// device.h
// Voodoo Edge — Device facade
//
// High-level API that ties together all SDK components.
// This is the primary entry point for application developers.
//
// Usage:
//   #include <voodoo_edge.h>
//
//   voodoo::edge::Device device;
//
//   void setup() {
//       device.begin("my-device", "esp32");
//       device.expose("led.control", led_handler, &led);
//       device.connect();
//   }
//
//   void loop() {
//       device.update();
//   }

#ifndef VODOO_CORE_DEVICE_H
#define VODOO_CORE_DEVICE_H

#include "voodoo_edge/core/protocol.h"
#include "voodoo_edge/core/transport.h"
#include "voodoo_edge/core/capability_registry.h"
#include "voodoo_edge/core/device_state.h"
#include "voodoo_edge/core/device_identity.h"
#include "voodoo_edge/core/effect_dispatcher.h"
#include "voodoo_edge/core/event_publisher.h"
#include "voodoo_edge/core/idempotency.h"
#include "voodoo_edge/core/connection_manager.h"

namespace voodoo
{
    namespace edge
    {

        // ---------------------------------------------------------------------------
        // Device configuration
        // ---------------------------------------------------------------------------
        struct DeviceConfig
        {
            DeviceIdentity identity;
            const char *credential;          // optional auth credential
            uint32_t heartbeat_interval_ms;  // default 30000
            uint32_t state_sync_interval_ms; // default 60000
        };

        // ---------------------------------------------------------------------------
        // Device — the main SDK entry point
        // ---------------------------------------------------------------------------
        class Device
        {
        public:
            Device();
            ~Device();

            // ---- Lifecycle ----

            // Initialize the device with an ID and type
            void begin(const char *device_id, const char *device_type = "esp32");

            // Initialize with full configuration
            void begin(const DeviceConfig &config);

            // Connect to the Voodoo Runtime via the given transport
            void connect(Transport &transport);

            // Call from loop() — handles all SDK housekeeping
            void update();

            // ---- Capabilities ----

            // Expose a capability with a handler
            bool expose(const char *name, CapabilityHandler handler, void *user_data = nullptr);

            // ---- Events ----

            // Emit an event
            bool emit(const char *event_name, JsonObjectConst payload = JsonObjectConst());

            // ---- State ----

            // Set a state field
            void set_state(const char *key, const char *value);
            void set_state(const char *key, int value);
            void set_state(const char *key, float value);
            void set_state(const char *key, bool value);

            // Force a state sync on next update
            void sync_state();

            // ---- Identity ----

            // Get the device identity
            const DeviceIdentity &identity() const;

            // Get the device ID
            const char *device_id() const;

            // ---- Status ----

            // Check if connected and ready
            bool is_ready() const;

            // Get connection state
            ConnectionState connection_state() const;

            // Get the connection manager (for advanced use)
            ConnectionManager &connection();

            // Get the capability registry (for advanced use)
            CapabilityRegistry &capabilities();

            // Get the device state (for advanced use)
            DeviceState &state();

            // Get the event publisher (for advanced use)
            EventPublisher &events();

        private:
            DeviceConfig _config;
            CapabilityRegistry _capabilities;
            DeviceState _state;
            IdempotencyCache _idempotency;
            EffectDispatcher *_dispatcher;
            EventPublisher *_events;
            ConnectionManager *_connection;
            Transport *_transport;
            bool _initialized;
            bool _connected;
        };

    } // namespace edge
} // namespace voodoo

#endif // VODOO_CORE_DEVICE_H
