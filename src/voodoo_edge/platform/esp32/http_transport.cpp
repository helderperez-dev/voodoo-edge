// http_transport.cpp
// Voodoo Edge — HTTP transport implementation

#include "voodoo_edge/platform/esp32/http_transport.h"
#include "voodoo_edge/platform/hal.h"
#include <cstring>
#include <cstdio>

namespace voodoo
{
    namespace platform
    {
        namespace esp32
        {

            HttpTransport::HttpTransport(const Config &config)
                : _config(config), _effect_callback(nullptr), _effect_user_data(nullptr), _last_poll(0), _connected(false)
            {
            }

            bool HttpTransport::begin()
            {
                auto &log = hal::get_platform().logging;
                if (log)
                    log->printf("[http] connecting to %s\n", _config.base_url);

                // Test connectivity with a ping
                HTTPClient http;
                char url[256];
                snprintf(url, sizeof(url), "%s/api/v1/ping", _config.base_url);

                http.begin(url);
                http.setTimeout(_config.timeout_ms);
                if (_config.api_key)
                {
                    http.addHeader("Authorization", _config.api_key);
                }

                int code = http.GET();
                http.end();

                _connected = (code == 200);
                if (log)
                {
                    log->printf("[http] ping result: %d (%s)\n",
                                code, _connected ? "connected" : "failed");
                }
                return _connected;
            }

            void HttpTransport::end()
            {
                _connected = false;
            }

            bool HttpTransport::connected()
            {
                return _connected;
            }

            void HttpTransport::loop()
            {
                if (!_connected)
                    return;

                auto &platform = hal::get_platform();
                uint32_t now = platform.timer ? platform.timer->millis() : 0;

                if ((now - _last_poll) >= _config.poll_interval_ms)
                {
                    _last_poll = now;
                    poll_effects();
                }
            }

            bool HttpTransport::publish(const edge::EdgeMessage &msg)
            {
                if (!_connected)
                    return false;

                char buffer[MAX_MESSAGE_BUFFER_SIZE];
                size_t len = msg.serialize(buffer, sizeof(buffer));
                if (len == 0)
                    return false;

                // Determine endpoint based on message type
                const char *endpoint = "events";
                switch (msg.type)
                {
                case edge::MessageType::HELLO:
                    endpoint = "hello";
                    break;
                case edge::MessageType::AUTH:
                    endpoint = "auth";
                    break;
                case edge::MessageType::STATE_SYNC:
                    endpoint = "state";
                    break;
                case edge::MessageType::EVENT:
                    endpoint = "events";
                    break;
                case edge::MessageType::EFFECT_ACK:
                    endpoint = "ack";
                    break;
                case edge::MessageType::HEARTBEAT:
                    endpoint = "heartbeat";
                    break;
                default:
                    break;
                }

                return post_message(endpoint, buffer, len);
            }

            bool HttpTransport::subscribe_effects()
            {
                // HTTP doesn't have a persistent subscription model.
                // Effects are polled via loop().
                return true;
            }

            void HttpTransport::on_effect(edge::EffectCallback callback, void *user_data)
            {
                _effect_callback = callback;
                _effect_user_data = user_data;
            }

            bool HttpTransport::post_message(const char *endpoint, const char *json, size_t len)
            {
                char url[256];
                snprintf(url, sizeof(url), "%s/api/v1/devices/%s/%s",
                         _config.base_url, _config.device_id, endpoint);

                HTTPClient http;
                http.begin(url);
                http.addHeader("Content-Type", "application/json");
                http.setTimeout(_config.timeout_ms);
                if (_config.api_key)
                {
                    http.addHeader("Authorization", _config.api_key);
                }

                int code = http.POST(reinterpret_cast<uint8_t *>(const_cast<char *>(json)), len);
                http.end();

                auto &log = hal::get_platform().logging;
                if (log && code < 0)
                {
                    log->printf("[http] POST %s failed: %d\n", endpoint, code);
                }

                return code >= 200 && code < 300;
            }

            void HttpTransport::poll_effects()
            {
                if (!_effect_callback)
                    return;

                char url[256];
                snprintf(url, sizeof(url), "%s/api/v1/devices/%s/effects/poll",
                         _config.base_url, _config.device_id);

                HTTPClient http;
                http.begin(url);
                http.setTimeout(_config.timeout_ms);
                if (_config.api_key)
                {
                    http.addHeader("Authorization", _config.api_key);
                }

                int code = http.GET();
                if (code == 200)
                {
                    String payload = http.getString();
                    if (payload.length() > 0)
                    {
                        edge::EdgeMessage msg;
                        if (msg.deserialize(payload.c_str(), payload.length()))
                        {
                            if (msg.type == edge::MessageType::EFFECT)
                            {
                                _effect_callback(msg, _effect_user_data);
                            }
                        }
                    }
                }
                http.end();
            }

        } // namespace esp32
    } // namespace platform
} // namespace voodoo
