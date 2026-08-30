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
│  │  Core (platform-independent)                │ │
│  │  protocol · transport · capabilities        │ │
│  │  effects · events · state · idempotency     │ │
│  └─────────────────────────────────────────────┘ │
│  ┌─────────────────────────────────────────────┐ │
│  │  Hardware (platform-independent)            │ │
│  │  LedController · ButtonController           │ │
│  └─────────────────────────────────────────────┘ │
│  ┌─────────────────────────────────────────────┐ │
│  │  Platform HAL (interfaces)                  │ │
│  │  Gpio · Wifi · Mqtt · Http · Random         │ │
│  │  Timer · Logging                            │ │
│  └─────────────────────────────────────────────┘ │
├─────────────────────────────────────────────────┤
│           Platform Implementations               │
│  ESP32: WiFi, PubSubClient, Arduino GPIO,       │
│         esp_random(), millis()                   │
│  Future: Arduino, RP2040, STM32                  │
└─────────────────────────────────────────────────┘
```

## Core Modules

| Module | Purpose |
|--------|---------|
| `protocol.h` | Message types, builders, validation, serialization |
| `transport.h` | Abstract transport interface (MQTT, HTTP) |
| `capability_registry.h` | Maps capability names to handler functions |
| `effect_dispatcher.h` | Validates and routes EFFECT messages to handlers |
| `event_publisher.h` | Validates and publishes EVENT messages |
| `device_state.h` | Tracks device state with monotonic versioning |
| `idempotency.h` | Prevents duplicate effect processing |

## HAL Interfaces

| Interface | Purpose |
|-----------|---------|
| `Gpio` | Digital I/O (pinMode, digitalWrite, digitalRead) |
| `Wifi` | WiFi connection management |
| `Mqtt` | MQTT client abstraction |
| `Http` | HTTP client abstraction |
| `Random` | Random number generation |
| `Timer` | Millisecond timer (millis, delay) |
| `Logging` | Serial/logging output |

## Protocol Flow

```
Device                          Voodoo Runtime
  │                                    │
  │──── HELLO ────────────────────────►│
  │◄─── AUTH_REQUEST ─────────────────│
  │──── AUTH ─────────────────────────►│
  │◄─── AUTH_RESPONSE ────────────────│
  │                                    │
  │──── STATE_SYNC ──────────────────►│
  │──── EVENT (button.pressed) ──────►│
  │                                    │
  │◄─── EFFECT (led.control) ─────────│
  │──── EFFECT_ACK ──────────────────►│
  │                                    │
  │──── HEARTBEAT ───────────────────►│
  │     (every 30s)                    │
```

## Adding a New Platform

1. Create `src/voodoo_edge/platform/<platform>/` directory
2. Implement all HAL interfaces (`Gpio`, `Wifi`, `Mqtt`, etc.)
3. Create a platform init function that registers implementations
4. Add transport implementation (MQTT or HTTP)
5. Add platform to `library.json` platforms list

## Adding a New Capability

1. Define a handler function matching `CapabilityHandler` signature
2. Register it with `CapabilityRegistry::register_capability()`
3. The effect dispatcher will route matching EFFECT messages automatically
