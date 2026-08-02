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

#ifndef DRONE_ID_FR_H
#define DRONE_ID_FR_H

#include <Arduino.h>
#include "../core/biadud_platform.h"

// ================================================================================================
// DRONE ID FR CLASS
// Implements the French drone identification frame as stated in
// https://www.legifrance.gouv.fr/eli/arrete/2019/12/27/ECOI1934044A/jo/texte
// ================================================================================================

class DroneIdFr
{
public:
    // -------------------------------------------------------------------------------------------
    // CONSTRUCTOR / DESTRUCTOR
    // -------------------------------------------------------------------------------------------
    
    DroneIdFr();
    ~DroneIdFr();

    // -------------------------------------------------------------------------------------------
    // SETTERS FOR CURRENT POSITION
    // -------------------------------------------------------------------------------------------

    /**
     * Set current position with GPS coordinates in integer centidegrees * 100 + altitude
     * This format is used by some GPS libraries (e.g., TinyGPS++ returns lat/lon in degrees * 1e7)
     * 
     * @param lat Latitude in WGS84 * 1e7 (degrees * 10^7)
     * @param lon Longitude in WGS84 * 1e7 (degrees * 10^7)
     * @param alt Altitude in meters (MSL)
     */
    void setCurrentPosition(int32_t lat, int32_t lon, int16_t alt);

    /**
     * Set current position with GPS coordinates in double degrees + altitude
     * Values are converted to internal integer representation.
     * 
     * @param lat Latitude in WGS84 decimal degrees
     * @param lon Longitude in WGS84 decimal degrees
     * @param alt Altitude in meters (MSL)
     */
    void setCurrentPosition(double lat, double lon, int16_t alt);

    // -------------------------------------------------------------------------------------------
    // SETTERS FOR HOME POSITION
    // -------------------------------------------------------------------------------------------

    /**
     * Set home position with GPS coordinates in integer centidegrees * 100 + altitude
     * This is typically called once when the drone first gets a GPS fix.
     * 
     * @param lat Latitude in WGS84 * 1e7
     * @param lon Longitude in WGS84 * 1e7
     * @param alt Altitude in meters (MSL)
     */
    void setHomePosition(int32_t lat, int32_t lon, int16_t alt);

    /**
     * Set home position with GPS coordinates in double degrees + altitude
     * Values are converted to internal integer representation.
     * 
     * @param lat Latitude in WGS84 decimal degrees
     * @param lon Longitude in WGS84 decimal degrees
     * @param alt Altitude in meters (MSL)
     */
    void setHomePosition(double lat, double lon, int16_t alt);

    // -------------------------------------------------------------------------------------------
    // SETTERS FOR OTHER GPS DATA
    // -------------------------------------------------------------------------------------------

    /**
     * Set altitude in MSL (Mean Sea Level)
     * 
     * @param alt Altitude in meters
     */
    void setAltitude(int16_t alt);

    /**
     * Set height above home position
     * 
     * @param height Height in meters
     */
    void setHeight(int16_t height);

    /**
     * Set ground speed
     * 
     * @param ground_speed Ground speed in m/s
     */
    void setGroundSpeed(double ground_speed);

    /**
     * Set heading (direction drone is pointing)
     * 
     * @param heading Heading in degrees from north (0-359)
     */
    void setHeading(uint16_t heading);

    /**
     * Set code info
     * 
     * @param codeinfo Code info value
     */
    void setCodeInfo(double codeinfo);

    // -------------------------------------------------------------------------------------------
    // SETTER FOR DRONE ID
    // -------------------------------------------------------------------------------------------

    /**
     * Set the drone ID
     * The ID is copied to internal buffer (max 30 characters)
     * If the provided ID is shorter than 30 chars, it's padded with nulls.
     * 
     * @param id_value Pointer to the drone ID string (max 30 chars)
     */
    void setDroneId(const char* id_value);

    // -------------------------------------------------------------------------------------------
    // GETTERS
    // -------------------------------------------------------------------------------------------

    /**
     * Get the distance from last position sent (3D distance)
     * 
     * @return Distance in meters
     */
    double getDistanceFromLastPositionSent() const;

    /**
     * Get ground speed in km/h
     * 
     * @return Speed in km/h
     */
    double getGroundSpeedKmh() const;

    /**
     * Get code info
     * 
     * @return Code info value
     */
    double getCodeInfo() const;

    /**
     * Get the drone ID string
     * 
     * @return Pointer to internal drone ID buffer
     */
    const char* getDroneId() const { return reinterpret_cast<const char*>(_droneID); }

    // -------------------------------------------------------------------------------------------
    // FRAME GENERATION
    // -------------------------------------------------------------------------------------------

    /**
     * Generate a complete 802.11 beacon frame with drone identification data
     * 
     * @param full_frame Output buffer for the beacon frame
     * @param start_from Starting offset in the buffer
     * @return Number of bytes written to the buffer
     */
    uint16_t generateBeaconFrame(uint8_t* full_frame, uint16_t start_from = 0);

    /**
     * Generate only the drone identification payload (TLV encoded)
     * This is used internally by generateBeaconFrame.
     * 
     * @param full_frame Output buffer for the payload
     * @param start_from Starting offset in the buffer
     * @return Number of bytes written to the buffer
     */
    uint8_t generateDroneFrame(uint8_t* full_frame, uint16_t start_from = 0);

    // -------------------------------------------------------------------------------------------
    // LAST SEND STATE MANAGEMENT
    // -------------------------------------------------------------------------------------------

    /**
     * Save the state of the last sent frame (position and time)
     * Called after successfully sending a beacon frame.
     */
    void setLastSend();

    // -------------------------------------------------------------------------------------------
    // CONDITION CHECKS FOR SENDING
    // -------------------------------------------------------------------------------------------

    /**
     * Check if home position is set
     * 
     * @return true if home position has been set
     */
    bool hasHomeSet() const { return _home_set; }

    /**
     * Check if time limit has been exceeded (3 seconds)
     * 
     * @return true if 3 seconds have passed since last send
     */
    bool hasPassTime() const;

    /**
     * Check if distance limit has been exceeded (30 meters)
     * Uses predicted distance for fast-moving drones.
     * 
     * @return true if drone has moved more than 30m from last sent position
     */
    bool hasPassDistance() const;

    /**
     * Check if it's time to send a new frame
     * Returns true if:
     * - New GPS data is available
     * - Time limit (3s) or distance limit (30m) has been exceeded
     * 
     * Note: This function also resets the new GPS data flag.
     * 
     * @return true if a new frame should be sent
     */
    bool timeToSend();

    // -------------------------------------------------------------------------------------------
    // HOME POSITION GETTER
    // -------------------------------------------------------------------------------------------

    /**
     * Get home position as string
     * 
     * @return String with home position info
     */
    String getHomePosition() const;

    /**
     * Get current position as string
     * 
     * @return String with current position info
     */
    String getCurrentPosition() const;

private:
    // -------------------------------------------------------------------------------------------
    // TIME LIMITS (DGAC REGULATION)
    // -------------------------------------------------------------------------------------------

    // Maximum time between two beacon frames: 3 seconds
    static constexpr uint32_t FRAME_TIME_LIMIT_MS = 3000;

    // Maximum distance between two beacon frames: 30 meters
    static constexpr double FRAME_DISTANCE_LIMIT_M = 30.0;

    // -------------------------------------------------------------------------------------------
    // ENUMERATION OF DATA TYPES (TLV encoding)
    // -------------------------------------------------------------------------------------------

    enum DataType : uint8_t
    {
        RESERVED = 0,
        PROTOCOL_VERSION = 1,
        ID_FR = 2,
        ID_ANSI_CTA = 3,
        LATITUDE = 4,        // In WGS84 in degree * 1e7
        LONGITUDE = 5,       // In WGS84 in degree * 1e7
        ALTITUDE = 6,        // In MSL in meters
        HEIGTH = 7,          // From Home in meters
        HOME_LATITUDE = 8,   // In WGS84 in degree * 1e7
        HOME_LONGITUDE = 9,  // In WGS84 in degree * 1e7
        GROUND_SPEED = 10,   // In m/s
        HEADING = 11,        // Heading in degree from north 0 to 359
        CODEINFO = 12,        // message info
        NOT_DEFINED_END = 13,
    };

    // -------------------------------------------------------------------------------------------
    // TLV LENGTHS FOR EACH DATA TYPE
    // -------------------------------------------------------------------------------------------

    static constexpr uint8_t TLV_LENGTH[]
    {
        0,  // [DataType::RESERVED]
        1,  // [DataType::PROTOCOL_VERSION]
        30, // [DataType::ID_FR]
        0,  // [DataType::ID_ANSI_CTA]
        4,  // [DataType::LATITUDE]
        4,  // [DataType::LONGITUDE]
        2,  // [DataType::ALTITUDE]
        2,  // [DataType::HEIGTH]
        4,  // [DataType::HOME_LATITUDE]
        4,  // [DataType::HOME_LONGITUDE]
        1,  // [DataType::GROUND_SPEED]
        2,  // [DataType::HEADING]
        1,  // [DataType::CODEINFO]
    };

    // -------------------------------------------------------------------------------------------
    // BEACON FRAME CONSTANTS
    // -------------------------------------------------------------------------------------------

    static constexpr uint8_t FRAME_VS = 0xDD;
    static constexpr uint8_t FRAME_VS_TYPE = 1;
    static constexpr uint8_t FRAME_OUI[3] = {0x6A, 0x5C, 0x35};

    // -------------------------------------------------------------------------------------------
    // INTERNAL DATA STORAGE
    // -------------------------------------------------------------------------------------------

    // GPS position data (stored in internal format: degrees * 1e7)
    int32_t _latitude;          // WGS84 latitude * 1e7
    int32_t _longitude;         // WGS84 longitude * 1e7
    int16_t _altitude;          // Altitude in meters (MSL)
    int16_t _height;            // Height above home in meters
    
    // Home position
    int32_t _home_latitude;     // WGS84 home latitude * 1e7
    int32_t _home_longitude;    // WGS84 home longitude * 1e7
    int16_t _home_altitude;     // Home altitude in meters
    
    // Navigation data
    double _ground_speed;        // Ground speed in m/s
    uint16_t _heading;          // Heading in degrees (0-359)
    double _codeinfo;           // Code info value
    
    // Drone ID (30 bytes + null terminator)
    uint8_t _droneID[TLV_LENGTH[ID_FR] + 1]; // +1 for null termination
    
    // Timing data (using millis() for ESP32-C3 compatibility)
    uint32_t _last_send_ms = 0;          // Timestamp of last frame sent
    uint32_t _last_data_rcv_ms = 0;      // Timestamp of last GPS data received
    double _data_period_GPS = 0.0;     // Average time between GPS updates
    
    // Distance tracking
    double _2D_distance = 0.0;                 // 2D distance from previous position
    double _3D_distance_from_last_point_sent = 0.0; // 3D distance from last sent position
    mutable double _predicted_distance = 0.0; // Predicted distance for fast-moving drones (scratch value computed in hasPassDistance())
    
    // Last sent position (for distance calculation)
    int32_t _last_latitude_sent;
    int32_t _last_longitude_sent;
    int16_t _last_altitude_sent;
    
    // State flags
    bool _home_set = false;              // Home position is set
    bool _new_GPS_data = false;          // New GPS data available

    // -------------------------------------------------------------------------------------------
    // PRIVATE HELPER FUNCTIONS
    // -------------------------------------------------------------------------------------------

    /**
     * Get 2's complement representation of int32_t value
     * Used for proper encoding of negative coordinates
     * 
     * @param value The value to convert
     * @return 32-bit unsigned representation
     */
    static inline uint32_t get2Complement(int32_t value) {
        return static_cast<uint32_t>(value) & 0xFFFFFFFF;
    }

    /**
     * Get 2's complement representation of int16_t value
     * 
     * @param value The value to convert
     * @return 16-bit unsigned representation
     */
    static inline uint16_t get2Complement(int16_t value) {
        return static_cast<uint16_t>(value) & 0xFFFF;
    }

    /**
     * Calculate distance between two WGS84 coordinates
     * Uses great-circle distance computation.
     * 
     * @param lat1 Latitude 1 in decimal degrees
     * @param lon1 Longitude 1 in decimal degrees
     * @param lat2 Latitude 2 in decimal degrees
     * @param lon2 Longitude 2 in decimal degrees
     * @return Distance in meters
     */
    static double distanceBetween(double lat1, double lon1, double lat2, double lon2);
};

#endif // #ifndef DRONE_ID_FR_H
