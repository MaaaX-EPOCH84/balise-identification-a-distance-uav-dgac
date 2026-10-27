// ************************************************************************************************
// * BALISE D'IDENTIFICATION A DISTANCE POUR UAV (DGAC)                                           *
// ************************************************************************************************,
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

#ifndef BIADUD_PLATFORM_H
#define BIADUD_PLATFORM_H

#include "biadud_core.h"

// ================================================================================================
// PLATFORM-SPECIFIC CONFIGURATION
// ================================================================================================

// ESP32-C3 Configuration
// ---------------------
#if defined(PLATFORM_ESP32C3) || defined(TARGET_ESP32C3)

    #ifndef PLATFORM_ESP32C3
        #define PLATFORM_ESP32C3 1
    #endif
    #ifndef TARGET_ESP32C3
        #define TARGET_ESP32C3 1
    #endif

    // Board pin definitions for ESP32-C3-DevKitM-1
    #define PIN_LED                8   // Built-in LED (GPIO 8)
    #define PIN_GPS_RX             0   // GPS receiver (UART RX) - Connect to GPS TX
    #define PIN_GPS_TX             1   // GPS transmitter (UART TX) - Connect to GPS RX
    #define PIN_FACTORY_RESET      2   // Factory reset pin (GPIO 2)
    #define PIN_BATTERY            3   // Battery voltage through a 2 x 220 kOhm divider (ADC1)
    #define PIN_LED_WARNING        6   // Warning LED, active low (anode to 3V3)

    // UART definitions
    #define GPS_UART_NUM           1   // Use UART 1 for GPS (UART 0 is used for debug)

    // WiFi definitions
    #include <WiFi.h>
    #define WIFI_CLASS            WiFi
    #define WIFI_SERVER_CLASS     WiFiServer

    // Web Server definitions
    #include <WebServer.h>
    #include <DNSServer.h>
    #define WEBSERVER_CLASS       WebServer
    #define DNS_SERVER_CLASS      DNSServer

    // EEPROM
    #include <EEPROM.h>

    // LittleFS
    #include <LittleFS.h>
    #define FILESYSTEM            LittleFS

    // ESP32-C3 has HardwareSerial on all pins
    #define HAS_HARDWARE_SERIAL   1

    // Function to send raw 802.11 packet (for beacon frames)
    // Declaration from esp_wifi.h - this is for internal use
    extern "C" {
        #include "esp_wifi.h"
        esp_err_t esp_wifi_80211_tx(wifi_interface_t ifx, const void *buffer, int len, bool en_sys_seq);
    }

    // Timeout for WiFi operations
    #define WIFI_CONNECT_TIMEOUT_MS   10000

// ESP32-S3 Configuration
// ---------------------
#elif defined(PLATFORM_ESP32S3) || defined(TARGET_ESP32S3)

    #ifndef PLATFORM_ESP32S3
        #define PLATFORM_ESP32S3 1
    #endif
    #ifndef TARGET_ESP32S3
        #define TARGET_ESP32S3 1
    #endif

    // Board pin definitions for ESP32-S3
    // TODO: Define actual pins for ESP32-S3 board
    #define PIN_LED                8   // Default LED pin
    #define PIN_GPS_RX             0   // Default GPS RX pin
    #define PIN_GPS_TX             1   // Default GPS TX pin
    #define PIN_FACTORY_RESET      2   // Default factory reset pin
    #define PIN_BATTERY           -1   // No battery measurement wired
    #define PIN_LED_WARNING       -1   // No warning LED wired

    #define GPS_UART_NUM           1   // Use UART 1

    #include <WiFi.h>
    #define WIFI_CLASS            WiFi

    #include <WebServer.h>
    #include <DNSServer.h>
    #define WEBSERVER_CLASS       WebServer
    #define DNS_SERVER_CLASS      DNSServer

    #include <EEPROM.h>
    #include <LittleFS.h>
    #define FILESYSTEM            LittleFS

    #define HAS_HARDWARE_SERIAL   1

    extern "C" {
        #include "esp_wifi.h"
        esp_err_t esp_wifi_80211_tx(wifi_interface_t ifx, const void *buffer, int len, bool en_sys_seq);
    }

    #define WIFI_CONNECT_TIMEOUT_MS   10000

// ESP32 Configuration (Generic)
// ----------------------------
#elif defined(PLATFORM_ESP32) || defined(TARGET_ESP32)

    #ifndef PLATFORM_ESP32
        #define PLATFORM_ESP32 1
    #endif
    #ifndef TARGET_ESP32
        #define TARGET_ESP32 1
    #endif

    // Default pin definitions for generic ESP32
    #define PIN_LED                2   // Built-in LED (GPIO 2)
    #define PIN_GPS_RX            16   // Default GPS RX
    #define PIN_GPS_TX            17   // Default GPS TX
    #define PIN_FACTORY_RESET     25   // Default factory reset
    #define PIN_BATTERY           -1   // No battery measurement wired
    #define PIN_LED_WARNING       -1   // No warning LED wired

    #define GPS_UART_NUM           1   // Use UART 1

    #include <WiFi.h>
    #define WIFI_CLASS            WiFi

    #include <WebServer.h>
    #include <DNSServer.h>
    #define WEBSERVER_CLASS       WebServer
    #define DNS_SERVER_CLASS      DNSServer

    #include <EEPROM.h>
    #include <LittleFS.h>
    #define FILESYSTEM            LittleFS

    #define HAS_HARDWARE_SERIAL   1

    extern "C" {
        #include "esp_wifi.h"
        esp_err_t esp_wifi_80211_tx(wifi_interface_t ifx, const void *buffer, int len, bool en_sys_seq);
    }

    #define WIFI_CONNECT_TIMEOUT_MS   10000

// ESP8266 Configuration
// --------------------
#elif defined(PLATFORM_ESP8266) || defined(TARGET_ESP8266)

    #ifndef PLATFORM_ESP8266
        #define PLATFORM_ESP8266 1
    #endif
    #ifndef TARGET_ESP8266
        #define TARGET_ESP8266 1
    #endif

    // Board pin definitions for ESP8266
    // Common configurations: D1=GPIO5, D2=GPIO4
    #define PIN_LED                2   // Built-in LED (GPIO 2)
    #define PIN_GPS_RX             4   // D2 - Connect to GPS TX
    #define PIN_GPS_TX             5   // D1 - Connect to GPS RX
    #define PIN_FACTORY_RESET      3   // RX - factory reset (with pullup)
    #define PIN_BATTERY           -1   // No battery measurement wired
    #define PIN_LED_WARNING       -1   // No warning LED wired

    // ESP8266 uses SoftwareSerial for GPS
    #define GPS_UART_NUM           -1  // Not applicable for SoftwareSerial
    #define HAS_HARDWARE_SERIAL   0

    #include <ESP8266WiFi.h>
    #define WIFI_CLASS            ESP8266WiFi

    #include <ESP8266WebServer.h>
    #include <DNSServer.h>
    #define WEBSERVER_CLASS       ESP8266WebServer
    #define DNS_SERVER_CLASS      DNSServer

    #include <EEPROM.h>
    #include <LittleFS.h>
    #define FILESYSTEM            LittleFS

    // Function to send raw 802.11 packet on ESP8266
    extern "C" {
        #include "user_interface.h"
        int wifi_send_pkt_freedom(uint8 *buf, int len, bool sys_seq);
    }

    #define WIFI_CONNECT_TIMEOUT_MS   10000

// No platform defined - Error
#else
    #error "No platform defined! Please define TARGET_ESP32C3, TARGET_ESP32S3, TARGET_ESP32, or TARGET_ESP8266."
#endif

// ================================================================================================
// SERIAL ABSTRACTION
// ================================================================================================

// HardwareSerial is available on ESP32 platforms
#if HAS_HARDWARE_SERIAL
    #include <HardwareSerial.h>
    extern HardwareSerial serialGPS;
#else
    // ESP8266 uses SoftwareSerial
    #include <SoftwareSerial.h>
    extern SoftwareSerial serialGPS;
#endif

// ================================================================================================
// GPS TYPE CONFIGURATION
// ================================================================================================

// Override GPS type if option is defined
#if defined(OPTION_GPS_UBLOX)
    #undef GPS_TYPE
    #define GPS_TYPE GPS_TYPE_UBLOX
#elif defined(OPTION_GPS_QUECTEL)
    #undef GPS_TYPE
    #define GPS_TYPE GPS_TYPE_QUECTEL
#endif

// ================================================================================================
// WIFI HELPER FUNCTIONS
// ================================================================================================

// Get WiFi MAC address as string
String getWiFiMacAddress();

// Format MAC address without colons
String getWiFiMacAddressNoColons();

// ================================================================================================
// BEACON TRANSMISSION FUNCTIONS
// ================================================================================================

// Send raw 802.11 beacon frame
// Parameters:
//   buffer: pointer to the beacon frame data
//   len: length of the frame
// Returns: true if successful, false otherwise
bool sendBeaconFrame(const uint8_t* buffer, uint16_t len);

/**
 * Last error code returned by the radio transmit call, 0 when none has failed yet.
 * Diagnostic only: it tells a refused transmission from a frame never built.
 */
int getLastBeaconTxError();

/**
 * Read the MAC address of the soft AP interface
 * The radio rejects any frame whose source address is not this one.
 *
 * @param mac_out Six byte buffer receiving the address
 * @return true on success
 */
bool getApMacAddress(uint8_t* mac_out);

// ================================================================================================
// TIME HELPERS
// ================================================================================================

// Check if a certain time period has elapsed
// Parameters:
//   last_time_ms: the last timestamp in milliseconds
//   period_ms: the period to check in milliseconds
// Returns: true if period has elapsed, false otherwise
inline bool hasElapsed(uint32_t last_time_ms, uint32_t period_ms) {
    uint32_t current_ms = millis();
    // Handle millis() overflow (after ~50 days)
    if (current_ms < last_time_ms) {
        return true; // Overflow occurred, definitely elapsed
    }
    return (current_ms - last_time_ms) >= period_ms;
}

// Get elapsed time in milliseconds
// Parameters:
//   start_time_ms: the start timestamp in milliseconds
// Returns: elapsed time in milliseconds (handles overflow)
inline uint32_t getElapsed(uint32_t start_time_ms) {
    uint32_t current_ms = millis();
    if (current_ms >= start_time_ms) {
        return current_ms - start_time_ms;
    } else {
        // Overflow occurred
        return (0xFFFFFFFF - start_time_ms) + current_ms;
    }
}

// ================================================================================================
// MEMORY HELPERS
// ================================================================================================

// Get free heap size (platform-specific)
size_t getFreeHeap();

// ================================================================================================
// REBOOT FUNCTION
// ================================================================================================

// Reboot the system
void systemReboot();

#endif // #ifndef BIADUD_PLATFORM_H
