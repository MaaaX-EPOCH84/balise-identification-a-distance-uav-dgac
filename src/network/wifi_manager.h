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

#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <Arduino.h>
#include "../core/biadud_platform.h"

// ================================================================================================
// WIFI MANAGER CLASS
// ================================================================================================

class WiFiManager
{
public:
    // -------------------------------------------------------------------------------------------
    // CONSTRUCTOR / DESTRUCTOR
    // -------------------------------------------------------------------------------------------
    
    WiFiManager();
    ~WiFiManager();

    // -------------------------------------------------------------------------------------------
    // INITIALIZATION
    // -------------------------------------------------------------------------------------------

    /**
     * Initialize WiFi manager
     * Call this in setup() before any WiFi operations.
     */
    void begin();

    /**
     * Initialize WiFi in Access Point mode (captive portal)
     * 
     * @param ssid SSID to use for AP
     * @param password Password for AP (empty for open network)
     * @param channel WiFi channel (1-13)
     * @param hidden Whether to hide SSID
     * @return true if AP started successfully
     */
    bool startAccessPoint(const String& ssid, const String& password = "", 
                          int channel = 1, bool hidden = false);

    /**
     * Initialize WiFi in Station mode (connect to existing network)
     * 
     * @param ssid SSID to connect to
     * @param password Password
     * @param timeout_ms Timeout in milliseconds
     * @return true if connected successfully
     */
    bool startStation(const String& ssid, const String& password, 
                      uint32_t timeout_ms = WIFI_CONNECT_TIMEOUT_MS);

    // -------------------------------------------------------------------------------------------
    // CONFIGURATION
    // -------------------------------------------------------------------------------------------

    /**
     * Set static IP configuration for AP mode
     * 
     * @param local_ip Local IP address
     * @param gateway Gateway IP address
     * @param subnet Subnet mask
     */
    void setStaticIP(const IPAddress& local_ip, const IPAddress& gateway, 
                     const IPAddress& subnet);

    /**
     * Set WiFi output power (ESP32 only)
     * 
     * @param power Power level (0-20.5 dBm, typically 8-20)
     */
    void setOutputPower(float power);

    // -------------------------------------------------------------------------------------------
    // STATUS CHECKS
    // -------------------------------------------------------------------------------------------

    /**
     * Check if WiFi is enabled
     * 
     * @return true if WiFi is enabled
     */
    bool isEnabled() const { return _enabled; }

    /**
     * Check if connected to a network (Station mode)
     * 
     * @return true if connected
     */
    bool isConnected() const;

    /**
     * Check if in Access Point mode
     * 
     * @return true if in AP mode
     */
    bool isAccessPoint() const;

    /**
     * Check if in Station mode
     * 
     * @return true if in Station mode
     */
    bool isStation() const;

    /**
     * Get current WiFi mode
     * 
     * @return Wifi mode constant
     */
    uint8_t getMode() const;

    // -------------------------------------------------------------------------------------------
    // INFORMATION GETTERS
    // -------------------------------------------------------------------------------------------

    /**
     * Get local IP address
     * 
     * @return IPAddress object
     */
    IPAddress getLocalIP() const;

    /**
     * Get local IP as string
     * 
     * @return IP address string (e.g., "192.168.1.1")
     */
    String getLocalIPString() const;

    /**
     * Get MAC address
     * 
     * @return MAC address string (e.g., "AA:BB:CC:DD:EE:FF")
     */
    String getMacAddress() const;

    /**
     * Get MAC address without colons
     * 
     * @return MAC address string without colons (e.g., "AABBCCDDEEFF")
     */
    String getMacAddressNoColons() const;

    /**
     * Get current SSID
     * 
     * @return SSID string
     */
    String getSSID() const;

    /**
     * Get current RSSI (signal strength)
     * 
     * @return RSSI in dBm
     */
    int32_t getRSSI() const;

    /**
     * Get number of connected clients (AP mode only)
     * 
     * @return Number of connected clients
     */
    uint8_t getClientCount() const;

    // -------------------------------------------------------------------------------------------
    // NETWORK INFORMATION
    // -------------------------------------------------------------------------------------------

    /**
     * Get network information as string
     * 
     * @return Formatted string with network info
     */
    String getNetworkInfo() const;

    // -------------------------------------------------------------------------------------------
    // UTILITY FUNCTIONS
    // -------------------------------------------------------------------------------------------

    /**
     * Disconnect from current network
     */
    void disconnect();

    /**
     * Stop Access Point
     */
    void stopAccessPoint();

    /**
     * Reconnect to previously configured network
     * 
     * @return true if reconnected successfully
     */
    bool reconnect();

    /**
     * Scan for available networks
     * 
     * @param async If true, start async scan
     * @return Number of networks found (0 if async scan started)
     */
    int8_t scanNetworks(bool async = false);

    /**
     * Get scan results
     * 
     * @param count Number of networks to retrieve
     * @return Vector of network names
     */
    // Note: Simplified interface, actual scan results would need more complex handling

    /**
     * Generate default SSID from MAC address
     * 
     * @param prefix SSID prefix (default: WIFI_SSID_PREFIX from biadud_core.h)
     * @return Generated SSID string
     */
    String generateDefaultSSID(const String& prefix = WIFI_SSID_PREFIX);

    // -------------------------------------------------------------------------------------------
    // BEACON TRANSMISSION HELPERS
    // -------------------------------------------------------------------------------------------

    /**
     * Enable WiFi in AP mode for beacon transmission
     * This is a special mode that allows sending raw 802.11 packets
     * 
     * @param ssid SSID for the beacon
     * @param channel WiFi channel to use
     * @param hidden Whether to hide SSID
     * @return true if beacon mode started successfully
     */
    bool startBeaconMode(const String& ssid, int channel = 1, bool hidden = true);

    /**
     * Check if WiFi is in beacon transmission mode
     * 
     * @return true if in beacon mode
     */
    bool isBeaconMode() const { return _beacon_mode; }

private:
    // -------------------------------------------------------------------------------------------
    // INTERNAL STATE
    // -------------------------------------------------------------------------------------------

    bool _enabled = false;
    bool _ap_started = false;
    bool _station_connected = false;
    bool _beacon_mode = false;
    
    IPAddress _static_ip;
    IPAddress _static_gateway;
    IPAddress _static_subnet;
    
    String _current_ssid;
    String _current_password;
    
    uint32_t _connect_timeout_ms = WIFI_CONNECT_TIMEOUT_MS;
    
    // -------------------------------------------------------------------------------------------
    // PRIVATE HELPER FUNCTIONS
    // -------------------------------------------------------------------------------------------

    /**
     * Initialize WiFi hardware
     */
    void initializeWiFi();

    /**
     * Setup static IP for AP mode
     */
    void setupStaticIP();

    /**
     * Setup WiFi for beacon transmission
     */
    void setupBeaconWiFi();
};

// Global instance
extern WiFiManager wifiManager;

#endif // #ifndef WIFI_MANAGER_H
