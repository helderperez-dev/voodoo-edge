// connection_manager.cpp
// Voodoo Edge — Connection manager implementation

#include "voodoo_edge/core/connection_manager.h"
#include "voodoo_edge/platform/hal.h"
#include <cstring>

namespace voodoo
{
    namespace edge
    {

        const char *connection_state_to_string(ConnectionState state)
        {
            switch (state)
            {
            case ConnectionState::DISCONNECTED:
                return "disconnected";
            case ConnectionState::CONNECTING:
                return "connecting";
            case ConnectionState::TRANSPORT_CONNECTED:
                return "transport_connected";
            case ConnectionState::HELLO_SENT:
                return "hello_sent";
            case ConnectionState::AUTHENTICATED:
                return "authenticated";
            case ConnectionState::READY:
                return "ready";
            case ConnectionState::RECONNECTING:
                return "reconnecting";
            default:
                return "unknown";
            }
        }

        ConnectionManager::ConnectionManager(
            Transport &transport,
            CapabilityRegistry &capabilities,
            DeviceState &state,
            EffectDispatcher &dispatcher,
            EventPublisher &events,
            IdempotencyCache &idempotency,
            const ConnectionConfig &config)
            : _transport(transport), _capabilities(capabilities), _device_state(state), _dispatcher(dispatcher), _events(events), _idempotency(idempotency), _config(config), _state(ConnectionState::DISCONNECTED), _last_heartbeat(0), _last_state_sync(0), _reconnect_delay(config.reconnect_initial_ms), _last_reconnect_attempt(0), _state_sync_requested(false)
        {
        }

        void ConnectionManager::begin()
        {
            auto &log = hal::get_platform().logging;
            if (log)
                log->printf("[connection] starting for device %s\n", _config.device_id);

            // Register effect callback
            _transport.on_effect(effect_callback, this);

            // Start transport
            enter_state(ConnectionState::CONNECTING);
            if (_transport.begin())
            {
                enter_state(ConnectionState::TRANSPORT_CONNECTED);
                send_hello();

                // Subscribe to effects
                _transport.subscribe_effects();

                // Send AUTH if credential provided
                if (_config.credential && _config.credential[0] != '\0')
                {
                    send_auth();
                }
                else
                {
                    // No auth required — go directly to READY
                    enter_state(ConnectionState::READY);
                    send_state_sync();
                }
            }
            else
            {
                enter_state(ConnectionState::DISCONNECTED);
            }
        }

        void ConnectionManager::loop()
        {
            auto &platform = hal::get_platform();
            uint32_t now = platform.timer ? platform.timer->millis() : 0;

            // Let transport do its work (handles reconnection at transport level)
            _transport.loop();

            // Check if transport disconnected
            if (_state >= ConnectionState::TRANSPORT_CONNECTED && !_transport.connected())
            {
                auto &log = platform.logging;
                if (log)
                    log->printf("[connection] transport disconnected\n");
                enter_state(ConnectionState::RECONNECTING);
            }

            // Handle reconnection
            if (_state == ConnectionState::RECONNECTING)
            {
                attempt_reconnect();
                return;
            }

            // Only send heartbeats and state sync when ready
            if (_state != ConnectionState::READY)
                return;

            // Heartbeat
            if (_config.heartbeat_interval_ms > 0 &&
                (now - _last_heartbeat) >= _config.heartbeat_interval_ms)
            {
                send_heartbeat();
            }

            // State sync
            if (_config.state_sync_interval_ms > 0 &&
                ((now - _last_state_sync) >= _config.state_sync_interval_ms || _state_sync_requested))
            {
                if (_device_state.has_changed() || _state_sync_requested)
                {
                    send_state_sync();
                    _state_sync_requested = false;
                }
            }
        }

        ConnectionState ConnectionManager::state() const
        {
            return _state;
        }

        bool ConnectionManager::is_ready() const
        {
            return _state == ConnectionState::READY;
        }

        void ConnectionManager::request_state_sync()
        {
            _state_sync_requested = true;
        }

        const char *ConnectionManager::device_id() const
        {
            return _config.device_id;
        }

        void ConnectionManager::enter_state(ConnectionState new_state)
        {
            auto &log = hal::get_platform().logging;
            if (log)
            {
                log->printf("[connection] %s -> %s\n",
                            connection_state_to_string(_state),
                            connection_state_to_string(new_state));
            }
            _state = new_state;
        }

        void ConnectionManager::send_hello()
        {
            auto &log = hal::get_platform().logging;

            EdgeMessage msg = build_hello(
                _config.device_id,
                _config.device_type,
                _config.firmware_version,
                _capabilities.names(),
                _capabilities.count());

            if (_transport.publish(msg))
            {
                if (log)
                    log->printf("[connection] HELLO sent (%zu capabilities)\n", _capabilities.count());
                enter_state(ConnectionState::HELLO_SENT);
            }
            else
            {
                if (log)
                    log->printf("[connection] HELLO send failed\n");
            }
        }

        void ConnectionManager::send_auth()
        {
            auto &log = hal::get_platform().logging;

            EdgeMessage msg = build_auth(_config.device_id, _config.credential);

            if (_transport.publish(msg))
            {
                if (log)
                    log->printf("[connection] AUTH sent\n");
                enter_state(ConnectionState::AUTHENTICATED);
                // Transition to READY after auth — in a real implementation,
                // we'd wait for an AUTH_RESPONSE from the Runtime
                enter_state(ConnectionState::READY);
                send_state_sync();
            }
            else
            {
                if (log)
                    log->printf("[connection] AUTH send failed\n");
            }
        }

        void ConnectionManager::send_heartbeat()
        {
            auto &platform = hal::get_platform();
            uint32_t now = platform.timer ? platform.timer->millis() : 0;
            uint32_t uptime_s = now / 1000;

            EdgeMessage msg = build_heartbeat(
                _config.device_id,
                _device_state.version(),
                uptime_s);

            if (_transport.publish(msg))
            {
                _last_heartbeat = now;
            }
        }

        void ConnectionManager::send_state_sync()
        {
            auto &platform = hal::get_platform();
            uint32_t now = platform.timer ? platform.timer->millis() : 0;

            EdgeMessage msg = _device_state.build_sync_message(_config.device_id);

            if (_transport.publish(msg))
            {
                _device_state.mark_synced();
                _last_state_sync = now;
            }
        }

        void ConnectionManager::handle_effect(const EdgeMessage &effect)
        {
            _dispatcher.dispatch(effect);
        }

        void ConnectionManager::attempt_reconnect()
        {
            auto &platform = hal::get_platform();
            uint32_t now = platform.timer ? platform.timer->millis() : 0;

            if ((now - _last_reconnect_attempt) < _reconnect_delay)
                return;
            _last_reconnect_attempt = now;

            auto &log = platform.logging;
            if (log)
                log->printf("[connection] reconnecting (delay %lu ms)\n", _reconnect_delay);

            // Transport handles its own reconnection via loop()
            if (_transport.connected())
            {
                if (log)
                    log->printf("[connection] transport reconnected\n");
                _reconnect_delay = _config.reconnect_initial_ms;

                enter_state(ConnectionState::TRANSPORT_CONNECTED);
                _idempotency.clear();
                send_hello();
                _transport.subscribe_effects();

                if (_config.credential && _config.credential[0] != '\0')
                {
                    send_auth();
                }
                else
                {
                    enter_state(ConnectionState::READY);
                    send_state_sync();
                }
            }
            else
            {
                // Exponential backoff
                _reconnect_delay = (_reconnect_delay * 2 < _config.reconnect_max_ms)
                                       ? _reconnect_delay * 2
                                       : _config.reconnect_max_ms;
            }
        }

        void ConnectionManager::effect_callback(const EdgeMessage &effect, void *user_data)
        {
            auto *self = static_cast<ConnectionManager *>(user_data);
            self->handle_effect(effect);
        }

    } // namespace edge
} // namespace voodoo
