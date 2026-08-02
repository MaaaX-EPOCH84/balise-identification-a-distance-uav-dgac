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
// See the GNU General License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
// ================================================================================================

#ifndef STRING_UTILS_H
#define STRING_UTILS_H

#include <Arduino.h>

// ================================================================================================
// STRING UTILITY CLASS
// Provides useful string manipulation functions
// ================================================================================================

class StringUtils
{
public:
    // -------------------------------------------------------------------------------------------
    // STRING SPLITTING
    // -------------------------------------------------------------------------------------------

    /**
     * Split a string by a delimiter
     * 
     * @param input The string to split
     * @param delimiter The delimiter character
     * @param max_parts Maximum number of parts to extract (0 = no limit)
     * @return Vector of string parts
     * 
     * Note: This is a simplified version. For production, use a more efficient approach.
     */
    static String* split(const String& input, char delimiter, uint8_t& part_count, uint8_t max_parts = 0);

    /**
     * Split a string by a delimiter (simple version, returns first n parts)
     * 
     * @param input The string to split
     * @param delimiter The delimiter character
     * @param parts Output array of strings
     * @param max_parts Maximum number of parts to extract
     * @return Number of parts extracted
     */
    static uint8_t splitSimple(const String& input, char delimiter, String parts[], uint8_t max_parts);

    // -------------------------------------------------------------------------------------------
    // STRING TRIMMING
    // -------------------------------------------------------------------------------------------

    /**
     * Trim whitespace from both ends of a string
     * 
     * @param input The string to trim
     * @return Trimmed string
     */
    static String trim(const String& input);

    /**
     * Trim whitespace from the left side of a string
     * 
     * @param input The string to trim
     * @return Trimmed string
     */
    static String trimLeft(const String& input);

    /**
     * Trim whitespace from the right side of a string
     * 
     * @param input The string to trim
     * @return Trimmed string
     */
    static String trimRight(const String& input);

    // -------------------------------------------------------------------------------------------
    // STRING CASE CONVERSION
    // -------------------------------------------------------------------------------------------

    /**
     * Convert a string to uppercase
     * 
     * @param input The string to convert
     * @return Uppercase string
     */
    static String toUpperCase(const String& input);

    /**
     * Convert a string to lowercase
     * 
     * @param input The string to convert
     * @return Lowercase string
     */
    static String toLowerCase(const String& input);

    // -------------------------------------------------------------------------------------------
    // STRING SEARCH
    // -------------------------------------------------------------------------------------------

    /**
     * Check if a string starts with a prefix
     * 
     * @param input The string to check
     * @param prefix The prefix to look for
     * @return true if the string starts with the prefix
     */
    static bool startsWith(const String& input, const String& prefix);

    /**
     * Check if a string ends with a suffix
     * 
     * @param input The string to check
     * @param suffix The suffix to look for
     * @return true if the string ends with the suffix
     */
    static bool endsWith(const String& input, const String& suffix);

    /**
     * Check if a string contains a substring
     * 
     * @param input The string to check
     * @param substring The substring to look for
     * @return true if the substring is found
     */
    static bool contains(const String& input, const String& substring);

    // -------------------------------------------------------------------------------------------
    // STRING MODIFICATION
    // -------------------------------------------------------------------------------------------

    /**
     * Replace all occurrences of a substring with another string
     * 
     * @param input The original string
     * @param from The substring to replace
     * @param to The replacement string
     * @return Modified string
     */
    static String replaceAll(const String& input, const String& from, const String& to);

    /**
     * Remove all whitespace from a string
     * 
     * @param input The string to process
     * @return String with whitespace removed
     */
    static String removeWhitespace(const String& input);

    /**
     * Remove all occurrences of a character from a string
     * 
     * @param input The string to process
     * @param c The character to remove
     * @return String with character removed
     */
    static String removeChar(const String& input, char c);

    // -------------------------------------------------------------------------------------------
    // STRING VALIDATION
    // -------------------------------------------------------------------------------------------

    /**
     * Check if a string is empty or contains only whitespace
     * 
     * @param input The string to check
     * @return true if the string is empty or whitespace-only
     */
    static bool isEmptyOrWhitespace(const String& input);

    /**
     * Check if a string is a valid number
     * 
     * @param input The string to check
     * @return true if the string represents a valid number
     */
    static bool isNumeric(const String& input);

    /**
     * Check if a string is a valid integer
     * 
     * @param input The string to check
     * @return true if the string represents a valid integer
     */
    static bool isInteger(const String& input);

    /**
     * Check if a string is a valid floating-point number
     * 
     * @param input The string to check
     * @return true if the string represents a valid floating-point number
     */
    static bool isFloat(const String& input);

    // -------------------------------------------------------------------------------------------
    // STRING FORMATTING
    // -------------------------------------------------------------------------------------------

    /**
     * Format a number with leading zeros
     * 
     * @param value The number to format
     * @param width The minimum width
     * @param zero_char The padding character (default: '0')
     * @return Formatted string
     */
    static String padLeft(int value, uint8_t width, char zero_char = '0');

    /**
     * Format a number with trailing spaces
     * 
     * @param value The number to format
     * @param width The minimum width
     * @return Formatted string
     */
    static String padRight(int value, uint8_t width, char space_char = ' ');

    /**
     * Format a floating-point number to a specified number of decimal places
     * 
     * @param value The number to format
     * @param decimal_places Number of decimal places
     * @return Formatted string
     */
    static String formatFloat(double value, uint8_t decimal_places = 2);

    // -------------------------------------------------------------------------------------------
    // STRING PARSING
    // -------------------------------------------------------------------------------------------

    /**
     * Parse a hexadecimal string to a byte value
     * 
     * @param input The hexadecimal string
     * @return The parsed byte value (0 if invalid)
     */
    static uint8_t parseHexByte(const String& input);

    /**
     * Parse a hexadecimal string to an integer value
     * 
     * @param input The hexadecimal string
     * @return The parsed integer value (0 if invalid)
     */
    static uint32_t parseHex(const String& input);

    /**
     * Parse a boolean from a string ("true", "1", "yes" -> true, otherwise false)
     * 
     * @param input The string to parse
     * @return The parsed boolean value
     */
    static bool parseBoolean(const String& input);

    // -------------------------------------------------------------------------------------------
    // IP ADDRESS AND MAC ADDRESS FUNCTIONS
    // -------------------------------------------------------------------------------------------

    /**
     * Check if a string is a valid IP address
     * 
     * @param ip_str The string to check
     * @return true if the string is a valid IP address
     */
    static bool isValidIP(const String& ip_str);

    /**
     * Convert an IP address string to an IPAddress object
     * 
     * @param ip_str The IP address string (e.g., "192.168.1.1")
     * @param output The IPAddress object to populate
     * @return true if conversion was successful
     */
    static bool stringToIPAddress(const String& ip_str, IPAddress& output);

    /**
     * Convert an IPAddress object to a string
     * 
     * @param ip The IPAddress object
     * @return The IP address string
     */
    static String ipAddressToString(const IPAddress& ip);

    /**
     * Format a MAC address string (add colons if missing)
     * 
     * @param mac The MAC address string (with or without colons)
     * @return Formatted MAC address with colons (e.g., "AA:BB:CC:DD:EE:FF")
     */
    static String formatMacAddress(const String& mac);

    /**
     * Remove colons from a MAC address string
     * 
     * @param mac The MAC address string with colons
     * @return MAC address without colons
     */
    static String removeMacColons(const String& mac);

    // -------------------------------------------------------------------------------------------
    // URL ENCODE/DECODE
    // -------------------------------------------------------------------------------------------

    /**
     * URL-encode a string
     * 
     * @param input The string to encode
     * @return URL-encoded string
     */
    static String urlEncode(const String& input);

    /**
     * URL-decode a string
     * 
     * @param input The URL-encoded string
     * @return Decoded string
     */
    static String urlDecode(const String& input);

    // -------------------------------------------------------------------------------------------
    // COORDINATE FORMATTING
    // -------------------------------------------------------------------------------------------

    /**
     * Format latitude/longitude in decimal degrees
     * 
     * @param value The coordinate value
     * @param is_latitude true for latitude, false for longitude
     * @param decimal_places Number of decimal places
     * @return Formatted coordinate string
     */
    static String formatCoordinate(double value, bool is_latitude = true, uint8_t decimal_places = 6);

    /**
     * Format latitude/longitude in degrees and decimal minutes (NMEA format)
     * 
     * @param value The coordinate value in decimal degrees
     * @param is_latitude true for latitude, false for longitude
     * @param decimal_places Number of decimal places for minutes
     * @return Formatted string (e.g., "4812.3456,N")
     */
    static String formatCoordinateNMEA(double value, bool is_latitude = true, uint8_t decimal_places = 4);
};

#endif // #ifndef STRING_UTILS_H
