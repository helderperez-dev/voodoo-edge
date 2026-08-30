# Voodoo Edge

[![CI](https://github.com/helderperez-dev/voodoo-edge/actions/workflows/ci.yml/badge.svg)](https://github.com/helderperez-dev/voodoo-edge/actions/workflows/ci.yml)
[![Release](https://img.shields.io/github/v/release/helderperez-dev/voodoo-edge)](https://github.com/helderperez-dev/voodoo-edge/releases)
[![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg)](LICENSE)
[![PlatformIO](https://img.shields.io/badge/PlatformIO-Registry-orange.svg)](https://registry.platformio.org/libraries/helderperez-dev/voodoo-edge)
[![Arduino](https://img.shields.io/badge/Arduino-Library_Manager-teal.svg)](#arduino-ide)

C++ SDK for connecting physical devices to the [Voodoo Runtime](https://github.com/nicegoodthings/voodoo).

## What is Voodoo Edge?

Voodoo Edge is the device connectivity layer that bridges physical hardware to the Voodoo Runtime. It provides:

- **Device Facade** — High-level API that ties everything together
- **Protocol Layer** — Message types, serialization, validation
- **Connection Manager** — Handshake, auth, heartbeat, reconnection, state sync
- **Transport Layer** — MQTT and HTTP implementations
- **Hardware Abstractions** — LED, button, motor, servo, relay, buzzer, encoder, sensors
- **Platform Support** — ESP32, with architecture for Arduino/RP2040/STM32

## Installation

### PlatformIO

Add to your `platformio.ini`:

```ini
[env:esp32dev]
platform = espressif32
board = esp32dev
framework = arduino
lib_deps =
    helderperez-dev/voodoo-edge@^1.0.0
```

Or install via CLI:

```bash
pio pkg install --library "helderperez-dev/voodoo-edge"
```

### Arduino IDE

1. Open **Sketch → Include Library → Manage Libraries**
2. Search for **voodoo-edge**
3. Click **Install**

Or install via CLI:

```bash
arduino-cli lib install "voodoo-edge"
```

### Manual / Git

```bash
# Clone into your Arduino libraries folder
cd ~/Arduino/libraries
git clone https://github.com/helderperez-dev/voodoo-edge.git

# Or into your PlatformIO project's lib folder
cd your-project/lib
git clone https://github.com/helderperez-dev/voodoo-edge.git
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
    helderperez-dev/voodoo-edge@^1.0.0
```

### 2. Use the Device facade

```cpp
#include <voodoo_edge.h>

using namespace voodoo;

edge::Device device;
hardware::LedController led(hal::get_platform().gpio, 2);

bool handle_led(const edge::EdgeMessage& effect, JsonObject result, void* ud) {
    auto* l = static_cast<hardware::LedController*>(ud);
    const char* action = effect.payload()["action"] | "";
    if (strcmp(action, "toggle") == 0) l->toggle();
    result["state"] = l->is_on();
    return true;
}

void setup() {
    platform::esp32::init();
    led.begin();

    device.begin("my-device", "esp32");
    device.expose("led.control", handle_led, &led);

    // Connect via MQTT
    platform::esp32::MqttTransport::Config cfg = { ... };
    platform::esp32::MqttTransport transport(cfg);
    device.connect(transport);
}

void loop() {
    device.update();  // handles heartbeat, state sync, effects, reconnection
}
```

## Architecture

```
┌─────────────────────────────────────────────────────────────┐
│                    Your Application                          │
│  (vacuum-former, noise-monitor, robot, etc.)                │
├─────────────────────────────────────────────────────────────┤
│                    Voodoo Edge SDK                           │
│  ┌─────────────────────────────────────────────────────┐   │
│  │  Device Facade (begin, connect, expose, emit, update)│   │
│  ├─────────────────────────────────────────────────────┤   │
│  │  Core (protocol, capabilities, effects, events)     │   │
│  │  Connection Manager (handshake, heartbeat, reconnect)│   │
│  ├─────────────────────────────────────────────────────┤   │
│  │  Hardware (LED, button, motor, servo, relay, etc.)   │   │
│  ├─────────────────────────────────────────────────────┤   │
│  │  Platform HAL (GPIO, PWM, ADC, I2C, SPI, WiFi, etc.)│   │
│  └─────────────────────────────────────────────────────┘   │
├─────────────────────────────────────────────────────────────┤
│           Platform Implementations                           │
│  ESP32: WiFi, MQTT, HTTP, GPIO, PWM, ADC, I2C, SPI         │
│  Future: Arduino, RP2040, STM32                              │
└─────────────────────────────────────────────────────────────┘
                            │
                            ▼
┌─────────────────────────────────────────────────────────────┐
│                    Voodoo Runtime                            │
│  (authoritative execution environment)                       │
└─────────────────────────────────────────────────────────────┘
```

## Project Structure

```
voodoo-edge/
├── src/
│   └── voodoo_edge/
│       ├── core/                    # Protocol logic (zero hardware deps)
│       │   ├── protocol.h/cpp       # Message types, builders, validation
│       │   ├── transport.h          # Abstract transport interface
│       │   ├── capability_registry.h/cpp
│       │   ├── effect_dispatcher.h/cpp
│       │   ├── event_publisher.h/cpp
│       │   ├── device_state.h/cpp
│       │   ├── device_identity.h
│       │   ├── idempotency.h/cpp
│       │   ├── connection_manager.h/cpp
│       │   └── device.h/cpp         # High-level Device facade
│       │
│       ├── platform/                # Hardware Abstraction Layer
│       │   ├── hal.h/cpp            # Abstract interfaces
│       │   └── esp32/               # ESP32 implementations
│       │       ├── esp32_platform.h/cpp
│       │       ├── mqtt_transport.h/cpp
│       │       └── http_transport.h/cpp
│       │
│       ├── hardware/                # Reusable hardware components
│       │   ├── led.h/cpp
│       │   ├── button.h/cpp
│       │   ├── motor.h/cpp
│       │   ├── servo.h/cpp
│       │   ├── relay.h/cpp
│       │   ├── buzzer.h/cpp
│       │   ├── encoder.h/cpp
│       │   ├── sensor.h/cpp
│       │   └── temperature_sensor.h/cpp
│       │
│       └── voodoo_edge.h           # Single-include header
│
├── examples/
│   ├── basic_button_led/            # LED + button via Voodoo Runtime
│   ├── motor_control/               # Motor + servo control
│   ├── sensor_monitor/              # Temperature sensor + events
│   └── connected_device/            # Multi-capability device
│
├── tests/                           # Unit tests (native + ESP32)
├── scripts/
│   └── bump_version.sh              # Version bump helper
├── .github/
│   └── workflows/
│       ├── ci.yml                   # CI: build + test on push/PR
│       └── release.yml              # Release: tag-based GitHub Release
├── docs/
│   └── architecture.md
├── platformio.ini
├── library.json                     # PlatformIO manifest
├── library.properties               # Arduino Library Manager manifest
├── CHANGELOG.md
└── README.md
```

## HAL Interfaces

| Interface | Purpose |
|-----------|---------|
| `Gpio` | Digital I/O (pinMode, digitalWrite, digitalRead) |
| `Pwm` | PWM output (setup, attach, write, write_float, stop) |
| `Adc` | Analog-to-digital (setup, read, read_voltage) |
| `I2c` | I2C bus (begin, write, read, write_read) |
| `Spi` | SPI bus (begin, transfer, select, deselect) |
| `Wifi` | WiFi connection management |
| `Mqtt` | MQTT client abstraction |
| `Http` | HTTP client abstraction |
| `Random` | Random number generation |
| `Timer` | Millisecond timer (millis, delay) |
| `Logging` | Serial/logging output |

## Hardware Components

| Component | Description |
|-----------|-------------|
| `LedController` | Digital LED (on/off/toggle) |
| `ButtonController` | Debounced button with press callback |
| `MotorController` | DC motor via H-bridge + PWM |
| `ServoController` | Servo motor via PWM (0-180°) |
| `RelayController` | Relay (active-high/low) |
| `BuzzerController` | Piezo buzzer via PWM tone |
| `EncoderController` | Quadrature rotary encoder |
| `SensorReader` | Analog sensor with calibration + smoothing |
| `NtcThermistor` | NTC thermistor temperature sensor |
| `I2cTemperatureSensor` | I2C digital temperature sensor |

## Examples

- [basic_button_led](examples/basic_button_led/) — Button press → LED toggle via Voodoo Runtime
- [motor_control](examples/motor_control/) — Motor speed + servo angle control
- [sensor_monitor](examples/sensor_monitor/) — Temperature reading + change events
- [connected_device](examples/connected_device/) — Multi-capability device (LED, relay, buzzer, sensor)

## Supported Platforms

| Platform | Status | Notes |
|----------|--------|-------|
| ESP32 | ✅ Full support | WiFi, MQTT, HTTP, GPIO, PWM, ADC, I2C, SPI |
| Arduino | 🔜 Planned | Ethernet, MQTT |
| RP2040 | 🔜 Planned | WiFi, MQTT |
| STM32 | 🔜 Planned | Ethernet, MQTT |

## Documentation

- [Architecture](docs/architecture.md) — Layer architecture, module overview, protocol flow
- [Changelog](CHANGELOG.md) — Version history

## Releasing

Versions follow [Semantic Versioning](https://semver.org/). The release process:

```bash
# 1. Bump version in all manifests
./scripts/bump_version.sh 1.1.0

# 2. Edit CHANGELOG.md with release notes

# 3. Commit, tag, and push
git add -A
git commit -m "chore: bump version to 1.1.0"
git tag -a v1.1.0 -m "Release v1.1.0"
git push origin main --tags
```

The [release workflow](.github/workflows/release.yml) automatically:
- Validates version consistency across manifests
- Builds all examples for ESP32
- Runs native unit tests
- Creates a GitHub Release with firmware binaries

### Distribution

| Channel | How it works |
|---------|-------------|
| **PlatformIO Registry** | Automatically indexed from `library.json` after release |
| **Arduino Library Manager** | Indexed from `library.properties` after release |
| **GitHub Releases** | Firmware binaries + source archive attached to each tag |

## License

MIT
