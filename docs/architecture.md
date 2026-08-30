# Voodoo Edge — Architecture

## Overview

Voodoo Edge is a C++ SDK for connecting physical devices to the Voodoo Runtime
via the `voodoo-edge/v1` protocol. It is designed as a reusable library that
abstracts platform-specific details behind a Hardware Abstraction Layer (HAL).

## Layer Architecture

```
┌─────────────────────────────────────────────────┐
│                 Application                      │
│  (device firmware, capability handlers)          │
├─────────────────────────────────────────────────┤
│              voodoo_edge (SDK)                   │
│  ┌─────────────────────────────────────────────┐ │
│  │  Device Facade                              │ │
│  │  begin · connect · expose · emit · update   │ │
│  ├─────────────────────────────────────────────┤ │
│  │  Core (platform-independent)                │ │
│  │  protocol · transport · capabilities        │ │
│  │  effects · events · state · idempotency     │ │
│  │  connection_manager · device_identity       │ │
│  └─────────────────────────────────────────────┘ │
│  ┌─────────────────────────────────────────────┐ │
│  │  Hardware (platform-independent)            │ │
│  │  Led · Button · Motor · Servo · Relay       │ │
│  │  Buzzer · Encoder · Sensor · TempSensor     │ │
│  └─────────────────────────────────────────────┘ │
│  ┌─────────────────────────────────────────────┐ │
│  │  Platform HAL (interfaces)                  │ │
│  │  Gpio · Pwm · Adc · I2c · Spi              │ │
│  │  Wifi · Mqtt · Http · Random                │ │
│  │  Timer · Logging                            │ │
│  └─────────────────────────────────────────────┘ │
├─────────────────────────────────────────────────┤
│           Platform Implementations               │
│  ESP32: WiFi, PubSubClient, Arduino GPIO,       │
│         LEDC PWM, ADC, Wire I2C, SPI,           │
│         esp_random(), millis()                   │
│  Future: Arduino, RP2040, STM32                  │
└─────────────────────────────────────────────────┘
```

## Core Modules

| Module | Purpose |
|--------|---------|
| `device.h` | High-level facade — ties all components together |
| `device_identity.h` | Device ID, type, firmware version, metadata |
| `protocol.h` | Message types, builders, validation, serialization |
| `transport.h` | Abstract transport interface (MQTT, HTTP) |
| `capability_registry.h` | Maps capability names to handler functions |
| `effect_dispatcher.h` | Validates and routes EFFECT messages to handlers |
| `event_publisher.h` | Validates and publishes EVENT messages |
| `device_state.h` | Tracks device state with monotonic versioning |
| `idempotency.h` | Prevents duplicate effect processing |
| `connection_manager.h` | Handshake, auth, heartbeat, reconnection, state sync |

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

| Component | Purpose |
|-----------|---------|
| `LedController` | Digital LED (on/off/toggle) |
| `ButtonController` | Debounced button with press callback |
| `MotorController` | DC motor via H-bridge + PWM speed control |
| `ServoController` | Servo motor via PWM (0-180° angle) |
| `RelayController` | Relay with active-high/low support |
| `BuzzerController` | Piezo buzzer tone generation via PWM |
| `EncoderController` | Quadrature rotary encoder with position tracking |
| `SensorReader` | Analog sensor with calibration and smoothing |
| `NtcThermistor` | NTC thermistor temperature via ADC |
| `I2cTemperatureSensor` | I2C digital temperature sensor |

## Connection Lifecycle

```
Device                          Voodoo Runtime
  │                                    │
  │──── HELLO ────────────────────────►│
  │     (device_id, capabilities)      │
  │                                    │
  │──── AUTH ─────────────────────────►│
  │     (credential, if configured)    │
  │                                    │
  │──── STATE_SYNC ──────────────────►│
  │     (current device state)         │
  │                                    │
  │◄─── EFFECT (led.control) ─────────│
  │──── EFFECT_ACK ──────────────────►│
  │                                    │
  │──── EVENT (button.pressed) ──────►│
  │                                    │
  │──── HEARTBEAT ───────────────────►│
  │     (every 30s, with state version)│
  │                                    │
  │     ... reconnection on failure    │
```

## Adding a New Platform

1. Create `src/voodoo_edge/platform/<platform>/` directory
2. Implement all HAL interfaces (`Gpio`, `Pwm`, `Adc`, `I2c`, `Spi`, `Wifi`, etc.)
3. Create a platform init function that registers implementations
4. Add transport implementation (MQTT or HTTP)
5. Add platform to `library.json` platforms list

## Adding a New Capability

1. Define a handler function matching `CapabilityHandler` signature
2. Register it with `device.expose("name", handler, user_data)`
3. The effect dispatcher will route matching EFFECT messages automatically

## Adding a New Hardware Component

1. Create `src/voodoo_edge/hardware/<component>.h/cpp`
2. Use HAL interfaces (not platform-specific APIs)
3. Include in `voodoo_edge.h` single-include header
4. Add an example in `examples/`
