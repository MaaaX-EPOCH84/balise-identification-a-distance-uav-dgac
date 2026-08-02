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
// See the GNU General License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
// ================================================================================================

#include "wifi_manager.h"

// Global instance
WiFiManager wifiManager;

// ================================================================================================
// CONSTRUCTOR / DESTRUCTOR
// ================================================================================================

WiFiManager::WiFiManager() {
    _enabled = false;
    _ap_started = false;
    _station_connected = false;
    _beacon_mode = false;
}

WiFiManager::~WiFiManager() {
    // Clean up if needed
    if (_enabled) {
        disconnect();
        stopAccessPoint();
    }
}

// ================================================================================================
// INITIALIZATION
// ================================================================================================

void WiFiManager::begin() {
    if (_enabled) {
        return;
    }
    
    initializeWiFi();
    _enabled = true;
}

void WiFiManager::initializeWiFi() {
    #if defined(PLATFORM_ESP32C3) || defined(PLATFORM_ESP32S3) || defined(PLATFORM_ESP32)
        // ESP32: Enable WiFi
        WiFi.mode(WIFI_OFF);
        delay(10);
        WiFi.mode(WIFI_AP);
        WiFi.setAutoConnect(false);
        WiFi.setAutoReconnect(false);
        
    #elif defined(PLATFORM_ESP8266)
        // ESP8266: Enable WiFi
        WiFi.mode(WIFI_OFF);
        delay(10);
        WiFi.mode(WIFI_AP);
        WiFi.persistent(false);
        
    #endif
}

bool WiFiManager::startAccessPoint(const String& ssid, const String& password, 
                                    int channel, bool hidden) {
    if (!isEnabled()) {
        begin();
    }
    
    // Stop any existing connection
    disconnect();
    stopAccessPoint();
    
    _current_ssid = ssid;
    _current_password = password;
    
    #if defined(PLATFORM_ESP32C3) || defined(PLATFORM_ESP32S3) || defined(PLATFORM_ESP32)
        
        // Configure static IP if set
        if (_static_ip != INADDR_NONE) {
            WiFi.softAPConfig(_static_ip, _static_gateway, _static_subnet);
        }
        
        // Start AP
        bool result = WiFi.softAP(ssid.c_str(), password.c_str(), channel, hidden);
        
        if (result) {
            _ap_started = true;
            _beacon_mode = false;
            
            // Set output power for beacon transmission
            setOutputPower(8.0); // Default: 8 dBm
        }
        
        return result;
        
    #elif defined(PLATFORM_ESP8266)
        
        // Configure static IP if set
        if (_static_ip != INADDR_NONE) {
            WiFi.config(_static_ip, _static_gateway, _static_subnet);
        }
        
        // Start AP
        bool result = WiFi.softAP(ssid.c_str(), password.c_str(), channel, hidden);
        
        if (result) {
            _ap_started = true;
            _beacon_mode = false;
        }
        
        return result;
        
    #else
        return false;
    #endif
}

bool WiFiManager::startStation(const String& ssid, const String& password, 
                                uint32_t timeout_ms) {
    if (!isEnabled()) {
        begin();
    }
    
    // Stop any existing AP
    stopAccessPoint();
    disconnect();
    
    _current_ssid = ssid;
    _current_password = password;
    _connect_timeout_ms = timeout_ms;
    
    #if defined(PLATFORM_ESP32C3) || defined(PLATFORM_ESP32S3) || defined(PLATFORM_ESP32)
        
        WiFi.begin(ssid.c_str(), password.c_str());
        
        uint32_t start_time = millis();
        while (WiFi.status() != WL_CONNECTED && 
               (millis() - start_time) < timeout_ms) {
            delay(100);
        }
        
        _station_connected = (WiFi.status() == WL_CONNECTED);
        _ap_started = false;
        _beacon_mode = false;
        
        return _station_connected;
        
    #elif defined(PLATFORM_ESP8266)
        
        WiFi.begin(ssid.c_str(), password.c_str());
        
        uint32_t start_time = millis();
        while (WiFi.status() != WL_CONNECTED && 
               (millis() - start_time) < timeout_ms) {
            delay(100);
        }
        
        _station_connected = (WiFi.status() == WL_CONNECTED);
        _ap_started = false;
        _beacon_mode = false;
        
        return _station_connected;
        
    #else
        return false;
    #endif
}

// ================================================================================================
// CONFIGURATION
// ================================================================================================

void WiFiManager::setStaticIP(const IPAddress& local_ip, const IPAddress& gateway, 
                             const IPAddress& subnet) {
    _static_ip = local_ip;
    _static_gateway = gateway;
    _static_subnet = subnet;
    
    // If AP is already running, reconfigure it
    if (_ap_started) {
        setupStaticIP();
    }
}

void WiFiManager::setupStaticIP() {
    #if defined(PLATFORM_ESP32C3) || defined(PLATFORM_ESP32S3) || defined(PLATFORM_ESP32)
        if (_static_ip != INADDR_NONE) {
            WiFi.softAPConfig(_static_ip, _static_gateway, _static_subnet);
        }
    #elif defined(PLATFORM_ESP8266)
        if (_static_ip != INADDR_NONE) {
            WiFi.config(_static_ip, _static_gateway, _static_subnet);
        }
    #endif
}

void WiFiManager::setOutputPower(float power) {
    #if defined(PLATFORM_ESP32C3) || defined(PLATFORM_ESP32S3) || defined(PLATFORM_ESP32)
        // ESP32 WiFi output power is set via esp_wifi_set_max_tx_power
        // Power is in dBm, range 8-20 typically
        esp_wifi_set_max_tx_power(power);
    #endif
    // ESP8266 doesn't have easy access to set output power
}

// ================================================================================================
// STATUS CHECKS
// ================================================================================================

bool WiFiManager::isConnected() const {
    #if defined(PLATFORM_ESP32C3) || defined(PLATFORM_ESP32S3) || defined(PLATFORM_ESP32)
        return _station_connected && WiFi.status() == WL_CONNECTED;
    #elif defined(PLATFORM_ESP8266)
        return _station_connected && WiFi.status() == WL_CONNECTED;
    #else
        return false;
    #endif
}

bool WiFiManager::isAccessPoint() const {
    #if defined(PLATFORM_ESP32C3) || defined(PLATFORM_ESP32S3) || defined(PLATFORM_ESP32)
        return _ap_started && WiFi.getMode() == WIFI_AP;
    #elif defined(PLATFORM_ESP8266)
        return _ap_started && WiFi.getMode() == WIFI_AP;
    #else
        return _ap_started;
    #endif
}

bool WiFiManager::isStation() const {
    #if defined(PLATFORM_ESP32C3) || defined(PLATFORM_ESP32S3) || defined(PLATFORM_ESP32)
        return _station_connected && WiFi.getMode() == WIFI_STA;
    #elif defined(PLATFORM_ESP8266)
        return _station_connected && WiFi.getMode() == WIFI_STA;
    #else
        return _station_connected;
    #endif
}

uint8_t WiFiManager::getMode() const {
    #if defined(PLATFORM_ESP32C3) || defined(PLATFORM_ESP32S3) || defined(PLATFORM_ESP32)
        return WiFi.getMode();
    #elif defined(PLATFORM_ESP8266)
        return WiFi.getMode();
    #else
        return 0;
    #endif
}

// ================================================================================================
// INFORMATION GETTERS
// ================================================================================================

IPAddress WiFiManager::getLocalIP() const {
    if (isStation()) {
        return WiFi.localIP();
    } else if (isAccessPoint()) {
        return WiFi.softAPIP();
    }
    return IPAddress(INADDR_NONE);
}

String WiFiManager::getLocalIPString() const {
    IPAddress ip = getLocalIP();
    return ip.toString();
}

String WiFiManager::getMacAddress() const {
    return getWiFiMacAddress();
}

String WiFiManager::getMacAddressNoColons() const {
    return getWiFiMacAddressNoColons();
}

String WiFiManager::getSSID() const {
    if (isStation()) {
        return WiFi.SSID();
    } else if (isAccessPoint()) {
        return WiFi.softAPSSID();
    }
    return _current_ssid;
}

int32_t WiFiManager::getRSSI() const {
    if (isStation()) {
        return WiFi.RSSI();
    }
    return 0;
}

uint8_t WiFiManager::getClientCount() const {
    if (!isAccessPoint()) {
        return 0;
    }
    
    #if defined(PLATFORM_ESP32C3) || defined(PLATFORM_ESP32S3) || defined(PLATFORM_ESP32)
        return WiFi.softAPgetStationNum();
    #elif defined(PLATFORM_ESP8266)
        // ESP8266 doesn't have a direct way to get client count
        // Would need to use esp_wifi_api.h for internal API
        return 0;
    #else
        return 0;
    #endif
}

// ================================================================================================
// NETWORK INFORMATION
// ================================================================================================

String WiFiManager::getNetworkInfo() const {
    String info;
    
    info += "WiFi: ";
    if (!isEnabled()) {
        info += "NOT ENABLED";
    } else if (isAccessPoint()) {
        info += "AP Mode";
        info += " | SSID: ";
        info += getSSID();
        info += " | IP: ";
        info += getLocalIPString();
        info += " | Clients: ";
        info += String(getClientCount());
    } else if (isStation()) {
        info += "STATION Mode";
        info += " | SSID: ";
        info += getSSID();
        info += " | IP: ";
        info += getLocalIPString();
        info += " | RSSI: ";
        info += String(getRSSI());
        info += " dBm";
    } else {
        info += "IDLE";
    }
    
    info += " | MAC: ";
    info += getMacAddressNoColons();
    
    return info;
}

// ================================================================================================
// UTILITY FUNCTIONS
// ================================================================================================

void WiFiManager::disconnect() {
    #if defined(PLATFORM_ESP32C3) || defined(PLATFORM_ESP32S3) || defined(PLATFORM_ESP32)
        if (WiFi.status() == WL_CONNECTED) {
            WiFi.disconnect();
        }
        _station_connected = false;
    #elif defined(PLATFORM_ESP8266)
        if (WiFi.status() == WL_CONNECTED) {
            WiFi.disconnect();
        }
        _station_connected = false;
    #endif
}

void WiFiManager::stopAccessPoint() {
    #if defined(PLATFORM_ESP32C3) || defined(PLATFORM_ESP32S3) || defined(PLATFORM_ESP32)
        if (WiFi.getMode() == WIFI_AP || WiFi.getMode() == WIFI_AP_STA) {
            WiFi.softAPdisconnect(true);
        }
        _ap_started = false;
    #elif defined(PLATFORM_ESP8266)
        if (WiFi.getMode() == WIFI_AP || WiFi.getMode() == WIFI_AP_STA) {
            WiFi.softAPdisconnect(true);
        }
        _ap_started = false;
    #endif
}

bool WiFiManager::reconnect() {
    if (_current_ssid.isEmpty()) {
        return false;
    }
    
    return startStation(_current_ssid, _current_password, _connect_timeout_ms);
}

int8_t WiFiManager::scanNetworks(bool async) {
    #if defined(PLATFORM_ESP32C3) || defined(PLATFORM_ESP32S3) || defined(PLATFORM_ESP32) || defined(PLATFORM_ESP8266)
        return WiFi.scanNetworks(async);
    #else
        return 0;
    #endif
}

String WiFiManager::generateDefaultSSID(const String& prefix) {
    return prefix + "_" + getMacAddressNoColons();
}

// ================================================================================================
// BEACON TRANSMISSION HELPERS
// ================================================================================================

bool WiFiManager::startBeaconMode(const String& ssid, int channel, bool hidden) {
    if (!isEnabled()) {
        begin();
    }
    
    // Stop any existing connections
    disconnect();
    stopAccessPoint();
    
    _current_ssid = ssid;
    
    #if defined(PLATFORM_ESP32C3) || defined(PLATFORM_ESP32S3) || defined(PLATFORM_ESP32)
        
        // For ESP32, we need to enable the AP interface for beacon transmission
        // This is a special configuration that allows sending raw 802.11 packets
        
        // Configure WiFi in AP mode with hidden SSID
        WiFi.mode(WIFI_AP);
        WiFi.softAP(ssid.c_str(), "", channel, hidden);
        
        // Set output power to maximum for beacon transmission
        esp_wifi_set_max_tx_power(80); // 80 * 0.25dBm = 20 dBm (maximum)
        
        _ap_started = true;
        _beacon_mode = true;
        _station_connected = false;
        
        return true;
        
    #elif defined(PLATFORM_ESP8266)
        
        // For ESP8266, similar approach
        WiFi.mode(WIFI_AP);
        WiFi.softAP(ssid.c_str(), "", channel, hidden);
        
        _ap_started = true;
        _beacon_mode = true;
        _station_connected = false;
        
        return true;
        
    #else
        return false;
    #endif
}

void WiFiManager::setupBeaconWiFi() {
    // This is called internally to configure WiFi for optimal beacon transmission
    #if defined(PLATFORM_ESP32C3) || defined(PLATFORM_ESP32S3) || defined(PLATFORM_ESP32)
        // Set output power to maximum
        esp_wifi_set_max_tx_power(80); // 80 * 0.25dBm = 20 dBm (maximum)
        
        // Enable beacon transmission
        // Note: The actual beacon frame sending is done via sendBeaconFrame()
        // which calls esp_wifi_80211_tx()
        
    #endif
}
