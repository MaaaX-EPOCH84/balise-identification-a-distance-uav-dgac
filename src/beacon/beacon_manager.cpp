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

#include "beacon_manager.h"

// Global beacon manager instance
BeaconManager beaconManager;

// ================================================================================================
// CONSTRUCTOR / DESTRUCTOR
// ================================================================================================

BeaconManager::BeaconManager() : _drone_id_fr() {
    _initialized = false;
    _home_set = false;
    _frame_count = 0;
    _last_send_ms = 0;
    _home_altitude = 0.0;
    _last_gps_data.reset();
    _last_send_position.latitude = 0.0;
    _last_send_position.longitude = 0.0;
}

BeaconManager::~BeaconManager() {
    // Nothing to clean up
}

// ================================================================================================
// INITIALIZATION
// ================================================================================================

void BeaconManager::initialize(const char* drone_id) {
    if (drone_id != nullptr) {
        setDroneId(drone_id);
    } else {
        // Use default drone ID
        setDroneId(UAV_ID_DEFAULT_TEMPLATE);
    }
    
    _initialized = true;
}

// ================================================================================================
// SETTERS
// ================================================================================================

void BeaconManager::setDroneId(const char* drone_id) {
    _drone_id_fr.setDroneId(drone_id);
}

void BeaconManager::setHomePosition(double lat, double lon, double alt) {
    // Set home position in DroneIdFr
    _drone_id_fr.setHomePosition(lat, lon, static_cast<int16_t>(alt));
    
    // Store for internal tracking
    _home_set = true;
    _home_altitude = alt;
    _last_send_position.latitude = lat;
    _last_send_position.longitude = lon;
}

// ================================================================================================
// UPDATE GPS DATA
// ================================================================================================

void BeaconManager::updateGpsData(const GpsData& gps_data) {
    // Store last GPS data
    _last_gps_data = gps_data;
    
    // If we don't have a home position yet, save it
    if (!_home_set && gps_data.has_fix && gps_data.position.latitude != 0.0) {
        setHomePosition(gps_data.position.latitude, gps_data.position.longitude, gps_data.altitude);
    }
    
    // Update DroneIdFr with new data
    if (_home_set) {
        updateDroneIdFr(gps_data);
    }
}

// ================================================================================================
// BEACON FRAME GENERATION AND TRANSMISSION
// ================================================================================================

uint16_t BeaconManager::generateBeaconFrame(uint8_t* buffer, uint16_t buffer_size) {
    if (buffer == nullptr || buffer_size == 0) {
        return 0;
    }
    
    // Use DroneIdFr to generate the frame
    return _drone_id_fr.generateBeaconFrame(buffer, 0);
}

bool BeaconManager::sendBeaconFrame() {
    // Check if we should send
    if (!shouldSend()) {
        return false;
    }
    
    // Generate frame
    uint8_t frame_buffer[512]; // Large enough for beacon frame
    uint16_t frame_size = generateBeaconFrame(frame_buffer, sizeof(frame_buffer));
    
    if (frame_size == 0) {
        return false;
    }
    
    // Send the frame using platform-specific function
    bool sent = ::sendBeaconFrame(frame_buffer, frame_size);
    
    if (sent) {
        _frame_count++;
        _last_send_ms = millis();
        _last_send_position = _last_gps_data.position;
        
        // Update DroneIdFr that we sent a frame
        _drone_id_fr.setLastSend();
    }
    
    return sent;
}

bool BeaconManager::shouldSend() {
    if (!_initialized) {
        return false;
    }
    
    if (!_home_set) {
        return false;
    }
    
    // Check if GPS data is valid
    if (!checkGpsDataValidity(_last_gps_data)) {
        return false;
    }
    
    // Check if DroneIdFr says it's time to send
    return _drone_id_fr.timeToSend();
}

// ================================================================================================
// STATUS AND STATISTICS
// ================================================================================================

bool BeaconManager::isReady() const {
    return _initialized && _home_set && _frame_count > 0;
}

bool BeaconManager::hasHomeSet() const {
    return _home_set;
}

GpsCoordinates BeaconManager::getLastSendPosition() const {
    return _last_send_position;
}

// ================================================================================================
// GPS STATUS CHECKS
// ================================================================================================

bool BeaconManager::isGpsValid() const {
    return checkGpsDataValidity(_last_gps_data);
}

String BeaconManager::getGpsStatusString() const {
    if (!checkGpsDataValidity(_last_gps_data)) {
        if (!_last_gps_data.has_fix) {
            return "NO FIX";
        }
        if (_last_gps_data.satellites < 4) {
            return "FEW SATELLITES";
        }
        if (_last_gps_data.hdop > 5.0) {
            return "POOR PRECISION";
        }
        if (_last_gps_data.altitude == 0.0) {
            return "NO ALTITUDE";
        }
    }
    return "OK";
}

// ================================================================================================
// UTILITY FUNCTIONS
// ================================================================================================

void BeaconManager::reset() {
    _frame_count = 0;
    _last_send_ms = 0;
    _home_set = false;
    _home_altitude = 0.0;
    _last_gps_data.reset();
    _last_send_position.latitude = 0.0;
    _last_send_position.longitude = 0.0;
}

String BeaconManager::getStateString() const {
    char buffer[128];
    snprintf(buffer, sizeof(buffer),
        "Beacon: %s, Frames: %u, Home: %s, GPS: %s",
        _initialized ? "INIT" : "NOT INIT",
        _frame_count,
        _home_set ? "SET" : "NOT SET",
        getGpsStatusString().c_str()
    );
    return String(buffer);
}

// ================================================================================================
// INTERNAL FUNCTIONS
// ================================================================================================

bool BeaconManager::checkGpsDataValidity(const GpsData& gps_data) const {
    // Check basic validity
    if (!gps_data.has_fix) {
        return false;
    }
    
    // Check coordinates
    if (gps_data.position.latitude == 0.0 || gps_data.position.longitude == 0.0) {
        return false;
    }
    
    // DGAC requirements:
    // - At least 4 satellites
    if (gps_data.satellites < 4) {
        return false;
    }
    
    // - HDOP < 5.0
    if (gps_data.hdop > 5.0 || gps_data.hdop == 0.0) {
        return false;
    }
    
    // - Valid altitude
    if (gps_data.altitude == 0.0) {
        return false;
    }
    
    return true;
}

void BeaconManager::updateDroneIdFr(const GpsData& gps_data) {
    // Update current position
    _drone_id_fr.setCurrentPosition(
        gps_data.position.latitude,
        gps_data.position.longitude,
        static_cast<int16_t>(gps_data.altitude)
    );
    
    // Update speed (convert from m/s to the internal representation)
    _drone_id_fr.setGroundSpeed(gps_data.speed);
    
    // Update heading
    if (gps_data.heading >= 0.0) {
        _drone_id_fr.setHeading(static_cast<uint16_t>(gps_data.heading));
    }
    
    // Note: Height is calculated automatically by DroneIdFr
}
