// ************************************************************************************************
// * BALISE D'IDENTIFICATION A DISTANCE POUR UAV (DGAC)                                           *
// ************************************************************************************************
// * Author:     Maxime MARCONATO (maxime@maaax.com, aka MaaaX^^, aka EPOCH84)
// * Repository: https://github.com/EPOCH84/Balise_Identification_A_Distance_UAV_DGAC
// * Based on:
// * - https://github.com/fanfanlatulipe26/BaliseDGAC_GPS_Logger
// * - https://github.com/dev-fred/GPS_Tracker_ESP8266
// * - https://github.com/khancyr/droneID_FR
// * - https://github.com/f5soh/balise_esp32
// * - https://www.tranquille-informatique.fr/modelisme/divers/balise-dgac-signalement-electronique-a-distance-drone-aeromodelisme.html
// ************************************************************************************************
// GPL V3:
// This program is free software: you can redistribute it and/or modify it under the terms of the
// GNU General Public License as published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.
// This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
// without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
// ================================================================================================

#ifndef BIADUD_CORE_H
#define BIADUD_CORE_H

#include <Arduino.h>

// ================================================================================================
// VERSION
// ================================================================================================

#define BIADUD_VERSION_MAJOR    1
#define BIADUD_VERSION_MINOR    0
#define BIADUD_VERSION_PATCH    0
#define BIADUD_VERSION_BUILD    0

#define BIADUD_VERSION_STRING   "1.0.0.0"

// ================================================================================================
// FIRMWARE SIGNATURE
// ================================================================================================

#define BIADUD_SIGNATURE        "BIADUD01"
#define BIADUD_SIGNATURE_LENGTH 8

// ================================================================================================
// PLATFORM DEFINITION
// ================================================================================================

// Platform detection - defined in platformio.ini build flags
#if defined(TARGET_ESP32C3)
    #define PLATFORM_ESP32C3 1
    #define PLATFORM_NAME "ESP32-C3"
#elif defined(TARGET_ESP32S3)
    #define PLATFORM_ESP32S3 1
    #define PLATFORM_NAME "ESP32-S3"
#elif defined(TARGET_ESP32)
    #define PLATFORM_ESP32 1
    #define PLATFORM_NAME "ESP32"
#elif defined(TARGET_ESP8266)
    #define PLATFORM_ESP8266 1
    #define PLATFORM_NAME "ESP8266"
#else
    #error "Unsupported platform! Define TARGET_ESP32, TARGET_ESP32C3, TARGET_ESP32S3, or TARGET_ESP8266."
#endif

// ================================================================================================
// GPS TYPES
// ================================================================================================

#define GPS_TYPE_NONE       0
#define GPS_TYPE_UBLOX      1
#define GPS_TYPE_QUECTEL    2

// Default GPS type - can be overridden in biadud_options.h
#ifndef GPS_TYPE
    #define GPS_TYPE GPS_TYPE_UBLOX
#endif

// ================================================================================================
// BEACON PROTOCOL CONSTANTS (DGAC FR)
// ================================================================================================

// Frame header
#define BEACON_FRAME_VS         0xDD
#define BEACON_FRAME_VS_TYPE     1

// OUI for vendor-specific frame
#define BEACON_FRAME_OUI_0      0x6A
#define BEACON_FRAME_OUI_1      0x5C
#define BEACON_FRAME_OUI_2      0x35

// Protocol version
#define BEACON_PROTOCOL_VERSION 1

// ================================================================================================
// DATA TYPES FOR TLV ENCODED BEACON FRAME (DGAC)
// ================================================================================================

enum class DroneIdFrDataType : uint8_t {
    RESERVED        = 0,
    PROTOCOL_VERSION = 1,
    ID_FR           = 2,
    ID_ANSI_CTA     = 3,
    LATITUDE        = 4,   // WGS84 in degree * 1e7 (for integer storage)
    LONGITUDE       = 5,   // WGS84 in degree * 1e7
    ALTITUDE        = 6,   // MSL in meters (int16_t)
    HEIGTH          = 7,   // From home in meters (int16_t)
    HOME_LATITUDE   = 8,   // WGS84 in degree * 1e7
    HOME_LONGITUDE  = 9,   // WGS84 in degree * 1e7
    GROUND_SPEED    = 10,  // In m/s (uint8_t)
    HEADING         = 11,  // Heading in degree from north 0-359 (uint16_t)
    NOT_DEFINED_END = 12,  // Types 12 to 200 are reserved by the order: never emit them
};

// ================================================================================================
// TLV LENGTHS FOR EACH DATA TYPE
// ================================================================================================

#define TLV_LENGTH_RESERVED         0
#define TLV_LENGTH_PROTOCOL_VERSION 1
#define TLV_LENGTH_ID_FR             30  // 30 bytes for drone ID string
#define TLV_LENGTH_ID_ANSI_CTA      0
#define TLV_LENGTH_LATITUDE          4   // int32_t
#define TLV_LENGTH_LONGITUDE         4   // int32_t
#define TLV_LENGTH_ALTITUDE          2   // int16_t
#define TLV_LENGTH_HEIGTH            2   // int16_t
#define TLV_LENGTH_HOME_LATITUDE     4   // int32_t
#define TLV_LENGTH_HOME_LONGITUDE    4   // int32_t
#define TLV_LENGTH_GROUND_SPEED      1   // uint8_t
#define TLV_LENGTH_HEADING           2   // uint16_t

// ================================================================================================
// BEACON FRAME LIMITS (DGAC REGULATION)
// ================================================================================================

// Maximum time between two beacon frames: 3 seconds
#define BEACON_FRAME_TIME_LIMIT_MS     3000

// Maximum distance between two beacon frames: 30 meters
#define BEACON_FRAME_DISTANCE_LIMIT_M  30.0

// Maximum payload length for beacon frame
#define BEACON_FRAME_PAYLOAD_LEN_MAX    251

// ================================================================================================
// GPS STATUS CODES
// ================================================================================================

enum class GpsStatus : uint8_t {
    GPS_NOT_DETECTED        = 1,    // GPS not detected, no data received
    GPS_INVALID_POSITION    = 2,    // Invalid GPS position (fix in progress)
    GPS_FIX_LOST           = 3,    // GPS fix lost
    GPS_BAD_SATELLITES     = 4,    // Not enough satellites (>3 required)
    GPS_BAD_HDOP           = 5,    // HDOP too high (>5.0)
    GPS_BAD_ALTITUDE       = 6,    // Unknown altitude
    GPS_SAVING_HOME        = 7,    // Saving home position
    GPS_OK                 = 8,    // GPS data valid and ready
};

// ================================================================================================
// UAV ID STRUCTURE
// ================================================================================================

// UAV ID format as per DGAC:
// CCC (3 chars) - Constructor code (000 for DIY/amateur)
// MMM (3 chars) - Beacon model identifier
// SSSSSSSSSSSS (12 chars) - First part of serial number
// TTTTTTTTTTTT (12 chars) - Last part of serial number (typically MAC address without colons)
// Total: 30 characters

#define UAV_ID_LENGTH            30
#define UAV_ID_CONSTRUCTOR_CODE  "000"    // DIY/amateur constructor code

// Default UAV ID template - last 12 chars will be replaced with MAC address
#define UAV_ID_DEFAULT_TEMPLATE "000MB1000000000000000000000001"

// ================================================================================================
// NETWORK CONFIGURATION
// ================================================================================================

// Default SSID prefix - will be completed with MAC address
#define WIFI_SSID_PREFIX         "BIADUD"

// Default IP configuration for captive portal
#define WIFI_DEFAULT_IP          "8.8.4.4"
#define WIFI_DEFAULT_GATEWAY     "8.8.4.4"
#define WIFI_DEFAULT_NETMASK     "255.255.255.0"

// DNS server port for captive portal
#define DNS_SERVER_PORT          53

// Maximum WiFi clients
#define WIFI_MAX_CLIENTS         4

// ================================================================================================
// EEPROM CONFIGURATION
// ================================================================================================

// EEPROM size to reserve (bytes)
#define EEPROM_SIZE              512

// ================================================================================================
// SERIAL CONFIGURATION
// ================================================================================================

// Default serial baud rate for debug output
#define DEBUG_SERIAL_BAUD        115200

// Default GPS baud rates
#define GPS_DEFAULT_BAUD        9600
#define GPS_DEFAULT_RATE        1       // 1 Hz

// ================================================================================================
// PIN DEFINITIONS
// ================================================================================================

// PIN_LED, PIN_GPS_RX, PIN_GPS_TX, PIN_FACTORY_RESET are defined per-platform in biadud_platform.h,
// which always includes this file first — no fallback needed here.

// ================================================================================================
// DEBUG UTILITIES
// ================================================================================================

// Debug output macros
#if defined(DEBUG_ENABLED) && DEBUG_ENABLED
    #define DEBUG_PRINT(x)      Serial.print(x)
    #define DEBUG_PRINTLN(x)    Serial.println(x)
    #define DEBUG_PRINTF(...)   Serial.printf(__VA_ARGS__)
    #define DEBUG_PRINTF_P(...)  Serial.printf_P(PSTR(__VA_ARGS__))
#else
    #define DEBUG_PRINT(x)
    #define DEBUG_PRINTLN(x)
    #define DEBUG_PRINTF(...)
    #define DEBUG_PRINTF_P(...)
#endif

// ================================================================================================
// COMMON TYPES
// ================================================================================================

// Structure to hold GPS coordinates
struct GpsCoordinates {
    double latitude;   // WGS84 in decimal degrees
    double longitude;  // WGS84 in decimal degrees
    
    bool operator==(const GpsCoordinates& other) const {
        return latitude == other.latitude && longitude == other.longitude;
    }
    bool operator!=(const GpsCoordinates& other) const {
        return !(*this == other);
    }
};

// Structure to hold GPS data
struct GpsData {
    GpsCoordinates position;
    double altitude;          // MSL in meters
    double speed;            // Ground speed in m/s
    double heading;          // Heading in degrees (0-359)
    uint8_t satellites;      // Number of satellites
    double hdop;             // Horizontal dilution of precision
    bool has_fix;           // Valid fix available
    uint32_t last_update_ms; // Timestamp of last valid data
    
    // Clear/Reset data
    void reset() {
        position.latitude = 0.0;
        position.longitude = 0.0;
        altitude = 0.0;
        speed = 0.0;
        heading = 0.0;
        satellites = 0;
        hdop = 0.0;
        has_fix = false;
        last_update_ms = 0;
    }
};

// ================================================================================================
// TEMPLATE FOR PREFERENCES STRUCTURE (defined in storage module)
// ================================================================================================

// Forward declaration - actual definition in storage/eeprom_manager.h
struct Preferences;

extern Preferences g_preferences;

#endif // #ifndef BIADUD_CORE_H
