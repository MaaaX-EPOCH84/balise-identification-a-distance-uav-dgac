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

#ifndef BEACON_MANAGER_H
#define BEACON_MANAGER_H

#include <Arduino.h>
#include "drone_id_fr.h"
#include "../gps/gps_manager.h"
#include "../core/biadud_platform.h"

// ================================================================================================
// BEACON MANAGER CLASS
// Manages the generation and transmission of DGAC beacon frames
// ================================================================================================

class BeaconManager
{
public:
    // -------------------------------------------------------------------------------------------
    // CONSTRUCTOR / DESTRUCTOR
    // -------------------------------------------------------------------------------------------
    
    BeaconManager();
    ~BeaconManager();

    // -------------------------------------------------------------------------------------------
    // INITIALIZATION
    // -------------------------------------------------------------------------------------------

    /**
     * Initialize the beacon manager
     * 
     * @param drone_id The drone ID string (30 characters max)
     */
    void initialize(const char* drone_id);

    // -------------------------------------------------------------------------------------------
    // SETTERS
    // -------------------------------------------------------------------------------------------

    /**
     * Set the drone ID
     * 
     * @param drone_id The drone ID string
     */
    void setDroneId(const char* drone_id);

    /**
     * Set the home position
     * 
     * @param lat Latitude in decimal degrees
     * @param lon Longitude in decimal degrees
     * @param alt Altitude in meters
     */
    void setHomePosition(double lat, double lon, double alt);

    // -------------------------------------------------------------------------------------------
    // UPDATE GPS DATA
    // -------------------------------------------------------------------------------------------

    /**
     * Update beacon with new GPS data
     * Call this when new GPS data is available
     * 
     * @param gps_data The GPS data
     */
    void updateGpsData(const GpsData& gps_data);

    // -------------------------------------------------------------------------------------------
    // BEACON FRAME GENERATION AND TRANSMISSION
    // -------------------------------------------------------------------------------------------

    /**
     * Generate a beacon frame with current data
     * 
     * @param buffer Output buffer for the beacon frame
     * @param buffer_size Size of the output buffer
     * @return Number of bytes written to the buffer
     */
    uint16_t generateBeaconFrame(uint8_t* buffer, uint16_t buffer_size);

    /**
     * Send the beacon frame
     * This will generate and transmit the frame via WiFi
     * 
     * @return true if frame was sent successfully
     */
    bool sendBeaconFrame();

    /**
     * Check if it's time to send a new beacon frame
     * This checks both time-based and distance-based conditions
     * 
     * @return true if a new frame should be sent
     */
    bool shouldSend();

    // -------------------------------------------------------------------------------------------
    // STATUS AND STATISTICS
    // -------------------------------------------------------------------------------------------

    /**
     * Check if beacon is ready to send
     * 
     * @return true if beacon is ready
     */
    bool isReady() const;

    /**
     * Check if home position is set
     * 
     * @return true if home position is set
     */
    bool hasHomeSet() const;

    /**
     * Get the number of beacon frames sent
     * 
     * @return Frame count
     */
    uint32_t getFrameCount() const { return _frame_count; }

    /**
     * Get the last send time
     * 
     * @return Last send timestamp in milliseconds
     */
    uint32_t getLastSendTime() const { return _last_send_ms; }

    /**
     * Get the last send position
     * 
     * @return The last sent position
     */
    GpsCoordinates getLastSendPosition() const;

    // -------------------------------------------------------------------------------------------
    // GPS STATUS CHECKS
    // These check if GPS data meets DGAC requirements
    // -------------------------------------------------------------------------------------------

    /**
     * Check if GPS data is valid for beacon transmission
     * 
     * @return true if GPS data meets DGAC requirements
     */
    bool isGpsValid() const;

    /**
     * Get the current GPS status as a string
     * 
     * @return String describing GPS status
     */
    String getGpsStatusString() const;

    // -------------------------------------------------------------------------------------------
    // UTILITY FUNCTIONS
    // -------------------------------------------------------------------------------------------

    /**
     * Reset the beacon manager
     */
    void reset();

    /**
     * Get a string representation of the current beacon state
     * 
     * @return State string
     */
    String getStateString() const;

private:
    // -------------------------------------------------------------------------------------------
    // INTERNAL DATA
    // -------------------------------------------------------------------------------------------

    DroneIdFr _drone_id_fr;
    GpsData _last_gps_data;
    
    // Beacon statistics
    uint32_t _frame_count = 0;
    uint32_t _last_send_ms = 0;
    GpsCoordinates _last_send_position;
    
    // State
    bool _initialized = false;
    bool _home_set = false;
    double _home_altitude = 0.0;

    // -------------------------------------------------------------------------------------------
    // INTERNAL FUNCTIONS
    // -------------------------------------------------------------------------------------------

    /**
     * Check if GPS data meets all DGAC requirements
     * 
     * @param gps_data The GPS data to check
     * @return true if data meets all requirements
     */
    bool checkGpsDataValidity(const GpsData& gps_data) const;

    /**
     * Update the DroneIdFr class with current GPS data
     * 
     * @param gps_data The GPS data to use
     */
    void updateDroneIdFr(const GpsData& gps_data);
};

// Global beacon manager instance
extern BeaconManager beaconManager;

#endif // #ifndef BEACON_MANAGER_H
