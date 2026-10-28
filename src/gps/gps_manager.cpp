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

#include "gps_manager.h"

// Global GPS manager instance
GpsManager gpsManager;

// ================================================================================================
// CONSTRUCTOR / DESTRUCTOR
// ================================================================================================

GpsManager::GpsManager() : _parser() {
    _status = GpsStatus::GPS_NOT_DETECTED;
    _initialized = false;
    _has_home_position = false;
    _home_position.latitude = 0.0;
    _home_position.longitude = 0.0;
}

GpsManager::~GpsManager() {
    // Nothing to clean up
}

// ================================================================================================
// INITIALIZATION
// ================================================================================================

bool GpsManager::initialize(uint32_t baud_rate, uint8_t refresh_rate) {
    // Initialize the GPS implementation if available
    #if !defined(GPS_MANAGER_BASIC_ONLY)
        #if GPS_TYPE == GPS_TYPE_UBLOX
            _initialized = _gps_impl.initialize(baud_rate, refresh_rate);
            _status = _gps_impl.getStatus();
            return _initialized;
        #elif GPS_TYPE == GPS_TYPE_QUECTEL
            // TODO: Implement Quectel initialization
            _initialized = false;
            _status = GpsStatus::GPS_NOT_DETECTED;
            return false;
        #endif
    #else
        // Basic mode: just initialize the serial port
        #if HAS_HARDWARE_SERIAL
            serialGPS.begin(baud_rate, SERIAL_8N1, PIN_GPS_RX, PIN_GPS_TX);
        #else
            serialGPS.begin(baud_rate);
        #endif
        _initialized = true;
        _status = GpsStatus::GPS_OK;
        return true;
    #endif
}

// ================================================================================================
// GPS DATA ACCESS
// ================================================================================================

const GpsData& GpsManager::getData() const {
    #if !defined(GPS_MANAGER_BASIC_ONLY)
        #if GPS_TYPE == GPS_TYPE_UBLOX
            return _gps_impl.getData();
        #elif GPS_TYPE == GPS_TYPE_QUECTEL
            // TODO: Return Quectel data
            return _parser.getData();
        #endif
    #else
        return _parser.getData();
    #endif
}

bool GpsManager::hasValidFix() const {
    const GpsData& data = getData();
    return data.has_fix && data.last_update_ms > 0;
}

bool GpsManager::hasEnoughSatellites() const {
    const GpsData& data = getData();
    // Quality threshold, not a regulatory one: at least 4 satellites
    return data.satellites >= 4;
}

bool GpsManager::hasGoodPrecision() const {
    const GpsData& data = getData();
    // Quality threshold, not a regulatory one: HDOP up to 5.0
    return data.hdop > 0 && data.hdop <= 5.0;
}

bool GpsManager::hasValidAltitude() const {
    const GpsData& data = getData();
    // Check if altitude is valid (not 0 or invalid value)
    return data.altitude != 0.0;
}

// ================================================================================================
// GPS STATUS
// ================================================================================================

GpsStatus GpsManager::getStatus() const {
    // Determine status based on GPS data
    if (!_initialized) {
        return GpsStatus::GPS_NOT_DETECTED;
    }
    
    if (!hasValidFix()) {
        return GpsStatus::GPS_FIX_LOST;
    }
    
    if (!hasEnoughSatellites()) {
        return GpsStatus::GPS_BAD_SATELLITES;
    }
    
    if (!hasGoodPrecision()) {
        return GpsStatus::GPS_BAD_HDOP;
    }
    
    if (!hasValidAltitude()) {
        return GpsStatus::GPS_BAD_ALTITUDE;
    }
    
    return GpsStatus::GPS_OK;
}

bool GpsManager::isReady() const {
    return _initialized && getStatus() == GpsStatus::GPS_OK;
}

// ================================================================================================
// UPDATE GPS DATA
// ================================================================================================

void GpsManager::update() {
    // Read data from GPS serial port
    while (serialGPS.available()) {
        char c = serialGPS.read();
        processChar(c);
    }
}

void GpsManager::processChar(char c) {
    // Pass to parser
    _parser.parse(c);
    
    // Also pass to specific implementation if available
    #if !defined(GPS_MANAGER_BASIC_ONLY)
        #if GPS_TYPE == GPS_TYPE_UBLOX
            _gps_impl.processSerialData(c);
        #elif GPS_TYPE == GPS_TYPE_QUECTEL
            // TODO: Process Quectel data
        #endif
    #endif
}

void GpsManager::processSerialData(const char* buffer, uint16_t length) {
    for (uint16_t i = 0; i < length; i++) {
        processChar(buffer[i]);
    }
}

// ================================================================================================
// DIAGNOSTIC FUNCTIONS
// ================================================================================================

void GpsManager::echoGpsData(uint32_t duration_ms) {
    #if !defined(GPS_MANAGER_BASIC_ONLY)
        #if GPS_TYPE == GPS_TYPE_UBLOX
            _gps_impl.echoGpsData(duration_ms);
            return;
        #elif GPS_TYPE == GPS_TYPE_QUECTEL
            // TODO: Implement Quectel echo
        #endif
    #endif
    
    // Basic echo
    uint32_t end_time = millis() + duration_ms;
    while (millis() < end_time) {
        while (serialGPS.available()) {
            char c = serialGPS.read();
            Serial.write(c);
        }
        yield();
    }
}

uint32_t GpsManager::getSentenceCount() const {
    return _parser.getSentenceCount();
}

uint32_t GpsManager::getFailedChecksumCount() const {
    return _parser.getFailedChecksumCount();
}

String GpsManager::getLastSentenceType() const {
    return _parser.getLastSentenceType();
}

const char* GpsManager::getNavigationStatus() const {
    #if !defined(GPS_MANAGER_BASIC_ONLY) && GPS_TYPE == GPS_TYPE_UBLOX
        switch (_gps_impl.getNavigationResult()) {
            case UbxAckResult::ACK:     return "ACK";
            case UbxAckResult::NAK:     return "NAK";
            case UbxAckResult::TIMEOUT: return "no answer";
            default:                    return "not sent";
        }
    #else
        return "n/a";
    #endif
}

// ================================================================================================
// CONFIGURATION
// ================================================================================================

void GpsManager::enableSentence(NmeaSentenceType sentence_type, bool enabled) {
    _parser.enableSentence(sentence_type, enabled);
    
    #if !defined(GPS_MANAGER_BASIC_ONLY)
        #if GPS_TYPE == GPS_TYPE_UBLOX
            // For UBLOX, we need to send commands to enable/disable sentences on the GPS itself
            // This is handled separately
        #elif GPS_TYPE == GPS_TYPE_QUECTEL
            // TODO: Handle Quectel sentence configuration
        #endif
    #endif
}

void GpsManager::setBaudRate(uint32_t baud_rate) {
    #if !defined(GPS_MANAGER_BASIC_ONLY)
        #if GPS_TYPE == GPS_TYPE_UBLOX
            _gps_impl.setBaudRate(baud_rate);
            return;
        #elif GPS_TYPE == GPS_TYPE_QUECTEL
            // TODO: Implement Quectel baud rate change
        #endif
    #endif
    
    // Basic mode: change serial baud rate
    #if HAS_HARDWARE_SERIAL
        serialGPS.flush();
        serialGPS.updateBaudRate(baud_rate);
    #else
        serialGPS.flush();
        serialGPS.end();
        serialGPS.begin(baud_rate);
    #endif
}

void GpsManager::setRefreshRate(uint8_t rate) {
    #if !defined(GPS_MANAGER_BASIC_ONLY)
        #if GPS_TYPE == GPS_TYPE_UBLOX
            _gps_impl.setRefreshRate(rate);
            return;
        #elif GPS_TYPE == GPS_TYPE_QUECTEL
            // TODO: Implement Quectel refresh rate change
        #endif
    #endif
    // Basic mode: refresh rate is not configurable
}

// ================================================================================================
// SAVE HOME POSITION
// ================================================================================================

void GpsManager::saveHomePosition() {
    const GpsData& data = getData();
    if (data.has_fix && data.position.latitude != 0.0 && data.position.longitude != 0.0) {
        _home_position = data.position;
        _has_home_position = true;
    }
}

bool GpsManager::hasHomePosition() const {
    return _has_home_position;
}

GpsCoordinates GpsManager::getHomePosition() const {
    return _home_position;
}
