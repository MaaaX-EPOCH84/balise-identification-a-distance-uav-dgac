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

#ifndef EEPROM_MANAGER_H
#define EEPROM_MANAGER_H

#include <Arduino.h>
#include <EEPROM.h>
#include "../core/biadud_core.h"

// ================================================================================================
// PREFERENCES STRUCTURE
// This structure holds all user-configurable preferences
// ================================================================================================

struct Preferences {
    // Signature for EEPROM validation
    char signature[BIADUD_SIGNATURE_LENGTH + 1] = BIADUD_SIGNATURE;
    
    // UAV ID (30 characters + null terminator)
    char uav_id[UAV_ID_LENGTH + 1] = UAV_ID_DEFAULT_TEMPLATE;
    
    // WiFi configuration
    char ssid[33] = "";           // SSID (32 chars max + null)
    char password[11] = "";       // Password (10 chars max + null)
    
    // IP configuration
    char ip_local[16] = WIFI_DEFAULT_IP;
    char ip_gateway[16] = WIFI_DEFAULT_GATEWAY;
    char ip_subnet[16] = WIFI_DEFAULT_NETMASK;
    
    // GPS configuration
    uint32_t gps_baud = GPS_DEFAULT_BAUD;
    uint8_t gps_rate = GPS_DEFAULT_RATE;  // Refresh rate in Hz
    
    // Other settings
    uint8_t wifi_timeout = 45;       // WiFi timeout in seconds (0 = no timeout)
    bool wifi_enabled = true;       // WiFi AP enabled
    uint8_t reserved[64 - 2] = {0};  // Reserved for future use
};

// ================================================================================================
// EEPROM MANAGER CLASS
// Manages reading and writing preferences to EEPROM
// ================================================================================================

class EepromManager
{
public:
    // -------------------------------------------------------------------------------------------
    // CONSTRUCTOR / DESTRUCTOR
    // -------------------------------------------------------------------------------------------
    
    EepromManager();
    ~EepromManager();

    // -------------------------------------------------------------------------------------------
    // INITIALIZATION
    // -------------------------------------------------------------------------------------------

    /**
     * Initialize the EEPROM manager
     * Must be called before any other functions
     * 
     * @param size EEPROM size to reserve
     */
    void begin(size_t size = EEPROM_SIZE);

    // -------------------------------------------------------------------------------------------
    // LOAD AND SAVE
    // -------------------------------------------------------------------------------------------

    /**
     * Load preferences from EEPROM
     * 
     * @return true if preferences were loaded successfully
     */
    bool load();

    /**
     * Save preferences to EEPROM
     * 
     * @return true if preferences were saved successfully
     */
    bool save();

    // -------------------------------------------------------------------------------------------
    // FACTORY DEFAULT
    // -------------------------------------------------------------------------------------------

    /**
     * Reset preferences to factory defaults
     */
    void resetToFactory();

    /**
     * Check if factory reset is requested (via pin)
     * If requested, reset to factory and save
     * 
     * @return true if factory reset was performed
     */
    bool checkFactoryReset();

    // -------------------------------------------------------------------------------------------
    // ACCESSORS
    // -------------------------------------------------------------------------------------------

    /**
     * Get the current preferences
     * 
     * @return Reference to the current preferences
     */
    Preferences& get() { return _preferences; }

    /**
     * Get the current preferences (const version)
     * 
     * @return Const reference to the current preferences
     */
    const Preferences& get() const { return _preferences; }

    /**
     * Get the current preferences (alias for get())
     * 
     * @return Reference to the current preferences
     */
    Preferences& getPreferences() { return _preferences; }

    /**
     * Get the current preferences (const version, alias for get())
     * 
     * @return Const reference to the current preferences
     */
    const Preferences& getPreferences() const { return _preferences; }

    /**
     * Get factory default preferences
     * 
     * @return Reference to the factory default preferences
     */
    const Preferences& getFactoryDefaults() const { return _factory_defaults; }

    /**
     * Check if preferences have been loaded
     * 
     * @return true if preferences are valid
     */
    bool isValid() const { return _valid; }

    // -------------------------------------------------------------------------------------------
    // UTILITY FUNCTIONS
    // -------------------------------------------------------------------------------------------

    /**
     * Print preferences to serial
     */
    void print() const;

    /**
     * Check if EEPROM data is valid
     * 
     * @return true if EEPROM data has valid signature
     */
    bool validateSignature() const;

    // -------------------------------------------------------------------------------------------
    // GENERATE UAV ID
    // -------------------------------------------------------------------------------------------

    /**
     * Generate a UAV ID based on the MAC address
     * This creates a unique UAV ID for each device
     * 
     * @return Generated UAV ID string
     */
    static String generateUavIdFromMac();

    /**
     * Generate a default SSID based on the MAC address
     * 
     * @return Generated SSID string
     */
    static String generateSsidFromMac();

private:
    // -------------------------------------------------------------------------------------------
    // INTERNAL DATA
    // -------------------------------------------------------------------------------------------

    Preferences _preferences;
    Preferences _factory_defaults;
    bool _valid = false;
    bool _eeprom_initialized = false;
    
    // -------------------------------------------------------------------------------------------
    // INTERNAL FUNCTIONS
    // -------------------------------------------------------------------------------------------

    /**
     * Copy preferences from one structure to another
     * 
     * @param dest Destination preferences
     * @param src Source preferences
     */
    static void copyPreferences(Preferences& dest, const Preferences& src);
};

// Global EEPROM manager instance
extern EepromManager eepromManager;

// Global preferences variable (for backward compatibility)
extern Preferences g_preferences;

#endif // #ifndef EEPROM_MANAGER_H
