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
#include "voodoo_edge/core/idempotency.h"

// Platform HAL
#include "voodoo_edge/platform/hal.h"

// Hardware abstractions
#include "voodoo_edge/hardware/led.h"
#include "voodoo_edge/hardware/button.h"

#endif // VODOO_EDGE_H
