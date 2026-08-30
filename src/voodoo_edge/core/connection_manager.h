// connection_manager.h
// Voodoo Edge — Connection manager
//
// Manages the lifecycle of an Edge connection to the Voodoo Runtime:
//   HELLO → AUTH → connected → heartbeat / state sync → reconnect
//
// This is the state machine that orchestrates protocol messages.

#ifndef VODOO_CORE_CONNECTION_MANAGER_H
#define VODOO_CORE_CONNECTION_MANAGER_H

#include "voodoo_edge/core/protocol.h"
#include "voodoo_edge/core/transport.h"
#include "voodoo_edge/core/capability_registry.h"
#include "voodoo_edge/core/device_state.h"
#include "voodoo_edge/core/effect_dispatcher.h"
#include "voodoo_edge/core/event_publisher.h"
#include "voodoo_edge/core/idempotency.h"

namespace voodoo
{
    namespace edge
    {

        // ---------------------------------------------------------------------------
        // Connection state
        // ---------------------------------------------------------------------------
        enum class ConnectionState : uint8_t
        {
            DISCONNECTED = 0,
            CONNECTING = 1,
            TRANSPORT_CONNECTED = 2,
            HELLO_SENT = 3,
            AUTHENTICATED = 4,
            READY = 5,
            RECONNECTING = 6
        };

        const char *connection_state_to_string(ConnectionState state);

        // ---------------------------------------------------------------------------
        // ConnectionManager configuration
        // ---------------------------------------------------------------------------
        struct ConnectionConfig
        {
            const char *device_id;
            const char *device_type;
            const char *firmware_version;
            const char *credential;          // optional, for AUTH message
            uint32_t heartbeat_interval_ms;  // default 30000
            uint32_t state_sync_interval_ms; // default 60000
            uint32_t reconnect_initial_ms;   // default 1000
            uint32_t reconnect_max_ms;       // default 60000
        };

        // ---------------------------------------------------------------------------
        // ConnectionManager
        // ---------------------------------------------------------------------------
        class ConnectionManager
        {
        public:
            ConnectionManager(
                Transport &transport,
                CapabilityRegistry &capabilities,
                DeviceState &state,
                EffectDispatcher &dispatcher,
                EventPublisher &events,
                IdempotencyCache &idempotency,
                const ConnectionConfig &config);

            // Start the connection process
            void begin();

            // Call from loop() — handles heartbeat, state sync, reconnection
            void loop();

            // Get current connection state
            ConnectionState state() const;

            // Check if ready to send/receive
            bool is_ready() const;

            // Force a state sync on next loop
            void request_state_sync();

            // Get the device ID
            const char *device_id() const;

        private:
            Transport &_transport;
            CapabilityRegistry &_capabilities;
            DeviceState &_device_state;
            EffectDispatcher &_dispatcher;
            EventPublisher &_events;
            IdempotencyCache &_idempotency;
            ConnectionConfig _config;

            ConnectionState _state;
            uint32_t _last_heartbeat;
            uint32_t _last_state_sync;
            uint32_t _reconnect_delay;
            uint32_t _last_reconnect_attempt;
            bool _state_sync_requested;

            void enter_state(ConnectionState new_state);
            void send_hello();
            void send_auth();
            void send_heartbeat();
            void send_state_sync();
            void handle_effect(const EdgeMessage &effect);
            void attempt_reconnect();

            // Static callback trampoline
            static void effect_callback(const EdgeMessage &effect, void *user_data);
        };

    } // namespace edge
} // namespace voodoo

#endif // VODOO_CORE_CONNECTION_MANAGER_H
