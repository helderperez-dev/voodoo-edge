Voodoo Edge — Embedded SDK

Architecture & Development Specification

Status: Architecture / Development Guide
Project: voodoo-edge
Primary Target: ESP32
Future Targets: Arduino, RP2040, STM32 and other embedded platforms
Parent Ecosystem: Voodoo
Core Principle: Voodoo Edge is an SDK/adapter that turns a device into a node of the Voodoo Mesh. It is not a second Voodoo Runtime.

⸻

1. Vision

Voodoo Edge is the embedded SDK responsible for connecting physical and edge devices to the Voodoo ecosystem.

Its purpose is to provide a consistent interface for:

* device identity;
* capabilities;
* hardware access;
* events;
* commands;
* state reporting;
* communication with the Voodoo Runtime;
* participation in the Voodoo Mesh.

The first implementation targets ESP32, but the architecture MUST NOT be fundamentally coupled to ESP32.

The long-term goal is to allow different platforms to become Voodoo Edge nodes:

ESP32
Arduino
RP2040
STM32
Linux
Python
Rust
Node.js
Go
...

All of them communicate with the Voodoo Runtime through the same conceptual Edge contract.

⸻

2. Core Architectural Principle

There is exactly one Voodoo Runtime.

Voodoo Edge MUST NOT recreate or duplicate the Runtime.

The architecture is:

                         V O O D O O
                             │
                      Voodoo Runtime
                             │
                       Voodoo Mesh
                             │
        ┌────────────────────┼────────────────────┐
        │                    │                    │
        ▼                    ▼                    ▼
   Voodoo Edge          Voodoo Edge          Voodoo Edge
      ESP32                 ESP32                RP2040
        │                    │                    │
     Hardware             Hardware             Hardware

The Runtime is responsible for orchestration.

The Edge is responsible for representing and operating a physical/edge node.

⸻

3. Responsibilities

3.1 Voodoo Runtime owns

The Voodoo Runtime is responsible for:

* global state;
* device registry;
* device discovery;
* mesh topology;
* routing;
* orchestration;
* workflows;
* agents;
* automation;
* permissions;
* authentication policy;
* authorization;
* persistence;
* business logic;
* cross-device coordination;
* global event processing;
* device relationships;
* capability composition;
* mesh-level configuration.

The Edge MUST NOT implement these responsibilities.

⸻

4. Responsibilities of Voodoo Edge

Voodoo Edge is responsible for:

* identifying itself;
* connecting to the Runtime;
* exposing capabilities;
* exposing hardware;
* receiving commands;
* executing commands;
* emitting events;
* reporting state;
* handling local hardware;
* maintaining communication;
* reconnecting after communication failures;
* providing local safety mechanisms;
* buffering critical messages when appropriate;
* exposing platform-specific resources through generic APIs.

The Edge should remain as small and deterministic as reasonably possible.

⸻

5. What Voodoo Edge Is NOT

Voodoo Edge is NOT:

* a second Voodoo Runtime;
* a workflow engine;
* an agent runtime;
* a database;
* a global state manager;
* a mesh orchestrator;
* a business-logic framework;
* a replacement for the Voodoo Framework;
* an ESP32-only framework;
* a collection of unrelated hardware drivers.

The Edge is the execution endpoint of the Voodoo architecture.

⸻

6. Conceptual Model

Every Voodoo Edge represents a node.

A node has:

Identity
Capabilities
State
Commands
Events
Metadata
Connection

Conceptually:

Device
├── Identity
├── Capabilities
├── State
├── Commands
├── Events
├── Metadata
└── Connection

For example:

vacuum-former-01
Capabilities:
    heater.temperature
    motor.position
    vacuum.control
    display.output
    sensor.temperature

The Runtime sees these capabilities.

It does not need to know the underlying MCU implementation.

⸻

7. Device Identity

Every Edge MUST have a unique identity.

Example:

device_id = "vacuum-former-01"

Identity should support:

* unique device ID;
* device type;
* firmware version;
* SDK version;
* hardware revision;
* manufacturer/vendor;
* capabilities;
* optional human-readable name.

Example conceptual metadata:

{
  "device_id": "vacuum-former-01",
  "device_type": "vacuum-former",
  "firmware_version": "0.1.0",
  "edge_version": "0.1.0",
  "hardware_revision": "A1"
}

The identity system must be platform-independent.

⸻

8. Capabilities

Capabilities are one of the most important concepts in Voodoo Edge.

The Runtime should interact with capabilities, not implementation details.

Example:

temperature.read
temperature.control
motor.position
motor.velocity
vacuum.control
display.output
gpio.read
gpio.write

The Runtime should not need to know:

ESP32 GPIO 18
NEMA motor
I2C address 0x3C
SSD1306

Those are implementation details of the Edge.

Instead:

axis_y
    capability: motor.position

The Edge translates the generic operation into the correct hardware operation.

⸻

9. Hardware Abstraction

Hardware APIs MUST be separated from the platform implementation.

Example:

Motor motor;
Display display;
Button button;
Sensor sensor;

The application should not need:

ESP32Motor
ESP32Display
ESP32Button

Platform-specific implementation belongs underneath the abstraction.

Conceptually:

Application
     │
     ▼
Voodoo Edge API
     │
     ▼
Hardware Abstraction Layer
     │
     ├── ESP32 implementation
     ├── Arduino implementation
     ├── RP2040 implementation
     └── STM32 implementation

⸻

10. Platform Independence

The core API MUST NOT depend directly on ESP32 APIs.

Avoid:

#include <WiFi.h>

inside generic core classes.

Instead:

core/
hal/
platform/

Platform-specific dependencies belong inside platform implementations.

For example:

platform/
└── esp32/
    ├── Esp32GPIO
    ├── Esp32Timer
    ├── Esp32Network
    └── Esp32Storage

Future:

platform/
├── esp32/
├── arduino/
├── rp2040/
└── stm32/

⸻

11. Voodoo Edge Layers

The architecture should be organized into clear layers.

┌─────────────────────────────────────┐
│           Application               │
│  Vacuum Former / Robot / Sensor     │
├─────────────────────────────────────┤
│          Voodoo Edge API            │
│ Device / Components / Capabilities  │
├─────────────────────────────────────┤
│              HAL                    │
│ GPIO / I2C / SPI / PWM / Timer      │
├─────────────────────────────────────┤
│          Platform Layer             │
│ ESP32 / RP2040 / STM32 / Arduino    │
├─────────────────────────────────────┤
│            Hardware                │
└─────────────────────────────────────┘

Communication is a separate concern:

Application
     │
Voodoo Edge
     │
Edge Protocol
     │
Transport
     │
Network
     │
Voodoo Runtime

⸻

12. Core Package

The core package should contain only platform-independent concepts.

Potential modules:

core/
├── Device
├── Identity
├── Capability
├── State
├── Event
├── Command
├── Metadata
├── Configuration
└── Error

The core MUST NOT depend on:

* ESP32;
* Wi-Fi;
* FreeRTOS;
* Arduino APIs;
* MQTT;
* HTTP;
* specific displays;
* specific sensors.

⸻

13. HAL

The Hardware Abstraction Layer provides generic access to hardware primitives.

Initial interfaces may include:

GPIO
PWM
ADC
I2C
SPI
UART
Timer
Storage
Interrupts

Example:

gpio.write(true);
gpio.read();
pwm.write(128);
i2c.write(...);

Platform-specific implementations provide the actual behavior.

⸻

14. Components

Voodoo Edge may provide reusable hardware components.

Initial candidates:

Button
Encoder
LED
Buzzer
Relay
Motor
Servo
Display
Sensor
TemperatureSensor

Components should be built on top of the HAL whenever possible.

Example:

Motor
  ↓
PWM
GPIO
Timer

The component should not unnecessarily expose platform-specific details.

⸻

15. Communication Layer

Communication with the Voodoo Runtime MUST be separated from hardware.

Conceptually:

Device
  │
  ├── Hardware
  │
  └── Communication
          │
          ▼
     Edge Protocol
          │
          ▼
       Runtime

The transport should be replaceable.

Possible transports:

Wi-Fi
Ethernet
Serial
USB
Bluetooth
BLE
WebSocket
TCP
MQTT
Other future transports

Do not make the application dependent on a specific transport.

⸻

16. Edge Protocol

The Edge Protocol defines how an Edge communicates with the Runtime.

The protocol should support at minimum:

Handshake
Authentication
Registration
Capability advertisement
Command
Command response
Event
State update
Error
Heartbeat
Disconnect
Reconnect

Conceptual flow:

EDGE → RUNTIME
    handshake
RUNTIME → EDGE
    registration accepted
EDGE → RUNTIME
    capabilities
EDGE → RUNTIME
    state
EDGE → RUNTIME
    event
RUNTIME → EDGE
    command
EDGE → RUNTIME
    command result

⸻

17. Protocol Example

An event could conceptually look like:

{
  "type": "event",
  "device": "vacuum-former-01",
  "capability": "temperature",
  "event": "changed",
  "value": 178.4
}

A command:

{
  "type": "command",
  "device": "vacuum-former-01",
  "capability": "motor.position",
  "action": "move",
  "parameters": {
    "position": 120
  }
}

These structures are conceptual.

The final wire protocol must be designed independently of any one implementation language.

⸻

18. Events

Events represent something that happened on the Edge.

Examples:

button.pressed
button.released
temperature.changed
motor.position_changed
sensor.triggered
device.connected
device.disconnected
error.occurred

Events should be lightweight.

The Edge emits them.

The Runtime decides what they mean globally.

Example:

Temperature Edge
       │
       │ temperature.changed
       ▼
Voodoo Runtime
       │
       │ command
       ▼
Heater Edge

The Temperature Edge does not need to know the Heater Edge exists.

⸻

19. Commands

Commands represent instructions sent by the Runtime.

Examples:

motor.move
motor.stop
relay.on
relay.off
heater.set_temperature
display.show
gpio.write

The Edge validates and executes the command.

The Runtime remains responsible for deciding when and why the command should happen.

⸻

20. State

The Edge should report relevant local state.

Examples:

temperature = 178.4
motor.position = 120
vacuum = true
heater = 180

State should not automatically become a global database inside the Edge.

The Runtime owns global state.

The Edge maintains only the local state necessary to operate correctly and report status.

⸻

21. Local Execution

Some operations MUST remain local.

Examples:

Emergency stop
Motor limits
Over-temperature protection
Hardware watchdog
Safety interlocks
Critical timing
PWM generation
Sensor sampling

The Runtime should not need to control every microsecond-level operation.

Example:

Runtime
   │
   │ "Move axis to 120mm"
   ▼
Edge
   │
   ├── validate limits
   ├── execute movement
   ├── monitor position
   └── stop at target

This is not creating a second Runtime.

It is local hardware execution.

⸻

22. Mesh Philosophy

The Voodoo Mesh is logically coordinated by the Voodoo Runtime.

Edges should not require knowledge of the entire mesh.

An Edge knows:

"I am device X."
"I expose capabilities Y."
"I can execute commands Z."
"I can emit events A."

The Runtime knows:

"What devices exist."
"What capabilities exist."
"How devices relate."
"What should happen."
"Which device should receive a command."

⸻

23. Direct Edge-to-Edge Communication

Direct Edge-to-Edge communication MAY be supported in the future for:

* low-latency operations;
* local safety;
* real-time control;
* offline operation;
* local device coordination.

However, this MUST NOT cause each Edge to become a mesh orchestrator.

The Runtime remains the authority for global topology and orchestration.

Conceptually:

Normal:
Edge A → Runtime → Edge B
Optional low-latency:
Edge A ─────────→ Edge B
       direct

The direct protocol should still follow the same Voodoo device/capability model.

⸻

24. Offline Operation

An Edge should be able to operate in a limited standalone mode.

For example:

Network unavailable
       │
       ▼
Edge continues local hardware operation

However, offline behavior must be explicitly defined by the application.

The Edge should not silently invent global behavior.

Example:

Vacuum Former
Local:
    emergency stop
    motor limits
    heater safety
    temperature monitoring
Runtime:
    production workflow
    UI
    scheduling
    automation
    remote commands

⸻

25. Reliability

The Edge communication layer should support:

connection timeout
heartbeat
reconnect
backoff
message validation
command timeout
duplicate detection where necessary
graceful disconnect

The implementation should be appropriate for constrained embedded devices.

Avoid unnecessary allocations and heavyweight abstractions.

⸻

26. Memory and Performance

Embedded resources are limited.

The SDK should prioritize:

* predictable memory usage;
* minimal dynamic allocation;
* low CPU overhead;
* deterministic behavior;
* small dependencies;
* compile-time configuration where useful;
* modular components.

Do not import large dependencies for functionality that can be implemented efficiently in the SDK.

⸻

27. ESP32 Implementation

ESP32 is the first supported platform.

ESP32-specific code belongs in:

platform/esp32/

Examples:

Wi-Fi
NVS
OTA
FreeRTOS integration
ESP-specific timers
ESP-specific networking

The generic application API should not need to know these details.

⸻

28. Arduino Compatibility

Arduino support is a future platform target.

The architecture should make it possible to support:

Arduino Uno
Arduino Nano
Arduino Mega
Arduino-compatible boards

However, implementation should NOT be forced prematurely.

First make the abstraction correct.

Then add platform implementations where feasible.

⸻

29. Future Platforms

Potential future targets:

ESP32
Arduino
RP2040
STM32
nRF52
Linux

The platform matrix should be driven by actual use cases.

Do not build speculative abstractions merely to claim compatibility.

⸻

30. Language SDKs

The concept of Edge should not be restricted to C++ or microcontrollers.

The Voodoo ecosystem may eventually provide:

voodoo-edge-esp32
voodoo-edge-python
voodoo-edge-rust
voodoo-edge-node
voodoo-edge-go

These implementations represent the same logical Edge contract.

For example:

                 Voodoo Runtime
                       │
                  Voodoo Mesh
                       │
        ┌──────────────┼──────────────┐
        │              │              │
   ESP32 Edge      Python Edge      Rust Edge
        │              │              │
    Hardware        AI/Vision       Service

The important abstraction is the Edge Protocol and device model, not the programming language.

⸻

31. Python/Rust/Node Edges

A non-embedded Edge may represent:

* a computer;
* an AI service;
* a camera;
* a local process;
* a browser automation process;
* a robotics controller;
* a local daemon;
* another physical interface.

For example:

Python Edge
    └── Camera capability
ESP32 Edge
    └── Motor capability
Runtime
    └── orchestrates both

The Runtime sees them as nodes with capabilities.

⸻

32. Repository Architecture

Recommended initial repository structure:

voodoo-edge/
│
├── src/
│   └── voodoo_edge/
│       ├── core/
│       │   ├── device/
│       │   ├── identity/
│       │   ├── capability/
│       │   ├── command/
│       │   ├── event/
│       │   └── state/
│       │
│       ├── hal/
│       │   ├── gpio/
│       │   ├── pwm/
│       │   ├── i2c/
│       │   ├── spi/
│       │   ├── serial/
│       │   ├── timer/
│       │   └── storage/
│       │
│       ├── components/
│       │   ├── button/
│       │   ├── encoder/
│       │   ├── motor/
│       │   ├── display/
│       │   ├── sensor/
│       │   ├── relay/
│       │   └── buzzer/
│       │
│       ├── protocol/
│       │   ├── messages/
│       │   ├── serialization/
│       │   └── validation/
│       │
│       ├── transport/
│       │   ├── wifi/
│       │   ├── ethernet/
│       │   ├── serial/
│       │   └── websocket/
│       │
│       ├── platform/
│       │   └── esp32/
│       │
│       └── VoodooEdge.h
│
├── examples/
│   ├── basic-device/
│   ├── button/
│   ├── motor/
│   ├── display/
│   ├── sensor/
│   └── connected-device/
│
├── tests/
│   ├── core/
│   ├── protocol/
│   └── components/
│
├── docs/
│   ├── architecture.md
│   ├── protocol.md
│   ├── platforms.md
│   └── getting-started.md
│
├── library.json
├── platformio.ini
├── README.md
└── LICENSE

The exact physical directory layout may evolve, but the architectural separation MUST remain.

⸻

33. Product/Application Repositories

Individual products should live outside voodoo-edge.

For example:

voodoo-edge
vacuum-former
noise-monitor
creator-controller
robot

A product imports the SDK:

#include <VoodooEdge.h>

The product repository contains only application-specific behavior.

Example:

vacuum-former/
├── src/
│   ├── main.cpp
│   ├── heater_controller.cpp
│   ├── vacuum_controller.cpp
│   ├── axis_controller.cpp
│   └── ui.cpp
├── platformio.ini
└── README.md

⸻

34. Example: Vacuum Former

The Vacuum Former should consume Voodoo Edge.

Voodoo Edge provides:

Motor
Relay
Temperature Sensor
Display
Button
Network
Device
Events
Commands

The Vacuum Former provides:

Heating cycle
Vacuum cycle
Axis movement
Temperature targets
Safety logic
User interface
Machine state

Do NOT place Vacuum Former logic into the generic SDK.

⸻

35. Example: Noise Monitor

A Noise Monitor might use:

#include <VoodooEdge.h>

and expose:

microphone
temperature
display
network

The Voodoo Runtime can consume:

sound.level.changed

without knowing which microphone or ESP32 board produced it.

⸻

36. Example: Robot

A robot might expose:

motor.left
motor.right
encoder.left
encoder.right
camera
battery

The Runtime could coordinate the robot without needing to understand its low-level GPIO mapping.

⸻

37. API Design Principles

The public API should be:

* simple;
* discoverable;
* consistent;
* platform-independent;
* composable;
* lightweight;
* strongly typed where practical;
* safe by default.

Prefer:

device.motor("axis_y")

over exposing implementation details.

Prefer:

device.expose(...)

over hardcoding Runtime-specific behavior into components.

⸻

38. Dependency Principles

Voodoo Edge should minimize dependencies.

Core should ideally have no external dependencies.

Platform implementations may use:

ESP-IDF
Arduino Core
PlatformIO

depending on the selected implementation strategy.

Optional features should remain optional.

For example:

Core
  no Wi-Fi dependency
Wi-Fi support
  optional
MQTT
  optional
Display driver
  optional

⸻

39. Security

The Edge must support secure communication with the Runtime.

Future/required capabilities may include:

device identity
authentication
TLS
credentials
key rotation
secure provisioning
authorization metadata

The Edge should not independently define global authorization rules.

The Runtime remains the authority.

⸻

40. Provisioning

An Edge should have a provisioning mechanism.

Potential process:

Factory
   │
   ▼
Device created
   │
   ▼
Provisioning
   │
   ▼
Device identity
   │
   ▼
Network configuration
   │
   ▼
Connect to Runtime
   │
   ▼
Register
   │
   ▼
Mesh

The exact provisioning mechanism should be implemented separately from the core device model.

⸻

41. Firmware Updates

OTA may be implemented for capable platforms.

OTA belongs to the platform/Edge infrastructure layer.

The Runtime may instruct:

"Update device firmware"

but the Edge handles the actual platform-specific update process.

The Runtime should not contain ESP32 flashing logic.

⸻

42. Logging

Provide a lightweight logging abstraction:

log.info(...)
log.warn(...)
log.error(...)
log.debug(...)

The backend may vary by platform:

Serial
USB
UART
ESP logging
file
network

Logging should be configurable.

⸻

43. Configuration

Configuration should distinguish:

Device configuration
Application configuration
Runtime configuration
Platform configuration

Do not mix all configuration into one global object.

Example:

Device:
    device_id
Network:
    Wi-Fi credentials
Runtime:
    Runtime endpoint
Application:
    heater limits
Platform:
    GPIO mapping

⸻

44. Testing Strategy

Tests should be layered.

Core tests

Test without hardware:

Identity
Capability
Command
Event
State
Serialization
Validation

Component tests

Test:

Motor
Button
Sensor
Display

using mocks where possible.

Platform tests

Run on real hardware:

ESP32
GPIO
I2C
SPI
PWM
Wi-Fi

Integration tests

Test:

Edge ↔ Runtime

including:

registration
capabilities
events
commands
state
reconnection

⸻

45. Examples Are Part of the SDK

Examples are not optional documentation.

The repository should provide minimal examples such as:

01-basic-device
02-button
03-gpio
04-motor
05-display
06-sensor
07-connected-device
08-events
09-commands

Each example should be independently understandable.

⸻

46. Versioning

Use semantic versioning:

MAJOR.MINOR.PATCH

Example:

0.1.0
0.2.0
1.0.0

Breaking changes to the public Edge API or protocol should be explicitly versioned.

Protocol version should be independent from SDK version.

Example:

Edge SDK: 0.4.0
Protocol: 1

⸻

47. Compatibility

The protocol should support backward compatibility where practical.

A Runtime should be able to identify:

Edge SDK version
Protocol version
Device firmware version
Hardware revision

This is important because a mesh can contain devices running different firmware versions.

⸻

48. Discovery

Device discovery belongs primarily to the Runtime/Mesh layer.

The Edge participates by providing identity and registration information.

Do not make every Edge maintain a complete copy of the device registry.

Example:

Edge starts
    │
    ▼
Connect
    │
    ▼
Handshake
    │
    ▼
Register
    │
    ▼
Advertise capabilities
    │
    ▼
Runtime adds device to Mesh

⸻

49. Capability Discovery

A device should expose its capabilities during registration.

Example:

{
  "device_id": "creator-controller-01",
  "capabilities": [
    "button.press",
    "encoder.value",
    "display.output"
  ]
}

The Runtime can then understand what the device can do.

⸻

50. No Product-Specific Assumptions

Voodoo Edge MUST NOT contain assumptions such as:

Vacuum Former
Noise Monitor
Robot
Creator Controller

unless they are represented as examples/tests.

The SDK should never require:

if (deviceType == "vacuum-former")

inside generic infrastructure.

⸻

51. No Runtime Duplication

Avoid adding features such as:

Workflow engine
Agent planner
Global scheduler
Mesh database
Global state manager
Cross-device orchestration engine

These belong to Voodoo Runtime.

If functionality seems to require one of these, first ask:

Should this actually be implemented in the Runtime?

Usually the answer is yes.

⸻

52. Edge as a Thin Runtime Boundary

The ideal Edge should feel like:

Physical world
      │
      ▼
Voodoo Edge
      │
      ▼
Voodoo Protocol
      │
      ▼
Voodoo Runtime

Not:

Physical world
      │
      ▼
Mini Voodoo Runtime
      │
      ▼
Another Runtime

This distinction must be preserved throughout development.

⸻

53. Development Priorities

Development should proceed in this order.

Phase 1 — Foundation

Implement:

Device
Identity
Capability
Event
Command
State

without hardware dependencies.

⸻

Phase 2 — HAL

Implement:

GPIO
PWM
I2C
SPI
Serial
Timer

for ESP32.

⸻

Phase 3 — Components

Implement reusable:

Button
LED
Relay
Motor
Display
Sensor
Encoder

⸻

Phase 4 — Protocol

Implement:

Handshake
Registration
Capability advertisement
Command
Command response
Event
State
Heartbeat
Reconnect

⸻

Phase 5 — Runtime Integration

Connect a real ESP32 to the Voodoo Runtime.

The device should:

connect
register
advertise capabilities
receive commands
execute commands
emit events
report state
recover from disconnection

⸻

Phase 6 — Real Product

Use the SDK to build:

Vacuum Former

without adding product-specific code to Voodoo Edge.

⸻

Phase 7 — Additional Platforms

Only after the abstraction is validated:

RP2040
Arduino
STM32

⸻

Phase 8 — Additional Language SDKs

If useful:

Python Edge
Rust Edge
Node Edge
Go Edge

all implementing the same logical Edge contract/protocol.

⸻

54. Definition of Done

Voodoo Edge v1 should be considered successful when a developer can:

1. Install Voodoo Edge.
2. Select an ESP32 board.
3. Include the SDK.
4. Create a device.
5. Give it an identity.
6. Expose hardware capabilities.
7. Connect to a Voodoo Runtime.
8. Register automatically.
9. See the device in the Voodoo Mesh.
10. Receive a command from the Runtime.
11. Execute it on hardware.
12. Emit an event.
13. Have the Runtime receive the event.
14. Report state.
15. Recover from temporary network failures.

Example:

#include <VoodooEdge.h>
VoodooEdge device;
void setup() {
    device.begin("example-device");
    device.expose("led", "gpio.output");
    device.expose("button", "button.input");
    device.connect();
}
void loop() {
    device.update();
}

The exact API may change during implementation, but the architectural behavior should remain.

⸻

55. Final Architecture

The complete Voodoo architecture should ultimately look like:

                         V O O D O O
                              │
                     ┌────────┴────────┐
                     │ Voodoo Runtime  │
                     │                 │
                     │ State           │
                     │ Agents          │
                     │ Workflows       │
                     │ Routing         │
                     │ Permissions     │
                     │ Mesh            │
                     │ Persistence     │
                     └────────┬────────┘
                              │
                       Voodoo Protocol
                              │
                       Voodoo Mesh
                              │
          ┌───────────────────┼───────────────────┐
          │                   │                   │
          ▼                   ▼                   ▼
   Voodoo Edge          Voodoo Edge          Voodoo Edge
      ESP32                Python               Rust
          │                   │                   │
       Hardware            Camera              Service
          │
    ┌─────┼─────┐
    ▼     ▼     ▼
  Motor Sensor Display

The central architectural rule is:

The Runtime orchestrates. The Edge executes.

And the ecosystem rule is:

Any platform capable of implementing the Voodoo Edge contract can become a node in the Voodoo Mesh.

The hardware, programming language, MCU, operating system, and implementation details are secondary.

The Voodoo Runtime sees:

Identity
Capabilities
State
Events
Commands

That is the stable contract.

⸻

56. Guiding Principle

When deciding whether a feature belongs in Voodoo Edge, ask:

Does it make a platform capable of becoming a Voodoo Mesh node?

If yes, it probably belongs in Edge.

Does it coordinate multiple devices or represent global application logic?

It probably belongs in Voodoo Runtime.

Does it implement a physical product’s behavior?

It belongs in the product/application firmware.

This separation should remain sacred:

Voodoo Runtime
    = brain / orchestrator
Voodoo Edge
    = bridge + local execution layer
Product Firmware
    = application behavior
Hardware
    = physical world

⸻

57. Long-Term Vision

Voodoo Edge should eventually make it possible to connect:

Sensors
Motors
Robots
Machines
Cameras
Displays
Microcontrollers
AI systems
Computers
Local services
Applications

into one coherent Voodoo Mesh.

A physical device should become another first-class node in Voodoo.

The ultimate abstraction is:

                 Voodoo Mesh
                      │
       ┌──────────────┼──────────────┐
       │              │              │
    Software       Hardware       AI/Services
       │              │              │
       └──────────────┼──────────────┘
                      │
                Voodoo Runtime

Voodoo Edge is the boundary that makes the physical and external world part of that architecture.