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

// ================================================================================================
// INCLUDES
// ================================================================================================

#include "core/biadud_core.h"
#include "core/biadud_platform.h"
#include "beacon/beacon_manager.h"
#include "gps/gps_manager.h"
#include "storage/eeprom_manager.h"
#include "hardware/led_manager.h"
#include "hardware/battery_monitor.h"
#include "network/wifi_manager.h"
#include "utils/debug_utils.h"

// ================================================================================================
// DEFINES
// ================================================================================================

// GPS Configuration
#define GPS_BAUD_RATE     9600
#define GPS_REFRESH_RATE  1    // 1 Hz

// WiFi Configuration
#define WIFI_AP_CHANNEL   1

// Timing Configuration
#define GPS_UPDATE_INTERVAL_MS   100

// ================================================================================================
// GLOBAL STATE
// ================================================================================================

// Timing state
static uint32_t _last_gps_update_ms = 0;

// GPS diagnostics — raw bytes pulled from the GPS UART by the main loop
static uint32_t _gps_bytes_read = 0;

// System state
static bool _system_initialized = false;
static bool _wifi_ready = false;
static bool _gps_ready = false;

// ================================================================================================
// SETUP
// ================================================================================================

void setup() {
    // Initialize debug output
    DebugUtils::begin(DEBUG_SERIAL_BAUD);
    delay(1000);
    
    DEBUG_INFO("==============================================");
    DEBUG_INFO("BIADUD - Balise Identification UAV DGAC");
    DEBUG_INFO("Version: " + String(BIADUD_VERSION_STRING));
    DEBUG_INFO("==============================================");
    
    // -------------------------------------------------------------------------------------------
    // Initialize EEPROM and load preferences
    // -------------------------------------------------------------------------------------------
    DEBUG_INFO("Initializing EEPROM...");
    eepromManager.begin(EEPROM_SIZE);
    
    // Check for factory reset pin
    bool factory_reset = eepromManager.checkFactoryReset();
    if (factory_reset) {
        DEBUG_INFO("Factory reset requested via pin!");
    }
    
    // Load preferences
    bool preferences_loaded = eepromManager.load();
    if (!preferences_loaded || !eepromManager.isValid() || factory_reset) {
        DEBUG_WARN("Preferences not loaded or invalid, using defaults");
        eepromManager.resetToFactory();
        
        // Generate default UAV ID from MAC address
        String default_uav_id = EepromManager::generateUavIdFromMac();
        String default_ssid = EepromManager::generateSsidFromMac();
        
        Preferences& prefs = eepromManager.get();
        strncpy(prefs.uav_id, default_uav_id.c_str(), UAV_ID_LENGTH);
        strncpy(prefs.ssid, default_ssid.c_str(), sizeof(prefs.ssid) - 1);
        
        eepromManager.save();
    }
    
    // Display preferences
    DEBUG_INFO("Current Preferences:");
    eepromManager.print();
    
    // -------------------------------------------------------------------------------------------
    // Initialize LED
    // -------------------------------------------------------------------------------------------
    DEBUG_INFO("Initializing LED...");
    // This board wires the LED active low: anode to 3V3, cathode to the pin.
    ledManager.begin(PIN_LED, false);
    ledManager.setState(LedState::LED_BLINK_SLOW);
    
    // -------------------------------------------------------------------------------------------
    // Initialize battery measurement
    // -------------------------------------------------------------------------------------------
    batteryMonitor.begin(PIN_BATTERY);
    
    // -------------------------------------------------------------------------------------------
    // Initialize GPS
    // -------------------------------------------------------------------------------------------
    DEBUG_INFO("Initializing GPS...");
    
    // Initialize serial GPS port
    #if HAS_HARDWARE_SERIAL
        serialGPS.begin(GPS_BAUD_RATE, SERIAL_8N1, PIN_GPS_RX, PIN_GPS_TX);
    #else
        serialGPS.begin(GPS_BAUD_RATE);
    #endif
    
    // Initialize GPS manager
    gpsManager.initialize(GPS_BAUD_RATE, GPS_REFRESH_RATE);
    
    // -------------------------------------------------------------------------------------------
    // Initialize WiFi (beacon transmission mode only)
    // -------------------------------------------------------------------------------------------
    // NOTE: DGAC beacon frames are sent via esp_wifi_80211_tx() on the AP interface
    // (see biadud_platform.cpp:sendBeaconFrame), so WiFi must stay active even though the
    // config web server / captive portal are not started in this minimal build.
    // TODO Phase 2: config UI (web server + captive portal DNS) — code exists in
    // network/web_server.h and network/dns_server.h, just not wired up yet.
    DEBUG_INFO("Initializing WiFi (beacon mode)...");
    wifiManager.begin();

    // Get preferences
    Preferences& prefs = eepromManager.get();

    String ssid = String(prefs.ssid[0] ? prefs.ssid : WIFI_SSID_PREFIX);

    _wifi_ready = wifiManager.startBeaconMode(ssid, WIFI_AP_CHANNEL, true);
    if (_wifi_ready) {
        DEBUG_INFO("WiFi beacon mode started: " + ssid);
    } else {
        DEBUG_ERROR("Failed to start WiFi beacon mode!");
    }

    // -------------------------------------------------------------------------------------------
    // Initialize Beacon Manager
    // -------------------------------------------------------------------------------------------
    DEBUG_INFO("Initializing beacon manager...");
    beaconManager.initialize(prefs.uav_id, ssid.c_str(), WIFI_AP_CHANNEL);
    
    _system_initialized = true;
    DEBUG_INFO("==============================================");
    DEBUG_INFO("System initialized successfully!");
    DEBUG_INFO("==============================================");
}

// ================================================================================================
// LOOP
// ================================================================================================

void loop() {
    // LedManager keeps its own timing, so it is updated every iteration. Gating this
    // call capped every pattern at the caller's rate: fast blink became slow blink,
    // and the pulse could not render at all.
    ledManager.update();
    batteryMonitor.update();
    
    // -------------------------------------------------------------------------------------------
    // Update GPS data
    // -------------------------------------------------------------------------------------------
    if (hasElapsed(_last_gps_update_ms, GPS_UPDATE_INTERVAL_MS)) {
        _last_gps_update_ms = millis();
        
        // Process GPS data from serial port
        while (serialGPS.available()) {
            char c = serialGPS.read();
            _gps_bytes_read++;
            gpsManager.processChar(c);
        }
        
        // Update GPS manager
        gpsManager.update();
        
        // Check if GPS is now ready (isReady() is the full DGAC criteria: fix,
        // 4+ satellites, HDOP below 5.0 and a valid altitude)
        if (!_gps_ready && gpsManager.isReady()) {
            _gps_ready = true;
            DEBUG_INFO("GPS has valid fix!");
            
            // Set home position when GPS first gets a fix
            GpsData gps_data = gpsManager.getData();
            if (gps_data.has_fix) {
                beaconManager.setHomePosition(gps_data.position.latitude, 
                                               gps_data.position.longitude, 
                                               gps_data.altitude);
            }
        }
        
        // Update beacon manager with latest GPS data
        if (_gps_ready) {
            beaconManager.updateGpsData(gpsManager.getData());
        }
    }
    
    // -------------------------------------------------------------------------------------------
    // Send beacon frames
    // -------------------------------------------------------------------------------------------
    // sendBeaconFrame() applies the DGAC conditions itself through shouldSend():
    // initialization, home position, GPS validity and the 3s / 30m period. No extra
    // guard here, so the very first frame can go out.
    if (_gps_ready) {
        beaconManager.sendBeaconFrame();
    }
    
    // -------------------------------------------------------------------------------------------
    // Periodic status update (every 5 seconds)
    // -------------------------------------------------------------------------------------------
    static uint32_t _last_status_update_ms = 0;
    if (hasElapsed(_last_status_update_ms, 5000)) {
        _last_status_update_ms = millis();
        
        // Print status to serial
        DEBUG_INFO("--- Status Update ---");
        DEBUG_INFO("Uptime: " + DebugUtils::getUptime());
        DEBUG_INFO("Free Heap: " + String(DebugUtils::getFreeHeap()) + " bytes");
        if (batteryMonitor.isAvailable()) {
            DEBUG_INFO("Battery: cell=" + String(batteryMonitor.getVoltageMv()) + " mV" +
                       " pin=" + String(batteryMonitor.getPinVoltageMv()) + " mV");
        }
        DEBUG_INFO(beaconManager.getStateString());
        DEBUG_INFO(beaconManager.getTxStatsString());

        // GPS link diagnostics: separates a dead RX line (bytes stuck at 0) from
        // sentences that arrive but get rejected, and from a receiver still searching.
        const GpsData& gps_diag = gpsManager.getData();
        DEBUG_INFO("GPS link: bytes=" + String(_gps_bytes_read) +
                   " sentences=" + String(gpsManager.getSentenceCount()) +
                   " badcrc=" + String(gpsManager.getFailedChecksumCount()) +
                   " last=" + gpsManager.getLastSentenceType());
        DEBUG_INFO("GPS data: fix=" + String(gps_diag.has_fix ? "YES" : "NO") +
                   " sats=" + String(gps_diag.satellites) +
                   " hdop=" + String(gps_diag.hdop, 2) +
                   " alt=" + String(gps_diag.altitude, 1) +
                   " lat=" + String(gps_diag.position.latitude, 6) +
                   " lon=" + String(gps_diag.position.longitude, 6) +
                   " speed=" + String(gps_diag.speed, 2) +
                   " course=" + String(gps_diag.heading, 1));

        if (_wifi_ready) {
            DEBUG_INFO(wifiManager.getNetworkInfo());
        }
        
        // Update LED state based on system state
        if (beaconManager.isReady()) {
            ledManager.setState(LedState::LED_ON);
        } else if (_gps_ready) {
            ledManager.setState(LedState::LED_BLINK_FAST);
        } else if (_wifi_ready) {
            ledManager.setState(LedState::LED_BLINK_SLOW);
        } else {
            ledManager.setState(LedState::LED_PULSE);
        }
    }
}
