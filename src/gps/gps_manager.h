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

#ifndef GPS_MANAGER_H
#define GPS_MANAGER_H

#include <Arduino.h>
#include "gps_parser.h"
#include "../core/biadud_core.h"
#include "../core/biadud_platform.h"

// Include the appropriate GPS implementation based on GPS_TYPE
#if GPS_TYPE == GPS_TYPE_UBLOX
    #include "gps_ublox.h"
#elif GPS_TYPE == GPS_TYPE_QUECTEL
    // TODO: Add Quectel GPS support
    #include "gps_quectel.h"
#else
    // No specific GPS type - use basic parser only
    #define GPS_MANAGER_BASIC_ONLY
#endif

// ================================================================================================
// GPS MANAGER CLASS
// Unified interface for all GPS modules
// ================================================================================================

class GpsManager
{
public:
    // -------------------------------------------------------------------------------------------
    // CONSTRUCTOR / DESTRUCTOR
    // -------------------------------------------------------------------------------------------
    
    GpsManager();
    ~GpsManager();

    // -------------------------------------------------------------------------------------------
    // INITIALIZATION
    // -------------------------------------------------------------------------------------------

    /**
     * Initialize the GPS module
     * 
     * @param baud_rate Desired baud rate
     * @param refresh_rate Desired refresh rate in Hz
     * @return true if initialization was successful
     */
    bool initialize(uint32_t baud_rate, uint8_t refresh_rate);

    // -------------------------------------------------------------------------------------------
    // GPS DATA ACCESS
    // -------------------------------------------------------------------------------------------

    /**
     * Get the current GPS data
     * 
     * @return Const reference to the GPS data
     */
    const GpsData& getData() const;

    /**
     * Check if GPS has valid data
     * 
     * @return true if valid fix is available
     */
    bool hasValidFix() const;

    /**
     * Check if GPS has enough satellites for a valid fix
     * A quality threshold, not a regulatory one: at least 4 satellites
     * 
     * @return true if enough satellites are available
     */
    bool hasEnoughSatellites() const;

    /**
     * Check if GPS has good precision (HDOP < 5.0)
     * 
     * @return true if HDOP is acceptable
     */
    bool hasGoodPrecision() const;

    /**
     * Check if GPS altitude is valid
     * 
     * @return true if altitude is valid
     */
    bool hasValidAltitude() const;

    // -------------------------------------------------------------------------------------------
    // GPS STATUS
    // -------------------------------------------------------------------------------------------

    /**
     * Get the current GPS status
     * 
     * @return Current GPS status
     */
    GpsStatus getStatus() const;

    /**
     * Check if GPS is ready
     * 
     * @return true if GPS is initialized and ready
     */
    bool isReady() const;

    // -------------------------------------------------------------------------------------------
    // UPDATE GPS DATA
    // Call this regularly to update GPS data from serial port
    // -------------------------------------------------------------------------------------------

    /**
     * Update GPS data by reading from serial port
     * Call this in the main loop
     */
    void update();

    /**
     * Process a single character from GPS
     * 
     * @param c The character to process
     */
    void processChar(char c);

    // -------------------------------------------------------------------------------------------
    // SERIAL DATA PROCESSING
    // -------------------------------------------------------------------------------------------

    /**
     * Process incoming serial data from GPS (buffer version)
     * 
     * @param buffer Pointer to the data buffer
     * @param length Number of bytes to process
     */
    void processSerialData(const char* buffer, uint16_t length);

    // -------------------------------------------------------------------------------------------
    // DIAGNOSTIC FUNCTIONS
    // -------------------------------------------------------------------------------------------

    /**
     * Echo GPS data to debug serial for a specified duration
     * 
     * @param duration_ms Duration to echo data in milliseconds
     */
    void echoGpsData(uint32_t duration_ms);

    /**
     * Get the number of sentences parsed
     * 
     * @return Sentence count
     */
    uint32_t getSentenceCount() const;

    /**
     * Get the number of failed checksums
     * 
     * @return Failed checksum count
     */
    uint32_t getFailedChecksumCount() const;

    /**
     * Get the last parsed sentence type as string
     * 
     * @return String representation of the last sentence type
     */
    String getLastSentenceType() const;

    /**
     * Outcome of the receiver navigation settings (u-blox UBX-CFG-NAV5)
     *
     * @return "ACK", "NAK", "no answer", "not sent", or "n/a" for another receiver
     */
    const char* getNavigationStatus() const;

    // -------------------------------------------------------------------------------------------
    // CONFIGURATION
    // -------------------------------------------------------------------------------------------

    /**
     * Enable/disable parsing of a specific NMEA sentence type
     * 
     * @param sentence_type The NMEA sentence type to enable/disable
     * @param enabled true to enable, false to disable
     */
    void enableSentence(NmeaSentenceType sentence_type, bool enabled);

    /**
     * Set the baud rate
     * 
     * @param baud_rate Desired baud rate
     */
    void setBaudRate(uint32_t baud_rate);

    /**
     * Set the refresh rate
     * 
     * @param rate Refresh rate in Hz
     */
    void setRefreshRate(uint8_t rate);

    // -------------------------------------------------------------------------------------------
    // SAVE HOME POSITION
    // -------------------------------------------------------------------------------------------

    /**
     * Save the current position as home position
     * Call this when the drone first gets a GPS fix
     */
    void saveHomePosition();

    /**
     * Check if home position has been saved
     * 
     * @return true if home position is saved
     */
    bool hasHomePosition() const;

    /**
     * Get the home position
     * 
     * @return The home position coordinates
     */
    GpsCoordinates getHomePosition() const;

private:
    // -------------------------------------------------------------------------------------------
    // INTERNAL DATA
    // -------------------------------------------------------------------------------------------

    // GPS data storage
    GpsData _data;
    bool _has_home_position = false;
    GpsCoordinates _home_position;

    // GPS parser
    NmeaParser _parser;

    // GPS status
    GpsStatus _status = GpsStatus::GPS_NOT_DETECTED;
    bool _initialized = false;
    
    #if !defined(GPS_MANAGER_BASIC_ONLY)
        // Specific GPS implementation
        #if GPS_TYPE == GPS_TYPE_UBLOX
            UbloxGpsManager _gps_impl;
        #elif GPS_TYPE == GPS_TYPE_QUECTEL
            QuectelGpsManager _gps_impl;
        #endif
    #endif
};

// Global GPS manager instance
extern GpsManager gpsManager;

#endif // #ifndef GPS_MANAGER_H
