// secrets.example.h
// Voodoo Edge — Example secrets file
//
// Copy this file to secrets.h and fill in your values.
// secrets.h is excluded from version control via .gitignore.

#ifndef VODOO_SECRETS_EXAMPLE_H
#define VODOO_SECRETS_EXAMPLE_H

// WiFi credentials
#define WIFI_SSID "your-wifi-ssid"
#define WIFI_PASSWORD "your-wifi-password"

// MQTT broker (Voodoo Runtime)
#define MQTT_BROKER_HOST "192.168.1.100"
#define MQTT_BROKER_PORT 1883

// Optional: MQTT authentication
// #define MQTT_USERNAME "device_username"
// #define MQTT_PASSWORD "device_password"

// Optional: Device credential for enrollment
// #define DEVICE_CREDENTIAL "your-device-credential"

#endif // VODOO_SECRETS_EXAMPLE_H
