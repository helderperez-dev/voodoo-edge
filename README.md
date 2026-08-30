# Voodoo Edge

Reusable SDK for connecting physical devices to the [Voodoo Runtime](https://github.com/nicegoodthings/voodoo).

## What is Voodoo Edge?

Voodoo Edge is the device connectivity layer that bridges physical hardware to the Voodoo Runtime. It provides:

- **Protocol Layer** — Message types, serialization, validation
- **Transport Layer** — MQTT and HTTP implementations
- **Hardware Abstractions** — GPIO, LED, button, and more
- **Platform Support** — ESP32, with architecture for Arduino/RP2040/STM32

## Architecture

```
┌─────────────────────────────────────────────────────────────┐
│                    Your Application                          │
│  (vacuum-former, noise-monitor, robot, etc.)                │
└─────────────────────────────────────────────────────────────┘
                            │
                            ▼
┌─────────────────────────────────────────────────────────────┐
│                    Voodoo Edge SDK                           │
│  ┌─────────────────────────────────────────────────────┐   │
│  │  Core (protocol, capabilities, effects, events)     │   │
│  └─────────────────────────────────────────────────────┘   │
│  ┌─────────────────────────────────────────────────────┐   │
│  │  Hardware (LED, button, motor, sensor, display)      │   │
│  └─────────────────────────────────────────────────────┘   │
│  ┌─────────────────────────────────────────────────────┐   │
│  │  Platform (ESP32, Arduino, RP2040, STM32)           │   │
│  └─────────────────────────────────────────────────────┘   │
└─────────────────────────────────────────────────────────────┘
                            │
                            ▼
┌─────────────────────────────────────────────────────────────┐
│                    Voodoo Runtime                            │
│  (authoritative execution environment)                       │
└─────────────────────────────────────────────────────────────┘
```

## Quick Start

### 1. Add to your PlatformIO project

```ini
; platformio.ini
[env:esp32dev]
platform = espressif32
board = esp32dev
framework = arduino
lib_deps =
    voodoo-edge
```

### 2. Use in your code

```cpp
#include <voodoo_edge.h>

using namespace voodoo::edge;
using namespace voodoo::hardware;

// Create components
DeviceIdentity identity;
MqttTransport transport(identity);
ConnectionManager connection(identity, transport);
CapabilityRegistry capabilities;
EffectDispatcher effects(identity, transport, capabilities);
EventPublisher events(identity, transport);
LedController led(PIN_LED);

// Handle LED effect
bool handle_led(const EdgeMessage& msg, JsonDocument& result, void* ctx) {
    auto params = msg.payload()["parameters"];
    bool state = params["state"] | false;
    led.set(state);
    result["led"] = state;
    return true;
}

void setup() {
    Serial.begin(115200);
    
    // Configure
    DeviceConfig config;
    strncpy(config.wifi_ssid, "your-wifi", sizeof(config.wifi_ssid));
    strncpy(config.wifi_password, "your-pass", sizeof(config.wifi_password));
    strncpy(config.mqtt_host, "192.168.1.100", sizeof(config.mqtt_host));
    
    // Initialize
    identity.begin(config);
    transport.begin();
    connection.begin();
    
    // Register capabilities
    capabilities.register_capability("led.control", handle_led);
}

void loop() {
    connection.loop();
    effects.loop();
}
```

## Project Structure

```
voodoo-edge/
├── src/
│   └── voodoo_edge/
│       ├── core/                    # Protocol logic (zero hardware deps)
│       │   ├── protocol.h/cpp
│       │   ├── transport.h
│       │   ├── capability_registry.h/cpp
│       │   ├── effect_dispatcher.h/cpp
│       │   ├── event_publisher.h/cpp
│       │   ├── device_state.h/cpp
│       │   └── idempotency.h/cpp
│       │
│       ├── platform/                # Hardware Abstraction Layer
│       │   ├── hal.h               # Abstract interfaces
│       │   └── esp32/              # ESP32 implementations
│       │       ├── wifi.h/cpp
│       │       ├── mqtt.h/cpp
│       │       ├── http.h/cpp
│       │       ├── gpio.h/cpp
│       │       └── logging.h/cpp
│       │
│       ├── hardware/                # Reusable hardware components
│       │   ├── led.h/cpp
│       │   └── button.h/cpp
│       │
│       └── voodoo_edge.h           # Single-include header
│
├── examples/
│   └── basic_button_led/
│
├── tests/
│   └── test_protocol/
│
├── tools/
│   └── edge_simulator.py
│
├── platformio.ini
├── library.json
└── README.md
```

## Examples

- [basic_button_led](examples/basic_button_led/) — Button press → LED toggle via Voodoo Runtime

## Supported Platforms

| Platform | Status | Notes |
|----------|--------|-------|
| ESP32 | ✅ Full support | WiFi, MQTT, HTTP, GPIO |
| Arduino | 🔜 Planned | Ethernet, MQTT |
| RP2040 | 🔜 Planned | WiFi, MQTT |
| STM32 | 🔜 Planned | Ethernet, MQTT |

## Documentation

- [Protocol Reference](docs/PROTOCOL.md) — Message types, MQTT topics, HTTP endpoints
- [Hardware Guide](docs/HARDWARE.md) — Supported hardware, wiring, configuration
- [API Reference](docs/API.md) — Class and function documentation

## License

MIT
