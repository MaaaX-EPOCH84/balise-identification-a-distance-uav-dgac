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

#include "eeprom_manager.h"
#include "../core/biadud_platform.h"

// Global instances
EepromManager eepromManager;
Preferences g_preferences;

// ================================================================================================
// CONSTRUCTOR / DESTRUCTOR
// ================================================================================================

EepromManager::EepromManager() {
    // Initialize with factory defaults
    copyPreferences(_preferences, _factory_defaults);
    copyPreferences(_factory_defaults, _factory_defaults);
    _valid = false;
    _eeprom_initialized = false;
}

EepromManager::~EepromManager() {
    // Nothing to clean up
}

// ================================================================================================
// INITIALIZATION
// ================================================================================================

void EepromManager::begin(size_t size) {
    if (!_eeprom_initialized) {
        EEPROM.begin(size);
        _eeprom_initialized = true;
    }
}

// ================================================================================================
// LOAD AND SAVE
// ================================================================================================

bool EepromManager::load() {
    if (!_eeprom_initialized) {
        begin();
    }
    
    // Read from EEPROM
    EEPROM.get(0, _preferences);
    
    // Validate signature
    if (!validateSignature()) {
        // Invalid signature - reset to factory defaults
        resetToFactory();
        save();
        _valid = true;
        return false;
    }
    
    _valid = true;
    return true;
}

bool EepromManager::save() {
    if (!_eeprom_initialized) {
        begin();
    }
    
    // Write to EEPROM
    EEPROM.put(0, _preferences);
    
    // Commit changes
    #if defined(PLATFORM_ESP32C3) || defined(PLATFORM_ESP32S3) || defined(PLATFORM_ESP32) || defined(PLATFORM_ESP8266)
        EEPROM.commit();
    #endif
    
    return true;
}

// ================================================================================================
// FACTORY DEFAULT
// ================================================================================================

void EepromManager::resetToFactory() {
    copyPreferences(_preferences, _factory_defaults);
    _valid = true;
}

bool EepromManager::checkFactoryReset() {
    // Check if factory reset pin is active
    #ifdef PIN_FACTORY_RESET
        // Initialize pin as input with pullup
        pinMode(PIN_FACTORY_RESET, INPUT_PULLUP);
        
        // If pin is LOW (button pressed), perform factory reset
        if (digitalRead(PIN_FACTORY_RESET) == LOW) {
            resetToFactory();
            save();
            return true;
        }
    #endif
    
    return false;
}

// ================================================================================================
// UTILITY FUNCTIONS
// ================================================================================================

void EepromManager::print() const {
    Serial.println("=== Preferences ===");
    Serial.print("Signature: ");
    Serial.println(_preferences.signature);
    
    Serial.print("UAV ID: ");
    Serial.println(_preferences.uav_id);
    
    Serial.print("SSID: ");
    Serial.println(_preferences.ssid[0] ? _preferences.ssid : "(empty)");
    
    Serial.print("Password: ");
    Serial.println(_preferences.password[0] ? "*****" : "(empty)");
    
    Serial.print("IP Local: ");
    Serial.println(_preferences.ip_local);
    
    Serial.print("IP Gateway: ");
    Serial.println(_preferences.ip_gateway);
    
    Serial.print("IP Subnet: ");
    Serial.println(_preferences.ip_subnet);
    
    Serial.print("GPS Baud: ");
    Serial.println(_preferences.gps_baud);
    
    Serial.print("GPS Rate: ");
    Serial.println(_preferences.gps_rate);
    
    Serial.print("WiFi Timeout: ");
    Serial.println(_preferences.wifi_timeout);
    
    Serial.print("WiFi Enabled: ");
    Serial.println(_preferences.wifi_enabled ? "YES" : "NO");
    
    Serial.println("==================");
}

bool EepromManager::validateSignature() const {
    return strcmp(_preferences.signature, BIADUD_SIGNATURE) == 0;
}

// ================================================================================================
// GENERATE UAV ID
// ================================================================================================

String EepromManager::generateUavIdFromMac() {
    // Get MAC address without colons
    String mac = getWiFiMacAddressNoColons();
    
    // Construct UAV ID from template and MAC
    // Template: "000MB1000000000000000000000001"
    // Replace last 12 characters with MAC
    String uav_id = UAV_ID_DEFAULT_TEMPLATE;
    
    if (mac.length() >= 12) {
        // Replace the last 12 characters with MAC
        uav_id = uav_id.substring(0, UAV_ID_LENGTH - 12) + mac.substring(0, 12);
    }
    
    return uav_id;
}

String EepromManager::generateSsidFromMac() {
    String mac = getWiFiMacAddressNoColons();
    return String(WIFI_SSID_PREFIX) + "_" + mac;
}

// ================================================================================================
// INTERNAL FUNCTIONS
// ================================================================================================

void EepromManager::copyPreferences(Preferences& dest, const Preferences& src) {
    // Simple byte-by-byte copy
    memcpy(&dest, &src, sizeof(Preferences));
}
