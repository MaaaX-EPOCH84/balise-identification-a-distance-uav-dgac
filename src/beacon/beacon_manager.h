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
     * @param ssid     SSID advertised by the beacon frame, 32 characters max
     * @param channel  Channel announced in the DS parameter set
     */
    void initialize(const char* drone_id, const char* ssid = nullptr, uint8_t channel = 1);

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
     * Call this when new GPS data is available. Without a live fix — fix lost, or the caller
     * clearing has_fix because the GPS went mute — the last live position is fed to the frame
     * instead, with a zero ground speed and the route held.
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
     * Check if beacon is ready to send: initialized, home set and a position known. A lost fix
     * does not make it unready, the last position is sent instead.
     * 
     * @return true if beacon is ready
     */
    bool isReady() const;

    /**
     * Check if the position sent is the last live one, held because the fix is gone
     *
     * @return true while the position is frozen
     */
    bool isPositionFrozen() const { return _position_frozen; }

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
    // Fix quality report, for diagnosis: none of it gates the emission
    // -------------------------------------------------------------------------------------------

    /**
     * Check if the last GPS data has a good quality fix
     * 
     * @return true with a fix, 4+ satellites, HDOP up to 5.0 and an altitude
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

    /**
     * Get transmission counters, for diagnosis only
     * Separates a frame refused by shouldSend() from one never built, and from
     * one the radio declined to send.
     *
     * @return Counters as a printable string
     */
    String getTxStatsString() const;

    /**
     * Radio refusals since the last accepted frame. Reset by any successful send.
     */
    uint32_t getConsecutiveTxErrors() const { return _consecutive_tx_errors; }

private:
    // -------------------------------------------------------------------------------------------
    // INTERNAL DATA
    // -------------------------------------------------------------------------------------------

    DroneIdFr _drone_id_fr;
    GpsData _last_gps_data;
    GpsData _last_live_gps_data;     // Last data with a fix, sent again while the fix is lost
    
    // Beacon frame identity
    char _ssid[33] = {0};
    uint8_t _channel = 1;

    // Transmission diagnostics
    uint32_t _attempt_count = 0;
    uint32_t _refused_count = 0;
    uint32_t _empty_frame_count = 0;
    uint32_t _tx_error_count = 0;
    uint32_t _consecutive_tx_errors = 0;
    uint32_t _interval_max_ms = 0;     // Longest gap between two sent frames
    uint32_t _interval_over_count = 0; // Gaps above the 3 s limit
    uint16_t _last_frame_size = 0;

    // Beacon statistics
    uint32_t _frame_count = 0;
    uint32_t _last_send_ms = 0;
    GpsCoordinates _last_send_position;
    
    // State
    bool _initialized = false;
    bool _home_set = false;
    double _home_altitude = 0.0;
    bool _heading_tracking = false;  // Speed high enough for the course to mean something
    bool _position_known = false;    // A live position was fed to the frame at least once
    bool _position_frozen = false;   // The frame holds the last live position, fix lost

    // -------------------------------------------------------------------------------------------
    // INTERNAL FUNCTIONS
    // -------------------------------------------------------------------------------------------

    /**
     * Check if GPS data has a good quality fix. The order of 27 December 2019 sets no such
     * threshold: this is a quality report, not an emission condition.
     * 
     * @param gps_data The GPS data to check
     * @return true with a fix, 4+ satellites, HDOP up to 5.0 and an altitude
     */
    bool checkGpsDataValidity(const GpsData& gps_data) const;

    /**
     * Check if GPS data carries a live position: a fix and non null coordinates
     *
     * @param gps_data The GPS data to check
     * @return true if the position can be sent as current
     */
    static bool hasLivePosition(const GpsData& gps_data);

    /**
     * Write the 802.11 beacon header — management frame, addresses, fixed parameters,
     * DS parameter set and SSID element — ahead of the vendor specific element.
     * esp_wifi_80211_tx() expects a complete frame, not a bare information element.
     *
     * @param frame Output buffer
     * @return Number of bytes written, where the vendor element starts
     */
    uint16_t buildBeaconHeader(uint8_t* frame) const;

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
