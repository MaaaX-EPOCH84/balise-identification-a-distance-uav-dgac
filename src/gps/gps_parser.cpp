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

#include "gps_parser.h"
#include <cstring>
#include <cstdlib>
#include <cmath>

// ================================================================================================
// CONSTRUCTOR / DESTRUCTOR
// ================================================================================================

NmeaParser::NmeaParser() {
    // Initialize buffer
    _buffer[0] = '\0';
    _buffer_pos = 0;
    
    // Initialize GPS data
    _data.reset();
    
    // Enable GGA and RMC by default (minimum required for DGAC beacon)
    _sentence_enabled[static_cast<uint8_t>(NmeaSentenceType::NMEA_GGA)] = true;
    _sentence_enabled[static_cast<uint8_t>(NmeaSentenceType::NMEA_RMC)] = true;
    
    // Other sentences disabled by default
    _sentence_enabled[static_cast<uint8_t>(NmeaSentenceType::NMEA_GLL)] = false;
    _sentence_enabled[static_cast<uint8_t>(NmeaSentenceType::NMEA_GSA)] = false;
    _sentence_enabled[static_cast<uint8_t>(NmeaSentenceType::NMEA_GSV)] = false;
    _sentence_enabled[static_cast<uint8_t>(NmeaSentenceType::NMEA_VTG)] = false;
    _sentence_enabled[static_cast<uint8_t>(NmeaSentenceType::NMEA_ZDA)] = false;
    _sentence_enabled[static_cast<uint8_t>(NmeaSentenceType::NMEA_PUBX)] = false;
    
    // Initialize sentence tracking
    _current_sentence = NmeaSentenceType::NMEA_UNKNOWN;
    _last_sentence = NmeaSentenceType::NMEA_UNKNOWN;
    
    // Reset statistics
    resetStats();
}

NmeaParser::~NmeaParser() {
    // Nothing to clean up
}

// ================================================================================================
// PARSE FUNCTIONS
// ================================================================================================

void NmeaParser::parse(char c) {
    // Handle newline characters (end of sentence)
    if (c == '\n' || c == '\r') {
        // End of sentence - process it
        if (_buffer_pos > 0) {
            _buffer[_buffer_pos] = '\0'; // Null-terminate
            processSentence();
        }
        _buffer_pos = 0;
        return;
    }
    
    // Buffer overflow protection
    if (_buffer_pos >= MAX_NMEA_SENTENCE_LENGTH) {
        // Buffer full, reset and start new sentence
        _buffer_pos = 0;
    }
    
    // Store character
    _buffer[_buffer_pos++] = c;
}

void NmeaParser::parseSentence(const char* sentence) {
    if (sentence == nullptr || *sentence == '\0') {
        return;
    }
    
    // Copy sentence to buffer and process
    uint16_t len = strlen(sentence);
    if (len > MAX_NMEA_SENTENCE_LENGTH) {
        return; // Too long
    }
    
    memcpy(_buffer, sentence, len);
    _buffer[len] = '\0';
    _buffer_pos = len;
    
    processSentence();
}

// ================================================================================================
// SENTENCE FILTERING
// ================================================================================================

void NmeaParser::enableSentence(NmeaSentenceType sentence_type, bool enabled) {
    uint8_t index = static_cast<uint8_t>(sentence_type);
    if (index < 9) {
        _sentence_enabled[index] = enabled;
    }
}

bool NmeaParser::isSentenceEnabled(NmeaSentenceType sentence_type) const {
    uint8_t index = static_cast<uint8_t>(sentence_type);
    if (index < 9) {
        return _sentence_enabled[index];
    }
    return false;
}

// ================================================================================================
// STATISTICS
// ================================================================================================

void NmeaParser::resetStats() {
    _sentence_count = 0;
    _failed_checksum_count = 0;
}

String NmeaParser::getLastSentenceType() const {
    switch (_last_sentence) {
        case NmeaSentenceType::NMEA_GGA:    return "GGA";
        case NmeaSentenceType::NMEA_GLL:    return "GLL";
        case NmeaSentenceType::NMEA_GSA:    return "GSA";
        case NmeaSentenceType::NMEA_GSV:    return "GSV";
        case NmeaSentenceType::NMEA_RMC:    return "RMC";
        case NmeaSentenceType::NMEA_VTG:    return "VTG";
        case NmeaSentenceType::NMEA_ZDA:    return "ZDA";
        case NmeaSentenceType::NMEA_PUBX:   return "PUBX";
        default:                          return "UNKNOWN";
    }
}

// ================================================================================================
// INTERNAL PARSING FUNCTIONS
// ================================================================================================

void NmeaParser::processSentence() {
    // Check if buffer starts with '$'
    if (_buffer[0] != '$') {
        return;
    }
    
    // Verify checksum
    if (!verifyChecksum(_buffer)) {
        _failed_checksum_count++;
        return;
    }
    
    // Extract sentence type
    _last_sentence = _current_sentence = getSentenceType(_buffer);
    
    // Check if sentence type is enabled
    if (!_sentence_enabled[static_cast<uint8_t>(_current_sentence)]) {
        return;
    }
    
    _sentence_count++;
    
    // Split into fields, starting after the sentence type token: "$GNGGA," is skipped
    // whole, so field 0 is the first data field, as parseGGA/parseRMC/parseGSA document.
    // Skipping only the '$' left the type in field 0 and shifted every field by one.
    char* fields_start = strchr(_buffer, FIELD_SEPARATOR);
    if (fields_start == nullptr) {
        return;
    }
    _field_count = splitFields(fields_start + 1, _field_ptrs, MAX_NMEA_FIELDS);
    
    // Parse based on sentence type
    switch (_current_sentence) {
        case NmeaSentenceType::NMEA_GGA:
            parseGGA();
            break;
        case NmeaSentenceType::NMEA_RMC:
            parseRMC();
            break;
        case NmeaSentenceType::NMEA_GSA:
            if (isSentenceEnabled(NmeaSentenceType::NMEA_GSA)) {
                parseGSA();
            }
            break;
        // Other sentence types can be added here
        default:
            break;
    }
}

void NmeaParser::parseGGA() {
    // GGA format: $GPGGA,hhmmss.ss,llll.ll,a,yyyyy.yy,a,x,xx,x.x,x.x,M,x.x,M,x.x,xxxx*hh
    // Fields:
    //  0: Time (hhmmss.ss)
    //  1: Latitude (llll.ll)
    //  2: Latitude hemisphere (N/S)
    //  3: Longitude (yyyyy.yy)
    //  4: Longitude hemisphere (E/W)
    //  5: Fix quality (0-8)
    //  6: Number of satellites
    //  7: HDOP
    //  8: Altitude
    //  9: Altitude units (M)
    // 10: Geoid separation
    // 11: Geoid separation units (M)
    // 12: Age of differential corrections
    // 13: Differential reference station ID
    // 14: Checksum (not split out: it stays glued to the last field)
    
    // Only the fields actually read are required, so a receiver that omits trailing
    // fields is still parsed instead of being silently discarded.
    if (_field_count < 9) {
        return;
    }
    
    // Parse time
    if (_field_ptrs[0] && *_field_ptrs[0]) {
        uint8_t hour, minute, second, centisecond;
        parseTime(_field_ptrs[0], hour, minute, second, centisecond);
        _data.last_update_ms = millis();
    }
    
    // Parse latitude
    if (_field_ptrs[1] && *_field_ptrs[1] && _field_ptrs[2] && *_field_ptrs[2]) {
        double lat = parseLatLon(_field_ptrs[1], _field_ptrs[2][0]);
        _data.position.latitude = lat;
    }
    
    // Parse longitude
    if (_field_ptrs[3] && *_field_ptrs[3] && _field_ptrs[4] && *_field_ptrs[4]) {
        double lon = parseLatLon(_field_ptrs[3], _field_ptrs[4][0]);
        _data.position.longitude = lon;
    }
    
    // Parse fix quality
    if (_field_ptrs[5] && *_field_ptrs[5]) {
        uint8_t fix_quality = static_cast<uint8_t>(parseInteger(_field_ptrs[5]));
        _data.has_fix = (fix_quality >= static_cast<uint8_t>(GpsFixQuality::GPS_FIX_GPS));
    }
    
    // Parse number of satellites
    if (_field_ptrs[6] && *_field_ptrs[6]) {
        _data.satellites = static_cast<uint8_t>(parseInteger(_field_ptrs[6]));
    }
    
    // Parse HDOP
    if (_field_ptrs[7] && *_field_ptrs[7]) {
        _data.hdop = parseDecimal(_field_ptrs[7]);
    }
    
    // Parse altitude (meters)
    if (_field_ptrs[8] && *_field_ptrs[8]) {
        _data.altitude = parseDecimal(_field_ptrs[8]);
    }
}

void NmeaParser::parseRMC() {
    // RMC format: $GPRMC,hhmmss.ss,A,llll.ll,a,yyyyy.yy,a,x.x,x.x,ddmmyy,x.x,a*hh
    // Fields:
    //  0: Time (hhmmss.ss)
    //  1: Status (A=valid, V=warning)
    //  2: Latitude (llll.ll)
    //  3: Latitude hemisphere (N/S)
    //  4: Longitude (yyyyy.yy)
    //  5: Longitude hemisphere (E/W)
    //  6: Speed over ground (knots)
    //  7: Course over ground (degrees)
    //  8: Date (ddmmyy)
    //  9: Magnetic variation (degrees)
    // 10: Magnetic variation direction (E/W)
    // 11: Mode indicator (A=autonomous, D=differential, E=estimated, M=manual, S=simulated)
    // 12: Checksum (not split out: it stays glued to the last field)
    
    if (_field_count < 9) {
        return;
    }
    
    // Parse time
    if (_field_ptrs[0] && *_field_ptrs[0]) {
        uint8_t hour, minute, second, centisecond;
        parseTime(_field_ptrs[0], hour, minute, second, centisecond);
        _data.last_update_ms = millis();
    }
    
    // Parse status (A = valid, V = warning/invalid)
    if (_field_ptrs[1] && *_field_ptrs[1]) {
        _data.has_fix = (_field_ptrs[1][0] == 'A');
    }
    
    // Parse latitude
    if (_field_ptrs[2] && *_field_ptrs[2] && _field_ptrs[3] && *_field_ptrs[3]) {
        double lat = parseLatLon(_field_ptrs[2], _field_ptrs[3][0]);
        _data.position.latitude = lat;
    }
    
    // Parse longitude
    if (_field_ptrs[4] && *_field_ptrs[4] && _field_ptrs[5] && *_field_ptrs[5]) {
        double lon = parseLatLon(_field_ptrs[4], _field_ptrs[5][0]);
        _data.position.longitude = lon;
    }
    
    // Parse speed (knots -> m/s: 1 knot = 0.514444 m/s)
    if (_field_ptrs[6] && *_field_ptrs[6]) {
        double speed_knots = parseDecimal(_field_ptrs[6]);
        _data.speed = speed_knots * 0.514444; // Convert knots to m/s
    }
    
    // Parse course (heading in degrees)
    if (_field_ptrs[7] && *_field_ptrs[7]) {
        _data.heading = parseDecimal(_field_ptrs[7]);
    }
    
    // Parse date
    if (_field_ptrs[8] && *_field_ptrs[8]) {
        uint8_t day, month;
        uint16_t year;
        parseDate(_field_ptrs[8], day, month, year);
        // Date is parsed but not currently used
    }
}

void NmeaParser::parseGSA() {
    // GSA format: $GPGSA,A,x,xx,xx,xx,xx,xx,xx,xx,xx,xx,xx,xx,xx,x.x,x.x,x.x*hh
    // Fields:
    //  0: Mode (A=auto, M=manual)
    //  1: Fix type (1=none, 2=2D, 3=3D)
    //  2-13: Satellite PRN numbers
    // 14: PDOP
    // 15: HDOP
    // 16: VDOP
    
    if (_field_count < 16) {
        return;
    }
    
    // Parse HDOP if available
    if (_field_ptrs[15] && *_field_ptrs[15]) {
        _data.hdop = parseDecimal(_field_ptrs[15]);
    }
}

// ================================================================================================
// CHECKSUM FUNCTIONS
// ================================================================================================

uint8_t NmeaParser::calculateChecksum(const char* sentence, uint16_t length) {
    uint8_t checksum = 0;
    for (uint16_t i = 0; i < length; i++) {
        checksum ^= sentence[i];
    }
    return checksum;
}

bool NmeaParser::verifyChecksum(const char* sentence) {
    // Find the checksum separator
    const char* checksum_ptr = strchr(sentence + 1, CHECKSUM_SEPARATOR); // Skip the '$'
    if (checksum_ptr == nullptr) {
        return false;
    }
    
    // Calculate checksum from the data part (between $ and *)
    uint16_t data_length = static_cast<uint16_t>(checksum_ptr - (sentence + 1));
    uint8_t calculated_checksum = calculateChecksum(sentence + 1, data_length);
    
    // Parse the expected checksum (hex value after *)
    uint8_t expected_checksum = 0;
    const char* hex_ptr = checksum_ptr + 1;
    if (*hex_ptr) {
        expected_checksum = static_cast<uint8_t>(strtol(hex_ptr, nullptr, 16));
    }
    
    return calculated_checksum == expected_checksum;
}

// ================================================================================================
// UTILITY FUNCTIONS
// ================================================================================================

NmeaSentenceType NmeaParser::getSentenceType(const char* sentence) {
    // Sentence format: $GPXXX,... or $GNXXX,... (for UBLOX)
    if (sentence[0] != '$') {
        return NmeaSentenceType::NMEA_UNKNOWN;
    }
    
    // Check for UBLOX proprietary sentences (PUBX)
    if (strncmp(sentence + 1, "PUBX", 4) == 0) {
        return NmeaSentenceType::NMEA_PUBX;
    }
    
    // Standard NMEA sentences
    if (strncmp(sentence + 3, "GGA", 3) == 0) {
        return NmeaSentenceType::NMEA_GGA;
    } else if (strncmp(sentence + 3, "GLL", 3) == 0) {
        return NmeaSentenceType::NMEA_GLL;
    } else if (strncmp(sentence + 3, "GSA", 3) == 0) {
        return NmeaSentenceType::NMEA_GSA;
    } else if (strncmp(sentence + 3, "GSV", 3) == 0) {
        return NmeaSentenceType::NMEA_GSV;
    } else if (strncmp(sentence + 3, "RMC", 3) == 0) {
        return NmeaSentenceType::NMEA_RMC;
    } else if (strncmp(sentence + 3, "VTG", 3) == 0) {
        return NmeaSentenceType::NMEA_VTG;
    } else if (strncmp(sentence + 3, "ZDA", 3) == 0) {
        return NmeaSentenceType::NMEA_ZDA;
    }
    
    return NmeaSentenceType::NMEA_UNKNOWN;
}

uint8_t NmeaParser::splitFields(const char* sentence, char** fields, uint8_t max_fields) {
    uint8_t field_count = 0;
    const char* start = sentence;
    const char* end = sentence + strlen(sentence);
    
    while (start < end && field_count < max_fields) {
        // Store field pointer
        fields[field_count++] = const_cast<char*>(start);
        
        // Find next separator or end
        while (*start && *start != FIELD_SEPARATOR && start < end) {
            start++;
        }
        
        // Null-terminate the field (temporarily)
        if (*start == FIELD_SEPARATOR) {
            *const_cast<char*>(start) = '\0';
            start++;
        } else if (start >= end) {
            break;
        }
    }
    
    return field_count;
}

double NmeaParser::parseLatLon(const char* field, char hemisphere) {
    if (field == nullptr || *field == '\0') {
        return 0.0;
    }
    
    // Parse DDM (Degrees Decimal Minutes)
    // Format: dddmm.mmmm
    double value = parseDecimal(field);
    
    // Extract degrees
    double degrees = static_cast<int32_t>(value / 100.0);
    double minutes = value - (degrees * 100.0);
    
    // Convert to decimal degrees
    double decimal = dmmToDecimal(degrees, minutes, hemisphere);
    
    return decimal;
}

double NmeaParser::parseDecimal(const char* field) {
    if (field == nullptr || *field == '\0') {
        return 0.0;
    }
    return atof(field);
}

int32_t NmeaParser::parseInteger(const char* field) {
    if (field == nullptr || *field == '\0') {
        return 0;
    }
    return atol(field);
}

float NmeaParser::parseFloat(const char* field) {
    if (field == nullptr || *field == '\0') {
        return 0.0f;
    }
    return atof(field);
}

double NmeaParser::dmmToDecimal(double degrees, double minutes, char hemisphere) {
    double decimal = degrees + (minutes / 60.0);
    
    // Apply hemisphere
    if (hemisphere == 'S' || hemisphere == 'W') {
        decimal = -decimal;
    }
    
    return decimal;
}

void NmeaParser::parseTime(const char* field, uint8_t& hour, uint8_t& minute, uint8_t& second, uint8_t& centisecond) {
    if (field == nullptr || *field == '\0') {
        hour = minute = second = centisecond = 0;
        return;
    }
    
    // Parse hhmmss.ss
    hour = static_cast<uint8_t>((field[0] - '0') * 10 + (field[1] - '0'));
    minute = static_cast<uint8_t>((field[2] - '0') * 10 + (field[3] - '0'));
    second = static_cast<uint8_t>((field[4] - '0') * 10 + (field[5] - '0'));
    
    if (field[6] == '.') {
        centisecond = static_cast<uint8_t>((field[7] - '0') * 10 + (field[8] - '0'));
    } else {
        centisecond = 0;
    }
}

void NmeaParser::parseDate(const char* field, uint8_t& day, uint8_t& month, uint16_t& year) {
    if (field == nullptr || *field == '\0' || strlen(field) < 6) {
        day = month = 0;
        year = 0;
        return;
    }
    
    // Parse ddmmyy
    day = static_cast<uint8_t>((field[0] - '0') * 10 + (field[1] - '0'));
    month = static_cast<uint8_t>((field[2] - '0') * 10 + (field[3] - '0'));
    year = static_cast<uint16_t>((field[4] - '0') * 10 + (field[5] - '0'));
    
    // Convert 2-digit year to 4-digit (2000 + yy)
    if (year < 100) {
        year += 2000;
    }
}
