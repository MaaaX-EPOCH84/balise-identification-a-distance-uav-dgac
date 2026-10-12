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

#include "drone_id_fr.h"
#include <cstring>
#include <cmath>

// ================================================================================================
// CONSTRUCTOR / DESTRUCTOR
// ================================================================================================

DroneIdFr::DroneIdFr() {
    // Initialize drone ID with default value
    setDroneId(UAV_ID_DEFAULT_TEMPLATE);
    
    // Initialize last sent position
    _last_latitude_sent = 0;
    _last_longitude_sent = 0;
    _last_altitude_sent = 0;
    
    // Initialize timing
    _last_send_ms = millis();
    _last_data_rcv_ms = millis();
}

DroneIdFr::~DroneIdFr() {
    // Nothing to clean up
}

// ================================================================================================
// SETTERS FOR CURRENT POSITION
// ================================================================================================

void DroneIdFr::setCurrentPosition(int32_t lat, int32_t lon, int16_t alt) {
    _latitude = lat;
    _longitude = lon;
    _altitude = alt;

    _new_GPS_data = true;
    
    // Calculate time since last GPS data
    uint32_t current_ms = millis();
    double time_period = static_cast<double>(current_ms - _last_data_rcv_ms) / 1000.0;
    _last_data_rcv_ms = current_ms;
    
    // Update average GPS data period (exponential moving average)
    if (_data_period_GPS == 0.0) {
        _data_period_GPS = time_period;
    } else {
        _data_period_GPS = (_data_period_GPS * 0.8) + (time_period * 0.2);
    }

    // If home is set, update height above home
    if (_home_set) {
        _height = _altitude - _home_altitude;

        // Calculate 2D distance from previous position
        _2D_distance = distanceBetween(
            static_cast<double>(lat) * 1e-7, 
            static_cast<double>(lon) * 1e-7,
            static_cast<double>(_last_latitude_sent) * 1e-7,
            static_cast<double>(_last_longitude_sent) * 1e-7
        );

        // Calculate 3D distance from last position sent
        _3D_distance_from_last_point_sent = std::sqrt(
            (_2D_distance * _2D_distance) + 
            ((_last_altitude_sent - _altitude) * (_last_altitude_sent - _altitude))
        );
    }
}

void DroneIdFr::setCurrentPosition(double lat, double lon, int16_t alt) {
    // Convert to internal format (degrees * 1e7)
    setCurrentPosition(
        static_cast<int32_t>(lat * 1e7),
        static_cast<int32_t>(lon * 1e7),
        alt
    );
}

// ================================================================================================
// SETTERS FOR HOME POSITION
// ================================================================================================

void DroneIdFr::setHomePosition(int32_t lat, int32_t lon, int16_t alt) {
    _home_latitude = lat;
    _home_longitude = lon;
    _home_altitude = alt;

    // Initialize last sent position with home position
    _last_latitude_sent = _home_latitude;
    _last_longitude_sent = _home_longitude;
    _last_altitude_sent = _home_altitude;

    _home_set = true;
    
    // Calculate initial height
    if (_altitude != 0) {
        _height = _altitude - _home_altitude;
    } else {
        _height = 0;
    }
}

void DroneIdFr::setHomePosition(double lat, double lon, int16_t alt) {
    // Convert to internal format (degrees * 1e7)
    setHomePosition(
        static_cast<int32_t>(lat * 1e7),
        static_cast<int32_t>(lon * 1e7),
        alt
    );
}

// ================================================================================================
// SETTERS FOR OTHER GPS DATA
// ================================================================================================

void DroneIdFr::setAltitude(int16_t alt) {
    _altitude = alt;
    
    if (_home_set) {
        _height = _altitude - _home_altitude;
    }
}

void DroneIdFr::setHeight(int16_t height) {
    _height = height;
}

void DroneIdFr::setGroundSpeed(double ground_speed) {
    _ground_speed = ground_speed;
}

void DroneIdFr::setHeading(uint16_t heading) {
    _heading = heading;
}

// ================================================================================================
// SETTER FOR DRONE ID
// ================================================================================================

void DroneIdFr::setDroneId(const char* id_value) {
    if (id_value == nullptr) {
        memset(_droneID, 0, sizeof(_droneID));
        return;
    }
    
    // Copy drone ID, ensuring it's null-terminated and doesn't exceed buffer size
    size_t copy_size = std::min(strlen(id_value), static_cast<size_t>(TLV_LENGTH[ID_FR]));
    memcpy(_droneID, id_value, copy_size);
    
    // Null-terminate and ensure full buffer is cleared
    _droneID[TLV_LENGTH[ID_FR]] = 0; // Ensure null termination
}

// ================================================================================================
// GETTERS
// ================================================================================================

double DroneIdFr::getDistanceFromLastPositionSent() const {
    return _3D_distance_from_last_point_sent;
}

double DroneIdFr::getGroundSpeedKmh() const {
    return _ground_speed * 3.6; // Convert m/s to km/h
}

// ================================================================================================
// FRAME GENERATION
// ================================================================================================

uint16_t DroneIdFr::generateBeaconFrame(uint8_t* full_frame, uint16_t start_from) {
    // Vendor specific 802.11 beacon frame
    full_frame[start_from] = FRAME_VS;
    start_from++;
    const uint16_t payload_marker = start_from;
    start_from++;

    // Add OUI
    for (uint8_t i = 0; i < 3; i++) {
        full_frame[start_from] = FRAME_OUI[i];
        start_from++;
    }
    
    // Add VS TYPE
    full_frame[start_from] = FRAME_VS_TYPE;
    start_from++;
    
    // Generate drone payload and get its size
    const uint8_t payload_size = generateDroneFrame(full_frame, start_from);
    
    // Set payload size (includes OUI + VS_TYPE + drone payload)
    full_frame[payload_marker] = payload_size + 4; // +4 for OUI (3) + VSTYPE (1)
    
    // Return total bytes written
    return start_from + payload_size;
}

uint8_t DroneIdFr::generateDroneFrame(uint8_t* full_frame, uint16_t start_from) {
    uint8_t count = 0;
    
    // PROTOCOL VERSION
    full_frame[start_from + count] = static_cast<uint8_t>(PROTOCOL_VERSION);
    count++;
    full_frame[start_from + count] = TLV_LENGTH[PROTOCOL_VERSION];
    count++;
    full_frame[start_from + count] = FRAME_VS_TYPE;
    count++;
    
    // DRONE ID FR
    full_frame[start_from + count] = static_cast<uint8_t>(ID_FR);
    count++;
    full_frame[start_from + count] = TLV_LENGTH[ID_FR];
    count++;
    for (uint8_t i = 0; i < TLV_LENGTH[ID_FR]; i++) {
        full_frame[start_from + count] = _droneID[i];
        count++;
    }
    
    // LATITUDE
    full_frame[start_from + count] = static_cast<uint8_t>(LATITUDE);
    count++;
    full_frame[start_from + count] = TLV_LENGTH[LATITUDE];
    count++;
    for (int8_t i = TLV_LENGTH[LATITUDE] - 1; i >= 0; i--) {
        full_frame[start_from + count] = (get2Complement(toFrameCoordinate(_latitude)) >> (8 * i)) & 0xFF;
        count++;
    }
    
    // LONGITUDE
    full_frame[start_from + count] = static_cast<uint8_t>(LONGITUDE);
    count++;
    full_frame[start_from + count] = TLV_LENGTH[LONGITUDE];
    count++;
    for (int8_t i = TLV_LENGTH[LONGITUDE] - 1; i >= 0; i--) {
        full_frame[start_from + count] = (get2Complement(toFrameCoordinate(_longitude)) >> (8 * i)) & 0xFF;
        count++;
    }
    
    // ALTITUDE
    full_frame[start_from + count] = static_cast<uint8_t>(ALTITUDE);
    count++;
    full_frame[start_from + count] = TLV_LENGTH[ALTITUDE];
    count++;
    for (int8_t i = TLV_LENGTH[ALTITUDE] - 1; i >= 0; i--) {
        full_frame[start_from + count] = (get2Complement(_altitude) >> (8 * i)) & 0xFF;
        count++;
    }
    
    // HEIGHT
    full_frame[start_from + count] = static_cast<uint8_t>(HEIGTH);
    count++;
    full_frame[start_from + count] = TLV_LENGTH[HEIGTH];
    count++;
    for (int8_t i = TLV_LENGTH[HEIGTH] - 1; i >= 0; i--) {
        full_frame[start_from + count] = (get2Complement(_height) >> (8 * i)) & 0xFF;
        count++;
    }
    
    // HOME LATITUDE
    full_frame[start_from + count] = static_cast<uint8_t>(HOME_LATITUDE);
    count++;
    full_frame[start_from + count] = TLV_LENGTH[HOME_LATITUDE];
    count++;
    for (int8_t i = TLV_LENGTH[HOME_LATITUDE] - 1; i >= 0; i--) {
        full_frame[start_from + count] = (get2Complement(toFrameCoordinate(_home_latitude)) >> (8 * i)) & 0xFF;
        count++;
    }
    
    // HOME LONGITUDE
    full_frame[start_from + count] = static_cast<uint8_t>(HOME_LONGITUDE);
    count++;
    full_frame[start_from + count] = TLV_LENGTH[HOME_LONGITUDE];
    count++;
    for (int8_t i = TLV_LENGTH[HOME_LONGITUDE] - 1; i >= 0; i--) {
        full_frame[start_from + count] = (get2Complement(toFrameCoordinate(_home_longitude)) >> (8 * i)) & 0xFF;
        count++;
    }
    
    // GROUND SPEED
    full_frame[start_from + count] = static_cast<uint8_t>(GROUND_SPEED);
    count++;
    full_frame[start_from + count] = TLV_LENGTH[GROUND_SPEED];
    count++;
    full_frame[start_from + count] = static_cast<uint8_t>(std::round(_ground_speed));
    count++;
    
    // HEADING
    full_frame[start_from + count] = static_cast<uint8_t>(HEADING);
    count++;
    full_frame[start_from + count] = TLV_LENGTH[HEADING];
    count++;
    for (int8_t i = TLV_LENGTH[HEADING] - 1; i >= 0; i--) {
        full_frame[start_from + count] = (get2Complement(static_cast<int16_t>(_heading)) >> (8 * i)) & 0xFF;
        count++;
    }
    
    return count;
}

// ================================================================================================
// LAST SEND STATE MANAGEMENT
// ================================================================================================

void DroneIdFr::setLastSend() {
    // Update last send timestamp
    _last_send_ms = millis();
    
    // Save current position as last sent
    _last_latitude_sent = _latitude;
    _last_longitude_sent = _longitude;
    _last_altitude_sent = _altitude;
    
    // Reset 3D distance (will be recalculated on next position update)
    _3D_distance_from_last_point_sent = 0.0;
}

// ================================================================================================
// CONDITION CHECKS FOR SENDING
// ================================================================================================

bool DroneIdFr::hasPassTime() const {
    // Check if 3 seconds have passed since last send
    // Use millis() for ESP32-C3 compatibility
    return hasElapsed(_last_send_ms, FRAME_TIME_LIMIT_MS);
}

bool DroneIdFr::hasPassDistance() const {
    _predicted_distance = 0.0;
    
    // If moving fast enough, predict future distance
    // Speed threshold: FRAME_DISTANCE_LIMIT / FRAME_TIME_LIMIT
    double speed_threshold = FRAME_DISTANCE_LIMIT_M / (FRAME_TIME_LIMIT_MS / 1000.0);
    
    if (_ground_speed >= speed_threshold) {
        // Predict distance with 10% margin
        _predicted_distance = _data_period_GPS * _ground_speed * 1.1;
    }
    
    // Check if predicted distance + current distance exceeds limit
    return (_3D_distance_from_last_point_sent + _predicted_distance) >= FRAME_DISTANCE_LIMIT_M;
}

bool DroneIdFr::timeToSend() {
    bool current_status = _new_GPS_data;
    
    if (current_status) {
        // Reset the flag
        _new_GPS_data = false;
    }
    
    // Send if we have new data and (time or distance limit exceeded)
    return current_status && (hasPassTime() || hasPassDistance());
}

// ================================================================================================
// POSITION GETTERS
// ================================================================================================

String DroneIdFr::getHomePosition() const {
    char buffer[64];
    snprintf(buffer, sizeof(buffer), 
        "Latitude: %.7f, Longitude: %.7f, Altitude: %.1f",
        static_cast<double>(_home_latitude) / 1e7,
        static_cast<double>(_home_longitude) / 1e7,
        static_cast<double>(_home_altitude)
    );
    return String(buffer);
}

String DroneIdFr::getCurrentPosition() const {
    char buffer[128];
    snprintf(buffer, sizeof(buffer),
        "Lat: %.7f, Lon: %.7f, Alt: %.1f, Height: %.1f, 2D: %.1f, 3D: %.1f, Heading: %.1f",
        static_cast<double>(_latitude) / 1e7,
        static_cast<double>(_longitude) / 1e7,
        static_cast<double>(_altitude),
        static_cast<double>(_height),
        _2D_distance,
        _3D_distance_from_last_point_sent,
        static_cast<double>(_heading)
    );
    return String(buffer);
}

// ================================================================================================
// PRIVATE HELPER FUNCTIONS
// ================================================================================================

double DroneIdFr::distanceBetween(double lat1, double lon1, double lat2, double lon2) {
    // Calculate distance in meters between two positions, both specified
    // as signed decimal-degrees latitude and longitude.
    // Uses great-circle distance computation for hypothetical sphere of radius 6372795 meters.
    // Because Earth is not an exact sphere, rounding errors may be up to 0.5%.
    // Courtesy of Maarten Lamers
    
    double delta = radians(lon1 - lon2);
    double sdlong = sin(delta);
    double cdlong = cos(delta);
    
    lat1 = radians(lat1);
    lat2 = radians(lat2);
    
    double slat1 = sin(lat1);
    double clat1 = cos(lat1);
    double slat2 = sin(lat2);
    double clat2 = cos(lat2);
    
    delta = (clat1 * slat2) - (slat1 * clat2 * cdlong);
    delta = delta * delta;
    delta += sq(clat2 * sdlong);
    delta = sqrt(delta);
    
    double denom = (slat1 * slat2) + (clat1 * clat2 * cdlong);
    delta = atan2(delta, denom);
    
    return fabs(delta * 6372795.0);
}
