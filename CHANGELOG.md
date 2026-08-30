# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [1.0.0] - 2026-08-30

### Added

#### Core
- Device facade (`device.h`) — high-level API: `begin`, `connect`, `expose`, `emit`, `set_state`, `update`
- Device identity (`device_identity.h`) — device ID, type, firmware version, hardware revision, manufacturer, name
- Connection manager (`connection_manager.h`) — handshake, auth, heartbeat, state sync, exponential backoff reconnection
- Protocol layer — message types (HELLO, AUTH, STATE_SYNC, EVENT, EFFECT, EFFECT_ACK, HEARTBEAT), builders, validation, serialization
- Capability registry — maps capability names to handler functions
- Effect dispatcher — validates and routes EFFECT messages to handlers
- Event publisher — validates and publishes EVENT messages
- Device state — tracks device state with monotonic versioning
- Idempotency cache — prevents duplicate effect processing
- Abstract transport interface

#### HAL (Hardware Abstraction Layer)
- `Gpio` — digital I/O (pinMode, digitalWrite, digitalRead)
- `Pwm` — PWM output (setup, attach, write, write_float, stop)
- `Adc` — analog-to-digital (setup, read, read_voltage)
- `I2c` — I2C bus (begin, write, read, write_read)
- `Spi` — SPI bus (begin, transfer, select, deselect)
- `Wifi` — WiFi connection management
- `Mqtt` — MQTT client abstraction
- `Http` — HTTP client abstraction
- `Random` — random number generation
- `Timer` — millisecond timer (millis, delay)
- `Logging` — serial/logging output

#### Hardware Components
- `LedController` — digital LED (on/off/toggle)
- `ButtonController` — debounced button with press callback
- `MotorController` — DC motor via H-bridge + PWM speed control
- `ServoController` — servo motor via PWM (0-180° angle)
- `RelayController` — relay with active-high/low support
- `BuzzerController` — piezo buzzer tone generation via PWM
- `EncoderController` — quadrature rotary encoder with position tracking
- `SensorReader` — analog sensor with calibration and smoothing
- `NtcThermistor` — NTC thermistor temperature via ADC (Steinhart-Hart)
- `I2cTemperatureSensor` — I2C digital temperature sensor

#### Platform: ESP32
- Full HAL implementation (GPIO, PWM via LEDC, ADC, I2C via Wire, SPI)
- MQTT transport (PubSubClient-based)
- HTTP transport (HTTPClient-based, POST + long-poll)

#### Examples
- `basic_button_led` — button press → LED toggle via Voodoo Runtime
- `motor_control` — motor speed + servo angle control
- `sensor_monitor` — temperature reading + change events
- `connected_device` — multi-capability device (LED, relay, buzzer, sensor)

#### Tooling
- PlatformIO library manifest (`library.json`)
- Arduino Library Manager manifest (`library.properties`)
- GitHub Actions CI (build + native tests)
- GitHub Actions release workflow (tag-based)
- Unit tests for protocol, capability registry, device state, idempotency
