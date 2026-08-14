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

#include "biadud_platform.h"

// ================================================================================================
// GLOBAL SERIAL GPS OBJECT
// ================================================================================================

// Define the GPS serial object based on platform
#if HAS_HARDWARE_SERIAL
    HardwareSerial serialGPS(GPS_UART_NUM);
#else
    SoftwareSerial serialGPS(PIN_GPS_RX, PIN_GPS_TX);
#endif

// ================================================================================================
// WIFI HELPER FUNCTIONS
// ================================================================================================

String getWiFiMacAddress() {
    #if defined(PLATFORM_ESP32C3) || defined(PLATFORM_ESP32S3) || defined(PLATFORM_ESP32)
        return WIFI_CLASS.macAddress();
    #elif defined(PLATFORM_ESP8266)
        return WIFI_CLASS.macAddress();
    #else
        return String("00:00:00:00:00:00");
    #endif
}

String getWiFiMacAddressNoColons() {
    String mac = getWiFiMacAddress();
    mac.replace(":", "");
    return mac;
}

// ================================================================================================
// BEACON TRANSMISSION FUNCTIONS
// ================================================================================================

// Last error returned by the radio transmit call, kept so the periodic status can
// tell a refused frame from a frame that was never built.
static int _last_beacon_tx_error = 0;

int getLastBeaconTxError() {
    return _last_beacon_tx_error;
}

bool getApMacAddress(uint8_t* mac_out) {
    if (mac_out == nullptr) {
        return false;
    }

    #if defined(PLATFORM_ESP32C3) || defined(PLATFORM_ESP32S3) || defined(PLATFORM_ESP32)
        return esp_wifi_get_mac(WIFI_IF_AP, mac_out) == ESP_OK;
    #elif defined(PLATFORM_ESP8266)
        return wifi_get_macaddr(SOFTAP_IF, mac_out);
    #else
        return false;
    #endif
}

bool sendBeaconFrame(const uint8_t* buffer, uint16_t len) {
    #if defined(PLATFORM_ESP32C3) || defined(PLATFORM_ESP32S3) || defined(PLATFORM_ESP32)
        // ESP32 platform - use esp_wifi_80211_tx
        // WIFI_IF_AP = softAP interface
        // en_sys_seq = true: the sequence number field is filled by the SDK, as the
        // beacon header leaves it at zero.
        esp_err_t err = esp_wifi_80211_tx(WIFI_IF_AP, (const void*)buffer, len, true);
        _last_beacon_tx_error = static_cast<int>(err);
        return err == ESP_OK;

    #elif defined(PLATFORM_ESP8266)
        // ESP8266 platform - use wifi_send_pkt_freedom
        return wifi_send_pkt_freedom(const_cast<uint8_t*>(buffer), len, false) == 0;
    
    #else
        // No beacon support on other platforms
        return false;
    #endif
}

// ================================================================================================
// MEMORY HELPERS
// ================================================================================================

size_t getFreeHeap() {
    #if defined(PLATFORM_ESP32C3) || defined(PLATFORM_ESP32S3) || defined(PLATFORM_ESP32)
        return ESP.getFreeHeap();
    #elif defined(PLATFORM_ESP8266)
        return ESP.getFreeHeap();
    #else
        return 0;
    #endif
}

// ================================================================================================
// REBOOT FUNCTION
// ================================================================================================

void systemReboot() {
    #if defined(PLATFORM_ESP32C3) || defined(PLATFORM_ESP32S3) || defined(PLATFORM_ESP32)
        ESP.restart();
    #elif defined(PLATFORM_ESP8266)
        ESP.restart();
    #endif
}
