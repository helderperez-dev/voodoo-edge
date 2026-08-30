// voodoo_edge.h
// Voodoo Edge — Single-include header for the entire SDK
//
// Usage:
//   #include <voodoo_edge.h>

#ifndef VODOO_EDGE_H
#define VODOO_EDGE_H

// Core protocol
#include "voodoo_edge/core/protocol.h"
#include "voodoo_edge/core/transport.h"
#include "voodoo_edge/core/capability_registry.h"
#include "voodoo_edge/core/effect_dispatcher.h"
#include "voodoo_edge/core/event_publisher.h"
#include "voodoo_edge/core/device_state.h"
#include "voodoo_edge/core/device_identity.h"
#include "voodoo_edge/core/idempotency.h"
#include "voodoo_edge/core/connection_manager.h"
#include "voodoo_edge/core/device.h"

// Platform HAL
#include "voodoo_edge/platform/hal.h"

// Hardware abstractions
#include "voodoo_edge/hardware/led.h"
#include "voodoo_edge/hardware/button.h"
#include "voodoo_edge/hardware/motor.h"
#include "voodoo_edge/hardware/servo.h"
#include "voodoo_edge/hardware/relay.h"
#include "voodoo_edge/hardware/buzzer.h"
#include "voodoo_edge/hardware/encoder.h"
#include "voodoo_edge/hardware/sensor.h"
#include "voodoo_edge/hardware/temperature_sensor.h"

#endif // VODOO_EDGE_H
