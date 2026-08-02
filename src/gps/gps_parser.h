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

#ifndef GPS_PARSER_H
#define GPS_PARSER_H

#include <Arduino.h>
#include "../core/biadud_core.h"

// ================================================================================================
// NMEA SENTENCE TYPES
// ================================================================================================

enum class NmeaSentenceType : uint8_t {
    NMEA_UNKNOWN = 0,
    NMEA_GGA,       // Global Positioning System Fix Data
    NMEA_GLL,       // Geographic Position - Latitude/Longitude
    NMEA_GSA,       // GPS DOP and Active Satellites
    NMEA_GSV,       // GPS Satellites in View
    NMEA_RMC,       // Recommended Minimum Specific GPS/Transit Data
    NMEA_VTG,       // Course Over Ground and Ground Speed
    NMEA_ZDA,       // Time & Date
    NMEA_PUBX,      // UBLOX proprietary sentences
};

// ================================================================================================
// GPS FIX QUALITY
// ================================================================================================

enum class GpsFixQuality : uint8_t {
    GPS_FIX_INVALID = 0,
    GPS_FIX_GPS = 1,         // Standard GPS fix
    GPS_FIX_DGPS = 2,        // Differential GPS fix
    GPS_FIX_PPS = 3,         // PPS fix
    GPS_FIX_RTK = 4,         // Real Time Kinematic fix
    GPS_FIX_FLOAT_RTK = 5,   // Float RTK fix
    GPS_FIX_ESTIMATED = 6,   // Estimated/Dead reckoning fix
    GPS_FIX_MANUAL = 7,      // Manual input mode
    GPS_FIX_SIMULATED = 8,   // Simulation mode
};

// ================================================================================================
// NMEA PARSER CLASS
// Lightweight NMEA-0183 parser that replaces TinyGPS++
// Only parses GGA and RMC sentences (minimum required for DGAC beacon)
// ================================================================================================

class NmeaParser
{
public:
    // -------------------------------------------------------------------------------------------
    // CONSTRUCTOR / DESTRUCTOR
    // -------------------------------------------------------------------------------------------
    
    NmeaParser();
    ~NmeaParser();

    // -------------------------------------------------------------------------------------------
    // PARSE FUNCTIONS
    // -------------------------------------------------------------------------------------------

    /**
     * Parse a single character from the GPS serial stream
     * Call this for each character received from the GPS
     * 
     * @param c The character to parse
     */
    void parse(char c);

    /**
     * Parse a complete NMEA sentence (null-terminated string)
     * Useful for testing or when reading from a buffer
     * 
     * @param sentence Pointer to the NMEA sentence string
     */
    void parseSentence(const char* sentence);

    // -------------------------------------------------------------------------------------------
    // DATA ACCESS
    // -------------------------------------------------------------------------------------------

    /**
     * Get the parsed GPS data
     * 
     * @return Const reference to the GPS data structure
     */
    const GpsData& getData() const { return _data; }

    /**
     * Get the parsed GPS data (non-const version)
     * 
     * @return Reference to the GPS data structure
     */
    GpsData& getData() { return _data; }

    /**
     * Check if we have valid GPS data
     * 
     * @return true if we have a valid fix
     */
    bool hasValidFix() const { return _data.has_fix; }

    /**
     * Get the timestamp of the last data update
     * 
     * @return Timestamp in milliseconds
     */
    uint32_t getLastUpdateTime() const { return _data.last_update_ms; }

    // -------------------------------------------------------------------------------------------
    // SENTENCE FILTERING
    // -------------------------------------------------------------------------------------------

    /**
     * Enable/disable parsing of a specific NMEA sentence type
     * By default, only GGA and RMC are enabled
     * 
     * @param sentence_type The NMEA sentence type to enable/disable
     * @param enabled true to enable, false to disable
     */
    void enableSentence(NmeaSentenceType sentence_type, bool enabled);

    /**
     * Check if a sentence type is enabled
     * 
     * @param sentence_type The NMEA sentence type to check
     * @return true if enabled, false otherwise
     */
    bool isSentenceEnabled(NmeaSentenceType sentence_type) const;

    // -------------------------------------------------------------------------------------------
    // STATISTICS
    // -------------------------------------------------------------------------------------------

    /**
     * Get the number of sentences parsed
     * 
     * @return Total sentence count
     */
    uint32_t getSentenceCount() const { return _sentence_count; }

    /**
     * Get the number of failed checksums
     * 
     * @return Failed checksum count
     */
    uint32_t getFailedChecksumCount() const { return _failed_checksum_count; }

    /**
     * Reset statistics
     */
    void resetStats();

    // -------------------------------------------------------------------------------------------
    // DEBUG
    // -------------------------------------------------------------------------------------------

    /**
     * Get the last parsed sentence type as string
     * 
     * @return String representation of the last sentence type
     */
    String getLastSentenceType() const;

private:
    // -------------------------------------------------------------------------------------------
    // INTERNAL CONSTANTS
    // -------------------------------------------------------------------------------------------

    // Maximum NMEA sentence length (excluding null terminator)
    static constexpr uint16_t MAX_NMEA_SENTENCE_LENGTH = 128;
    
    // Maximum number of fields in a NMEA sentence
    static constexpr uint8_t MAX_NMEA_FIELDS = 20;
    
    // Field separator character
    static constexpr char FIELD_SEPARATOR = ',';
    
    // Checksum separator character
    static constexpr char CHECKSUM_SEPARATOR = '*';

    // -------------------------------------------------------------------------------------------
    // INTERNAL DATA
    // -------------------------------------------------------------------------------------------

    // GPS data storage
    GpsData _data;
    
    // Parsing state
    char _buffer[MAX_NMEA_SENTENCE_LENGTH + 1]; // +1 for null terminator
    uint16_t _buffer_pos = 0;
    
    // Sentence parsing state
    char* _field_ptrs[MAX_NMEA_FIELDS];
    uint8_t _field_count = 0;
    NmeaSentenceType _current_sentence = NmeaSentenceType::NMEA_UNKNOWN;
    NmeaSentenceType _last_sentence = NmeaSentenceType::NMEA_UNKNOWN;
    
    // Statistics
    uint32_t _sentence_count = 0;
    uint32_t _failed_checksum_count = 0;
    
    // Sentence filtering (by default, only GGA and RMC are enabled)
    bool _sentence_enabled[9] = {false}; // Index by NmeaSentenceType enum

    // -------------------------------------------------------------------------------------------
    // INTERNAL PARSING FUNCTIONS
    // -------------------------------------------------------------------------------------------

    /**
     * Process a complete NMEA sentence
     * Called when a complete sentence has been received
     */
    void processSentence();

    /**
     * Parse the GGA sentence
     * Format: $GPGGA,hhmmss.ss,llll.ll,a,yyyyy.yy,a,x,xx,x.x,x.x,M,x.x,M,x.x,xxxx*hh
     */
    void parseGGA();

    /**
     * Parse the RMC sentence
     * Format: $GPRMC,hhmmss.ss,A,llll.ll,a,yyyyy.yy,a,x.x,x.x,ddmmyy,x.x,a*hh
     */
    void parseRMC();

    /**
     * Parse the GSA sentence
     * Format: $GPGSA,A,x,xx,xx,xx,xx,xx,xx,xx,xx,xx,xx,xx,xx,x.x,x.x,x.x*hh
     */
    void parseGSA();

    /**
     * Calculate NMEA checksum
     * 
     * @param sentence The NMEA sentence (without the $ and *hh)
     * @param length Length of the sentence (without $ and *hh)
     * @return The calculated checksum
     */
    static uint8_t calculateChecksum(const char* sentence, uint16_t length);

    /**
     * Verify NMEA checksum
     * 
     * @param sentence The complete NMEA sentence (including $ and *hh)
     * @return true if checksum is valid, false otherwise
     */
    static bool verifyChecksum(const char* sentence);

    /**
     * Extract the sentence type from a NMEA sentence
     * 
     * @param sentence The NMEA sentence
     * @return The sentence type
     */
    static NmeaSentenceType getSentenceType(const char* sentence);

    /**
     * Split a NMEA sentence into fields
     * 
     * @param sentence The NMEA sentence
     * @param fields Array to store field pointers
     * @param max_fields Maximum number of fields
     * @return Number of fields found
     */
    static uint8_t splitFields(const char* sentence, char** fields, uint8_t max_fields);

    /**
     * Parse a latitude or longitude field
     * Format: dddmm.mmmm or dddmm.mmmm,H (H = N/S or E/W)
     * 
     * @param field The field to parse
     * @param hemisphere The hemisphere character ('N', 'S', 'E', 'W')
     * @return Decimal degrees
     */
    static double parseLatLon(const char* field, char hemisphere);

    /**
     * Parse a decimal value
     * 
     * @param field The field to parse
     * @return Parsed double value
     */
    static double parseDecimal(const char* field);

    /**
     * Parse an integer value
     * 
     * @param field The field to parse
     * @return Parsed integer value
     */
    static int32_t parseInteger(const char* field);

    /**
     * Parse a float value
     * 
     * @param field The field to parse
     * @return Parsed float value
     */
    static float parseFloat(const char* field);

    /**
     * Convert DMM to decimal degrees
     * 
     * @param degrees Degrees part
     * @param minutes Minutes part (with decimal)
     * @param hemisphere Hemisphere ('N', 'S', 'E', 'W')
     * @return Decimal degrees
     */
    static double dmmToDecimal(double degrees, double minutes, char hemisphere);

    /**
     * Parse time field (hhmmss.ss)
     * 
     * @param field The time field
     * @param hour Output: hours
     * @param minute Output: minutes
     * @param second Output: seconds
     * @param centisecond Output: centiseconds
     */
    static void parseTime(const char* field, uint8_t& hour, uint8_t& minute, uint8_t& second, uint8_t& centisecond);

    /**
     * Parse date field (ddmmyy)
     * 
     * @param field The date field
     * @param day Output: day
     * @param month Output: month
     * @param year Output: year
     */
    static void parseDate(const char* field, uint8_t& day, uint8_t& month, uint16_t& year);
};

#endif // #ifndef GPS_PARSER_H
