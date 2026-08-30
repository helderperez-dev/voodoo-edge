// device.cpp
// Voodoo Edge — Device facade implementation

#include "voodoo_edge/core/device.h"
#include "voodoo_edge/platform/hal.h"
#include <cstring>

namespace voodoo
{
    namespace edge
    {

        Device::Device()
            : _dispatcher(nullptr), _events(nullptr), _connection(nullptr), _transport(nullptr), _initialized(false), _connected(false)
        {
        }

        Device::~Device()
        {
            delete _connection;
            delete _events;
            delete _dispatcher;
        }

        void Device::begin(const char *device_id, const char *device_type)
        {
            DeviceConfig config;
            config.identity.set_device_id(device_id);
            config.identity.set_device_type(device_type);
            config.identity.set_firmware_version(VOODOO_EDGE_FIRMWARE_VERSION);
            config.credential = nullptr;
            config.heartbeat_interval_ms = 30000;
            config.state_sync_interval_ms = 60000;
            begin(config);
        }

        void Device::begin(const DeviceConfig &config)
        {
            _config = config;
            _initialized = true;

            auto &log = hal::get_platform().logging;
            if (log)
            {
                log->printf("[device] initialized: %s (type=%s, fw=%s)\n",
                            _config.identity.device_id,
                            _config.identity.device_type,
                            _config.identity.firmware_version);
            }
        }

        void Device::connect(Transport &transport)
        {
            if (!_initialized)
            {
                auto &log = hal::get_platform().logging;
                if (log)
                    log->printf("[device] error: begin() must be called before connect()\n");
                return;
            }

            _transport = &transport;

            // Create connection components
            _dispatcher = new EffectDispatcher(transport, _capabilities, _idempotency,
                                               _config.identity.device_id);
            _events = new EventPublisher(transport, _config.identity.device_id);

            ConnectionConfig conn_config;
            conn_config.device_id = _config.identity.device_id;
            conn_config.device_type = _config.identity.device_type;
            conn_config.firmware_version = _config.identity.firmware_version;
            conn_config.credential = _config.credential;
            conn_config.heartbeat_interval_ms = _config.heartbeat_interval_ms;
            conn_config.state_sync_interval_ms = _config.state_sync_interval_ms;
            conn_config.reconnect_initial_ms = 1000;
            conn_config.reconnect_max_ms = 60000;

            _connection = new ConnectionManager(
                transport, _capabilities, _state, *_dispatcher, *_events,
                _idempotency, conn_config);

            _connection->begin();
            _connected = true;
        }

        void Device::update()
        {
            if (!_connected || !_connection)
                return;
            _connection->loop();
        }

        bool Device::expose(const char *name, CapabilityHandler handler, void *user_data)
        {
            return _capabilities.register_capability(name, handler, user_data);
        }

        bool Device::emit(const char *event_name, JsonObjectConst payload)
        {
            if (!_events)
                return false;
            return _events->publish(event_name, payload);
        }

        void Device::set_state(const char *key, const char *value)
        {
            _state.set(key, value);
        }

        void Device::set_state(const char *key, int value)
        {
            _state.set(key, value);
        }

        void Device::set_state(const char *key, float value)
        {
            _state.set(key, value);
        }

        void Device::set_state(const char *key, bool value)
        {
            _state.set(key, value);
        }

        void Device::sync_state()
        {
            if (_connection)
                _connection->request_state_sync();
        }

        const DeviceIdentity &Device::identity() const
        {
            return _config.identity;
        }

        const char *Device::device_id() const
        {
            return _config.identity.device_id;
        }

        bool Device::is_ready() const
        {
            return _connection && _connection->is_ready();
        }

        ConnectionState Device::connection_state() const
        {
            if (!_connection)
                return ConnectionState::DISCONNECTED;
            return _connection->state();
        }

        ConnectionManager &Device::connection()
        {
            return *_connection;
        }

        CapabilityRegistry &Device::capabilities()
        {
            return _capabilities;
        }

        DeviceState &Device::state()
        {
            return _state;
        }

        EventPublisher &Device::events()
        {
            return *_events;
        }

    } // namespace edge
} // namespace voodoo
