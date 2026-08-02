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

#ifndef GPS_UBLOX_H
#define GPS_UBLOX_H

#include <Arduino.h>
#include "gps_parser.h"
#include "../core/biadud_platform.h"

// ================================================================================================
// UBLOX GPS CONFIGURATION COMMANDS
// ================================================================================================

// UBX protocol sync characters
#define UBX_SYNC_CHAR_1    0xB5
#define UBX_SYNC_CHAR_2    0x62

// UBX Message Classes
#define UBX_CLASS_NAV       0x01
#define UBX_CLASS_RXM       0x02
#define UBX_CLASS_INF       0x04
#define UBX_CLASS_ACK       0x05
#define UBX_CLASS_CFG       0x06
#define UBX_CLASS_UPD       0x09
#define UBX_CLASS_MON       0x0A
#define UBX_CLASS_AID       0x0B
#define UBX_CLASS_TIM       0x0D
#define UBX_CLASS_ESF       0x10
#define UBX_CLASS_PWR       0x1E
#define UBX_CLASS_MGA       0x13
#define UBX_CLASS_LOG       0x21
#define UBX_CLASS_SEC       0x27
#define UBX_CLASS_HNR       0x28

// UBX Message IDs
#define UBX_MSG_NAV_PVT     0x07  // Navigation Position Velocity Time Solution
#define UBX_MSG_NAV_POSLLH  0x02  // Geodetic Position Solution
#define UBX_MSG_NAV_VELNED  0x12  // Velocity Solution in NED
#define UBX_MSG_NAV_STATUS  0x03  // Receiver Navigation Status
#define UBX_MSG_NAV_SAT     0x35  // Satellite Information

#define UBX_MSG_CFG_PRT     0x00  // Port Configuration
#define UBX_MSG_CFG_MSG     0x01  // Message Configuration
#define UBX_MSG_CFG_RATE    0x08  // Navigation/Measurement Rate Settings
#define UBX_MSG_CFG_CFG     0x09  // Clear, Save, and Load Configuration
#define UBX_MSG_CFG_GNSS    0x3E  // GNSS Configuration
#define UBX_MSG_CFG_NMEA    0x17  // NMEA Protocol Configuration
#define UBX_MSG_CFG_USB     0x1B  // USB Configuration

// ================================================================================================
// NMEA SENTENCE CONFIGURATION COMMANDS FOR UBLOX
// ================================================================================================

// Command to limit NMEA sentences to GGA and RMC only
#define UBX_NMEA_LIMIT_SENTENCES \
    "$PUBX,40,GSV,0,0,0,0,0,0*59\r\n" \
    "$PUBX,40,GSA,0,0,0,0,0,0*4E\r\n" \
    "$PUBX,40,GLL,0,0,0,0,0,0*5C\r\n" \
    "$PUBX,40,VTG,0,0,0,0,0,0*5E\r\n"

// ================================================================================================
// UBLOX GPS MANAGER CLASS
// ================================================================================================

class UbloxGpsManager
{
public:
    // -------------------------------------------------------------------------------------------
    // CONSTRUCTOR / DESTRUCTOR
    // -------------------------------------------------------------------------------------------
    
    UbloxGpsManager();
    ~UbloxGpsManager();

    // -------------------------------------------------------------------------------------------
    // INITIALIZATION
    // -------------------------------------------------------------------------------------------

    /**
     * Initialize the UBLOX GPS module
     * 
     * @param baud_rate Desired baud rate (9600, 19200, 38400, 57600, 115200)
     * @param refresh_rate Desired refresh rate in Hz (1-10)
     * @return true if initialization was successful
     */
    bool initialize(uint32_t baud_rate, uint8_t refresh_rate);

    /**
     * Set the baud rate of the GPS
     * This will try all common baud rates to send the command
     * 
     * @param baud_rate Desired baud rate
     */
    void setBaudRate(uint32_t baud_rate);

    /**
     * Set the refresh rate of the GPS
     * 
     * @param rate Refresh rate in Hz
     */
    void setRefreshRate(uint8_t rate);

    // -------------------------------------------------------------------------------------------
    // GPS DATA ACCESS
    // -------------------------------------------------------------------------------------------

    /**
     * Get the NMEA parser instance
     * 
     * @return Reference to the NMEA parser
     */
    NmeaParser& getParser() { return _parser; }

    /**
     * Get the current GPS data
     * 
     * @return Const reference to the GPS data
     */
    const GpsData& getData() const { return _parser.getData(); }

    /**
     * Check if GPS has valid data
     * 
     * @return true if valid fix is available
     */
    bool hasValidFix() const { return _parser.hasValidFix(); }

    // -------------------------------------------------------------------------------------------
    // GPS STATUS
    // -------------------------------------------------------------------------------------------

    /**
     * Get the current GPS status
     * 
     * @return Current GPS status
     */
    GpsStatus getStatus() const { return _status; }

    /**
     * Check if GPS is ready
     * 
     * @return true if GPS is initialized and ready
     */
    bool isReady() const { return _initialized; }

    // -------------------------------------------------------------------------------------------
    // SERIAL DATA PROCESSING
    // -------------------------------------------------------------------------------------------

    /**
     * Process incoming serial data from GPS
     * Call this for each character received from the GPS
     * 
     * @param c The character to process
     */
    void processSerialData(char c);

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
     * Useful for debugging GPS communication
     * 
     * @param duration_ms Duration to echo data in milliseconds
     */
    void echoGpsData(uint32_t duration_ms);

    /**
     * Send a raw command to the GPS
     * 
     * @param command The command string to send
     */
    void sendCommand(const char* command);

    /**
     * Send a UBX packet to the GPS
     * 
     * @param packet Pointer to the UBX packet data
     * @param length Length of the packet
     */
    void sendUbxPacket(const uint8_t* packet, uint16_t length);

    // -------------------------------------------------------------------------------------------
    // UTILITY FUNCTIONS
    // -------------------------------------------------------------------------------------------

    /**
     * Calculate UBX checksum for a packet
     * The packet should already have the sync chars (0xB5, 0x62) and the payload
     * This function will calculate and append the checksum to the packet
     * 
     * @param packet Pointer to the packet (will be modified to include checksum)
     * @param length Total length of the packet including header (excluding checksum)
     */
    static void calculateUbxChecksum(uint8_t* packet, uint16_t length);

    /**
     * Add NMEA checksum to a command string
     * The buffer must have at least 3 extra bytes for the *hh and null terminator
     * 
     * @param buffer The command buffer (will be modified)
     */
    static void addNmeaChecksum(char* buffer);

private:
    // -------------------------------------------------------------------------------------------
    // INTERNAL CONSTANTS
    // -------------------------------------------------------------------------------------------

    // Common baud rates to try when sending commands
    static constexpr uint32_t COMMON_BAUD_RATES[] = {
        9600, 19200, 38400, 57600, 115200
    };
    static constexpr uint8_t NUM_COMMON_BAUD_RATES = 5;

    // Default timeout for GPS commands
    static constexpr uint32_t GPS_COMMAND_TIMEOUT_MS = 1000;

    // -------------------------------------------------------------------------------------------
    // INTERNAL DATA
    // -------------------------------------------------------------------------------------------

    NmeaParser _parser;
    GpsStatus _status = GpsStatus::GPS_NOT_DETECTED;
    bool _initialized = false;
    uint32_t _current_baud = 0;
    uint8_t _current_rate = 0;

    // -------------------------------------------------------------------------------------------
    // INTERNAL FUNCTIONS
    // -------------------------------------------------------------------------------------------

    /**
     * Change the serial baud rate
     * Platform-specific implementation
     * 
     * @param baud_rate The new baud rate
     */
    void changeSerialBaudRate(uint32_t baud_rate);

    /**
     * Flush the serial buffer
     */
    void flushSerial();

    /**
     * Send configuration commands to limit NMEA sentences
     * This improves GPS performance by reducing unnecessary data
     */
    void configureNmeaSentences();

    /**
     * Send UBX configuration for measurement rate
     * 
     * @param rate Refresh rate in Hz
     */
    void configureUbxRate(uint8_t rate);

    /**
     * Wait for GPS to respond or timeout
     * 
     * @param timeout_ms Timeout in milliseconds
     * @return true if data was received, false on timeout
     */
    bool waitForResponse(uint32_t timeout_ms);
};

#endif // #ifndef GPS_UBLOX_H
