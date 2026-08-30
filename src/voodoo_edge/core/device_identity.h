// device_identity.h
// Voodoo Edge — Device identity
//
// Holds device identity information: ID, type, firmware version,
// hardware revision, and other metadata.

#ifndef VODOO_CORE_DEVICE_IDENTITY_H
#define VODOO_CORE_DEVICE_IDENTITY_H

#include "voodoo_edge/core/protocol.h"
#include <cstdint>
#include <cstring>

namespace voodoo
{
    namespace edge
    {

        struct DeviceIdentity
        {
            char device_id[MAX_DEVICE_ID_LEN];
            char device_type[32];
            char firmware_version[16];
            char hardware_revision[16];
            char manufacturer[32];
            char name[48]; // human-readable name

            DeviceIdentity()
            {
                memset(this, 0, sizeof(DeviceIdentity));
            }

            // Set fields with safe string copy
            void set_device_id(const char *id)
            {
                strncpy(device_id, id, MAX_DEVICE_ID_LEN - 1);
            }

            void set_device_type(const char *type)
            {
                strncpy(device_type, type, sizeof(device_type) - 1);
            }

            void set_firmware_version(const char *version)
            {
                strncpy(firmware_version, version, sizeof(firmware_version) - 1);
            }

            void set_hardware_revision(const char *revision)
            {
                strncpy(hardware_revision, revision, sizeof(hardware_revision) - 1);
            }

            void set_manufacturer(const char *mfr)
            {
                strncpy(manufacturer, mfr, sizeof(manufacturer) - 1);
            }

            void set_name(const char *n)
            {
                strncpy(name, n, sizeof(name) - 1);
            }
        };

    } // namespace edge
} // namespace voodoo

#endif // VODOO_CORE_DEVICE_IDENTITY_H
