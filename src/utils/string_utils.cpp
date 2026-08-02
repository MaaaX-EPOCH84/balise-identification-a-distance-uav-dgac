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

#include "string_utils.h"

// ================================================================================================
// STRING SPLITTING
// ================================================================================================

String* StringUtils::split(const String& input, char delimiter, uint8_t& part_count, uint8_t max_parts) {
    // Count the number of delimiters
    uint8_t delimiter_count = 0;
    for (uint16_t i = 0; i < input.length(); i++) {
        if (input.charAt(i) == delimiter) {
            delimiter_count++;
        }
    }
    
    // Calculate the number of parts
    part_count = delimiter_count + 1;
    
    // Apply max_parts limit
    if (max_parts > 0 && part_count > max_parts) {
        part_count = max_parts;
    }
    
    // Allocate array of strings
    String* parts = new String[part_count];
    
    // Split the string
    uint16_t start_pos = 0;
    for (uint8_t i = 0; i < part_count - 1; i++) {
        uint16_t end_pos = input.indexOf(delimiter, start_pos);
        if (end_pos == -1) {
            // No more delimiters - last part
            parts[i] = input.substring(start_pos);
            break;
        }
        parts[i] = input.substring(start_pos, end_pos);
        start_pos = end_pos + 1;
    }
    
    // Add the last part
    if (start_pos < input.length()) {
        parts[part_count - 1] = input.substring(start_pos);
    } else {
        parts[part_count - 1] = "";
    }
    
    return parts;
}

uint8_t StringUtils::splitSimple(const String& input, char delimiter, String parts[], uint8_t max_parts) {
    uint8_t part_count = 0;
    uint16_t start_pos = 0;
    
    for (uint8_t i = 0; i < max_parts; i++) {
        uint16_t end_pos = input.indexOf(delimiter, start_pos);
        if (end_pos == -1) {
            // No more delimiters
            parts[part_count] = input.substring(start_pos);
            part_count++;
            break;
        }
        parts[part_count] = input.substring(start_pos, end_pos);
        part_count++;
        start_pos = end_pos + 1;
    }
    
    return part_count;
}

// ================================================================================================
// STRING TRIMMING
// ================================================================================================

String StringUtils::trim(const String& input) {
    return trimRight(trimLeft(input));
}

String StringUtils::trimLeft(const String& input) {
    uint16_t start = 0;
    while (start < input.length() && isspace(input.charAt(start))) {
        start++;
    }
    return input.substring(start);
}

String StringUtils::trimRight(const String& input) {
    int16_t end = input.length() - 1;
    while (end >= 0 && isspace(input.charAt(end))) {
        end--;
    }
    return input.substring(0, end + 1);
}

// ================================================================================================
// STRING CASE CONVERSION
// ================================================================================================

String StringUtils::toUpperCase(const String& input) {
    String result = input;
    for (uint16_t i = 0; i < result.length(); i++) {
        result.setCharAt(i, toupper(result.charAt(i)));
    }
    return result;
}

String StringUtils::toLowerCase(const String& input) {
    String result = input;
    for (uint16_t i = 0; i < result.length(); i++) {
        result.setCharAt(i, tolower(result.charAt(i)));
    }
    return result;
}

// ================================================================================================
// STRING SEARCH
// ================================================================================================

bool StringUtils::startsWith(const String& input, const String& prefix) {
    if (prefix.length() > input.length()) {
        return false;
    }
    return input.substring(0, prefix.length()) == prefix;
}

bool StringUtils::endsWith(const String& input, const String& suffix) {
    if (suffix.length() > input.length()) {
        return false;
    }
    return input.substring(input.length() - suffix.length()) == suffix;
}

bool StringUtils::contains(const String& input, const String& substring) {
    return input.indexOf(substring) != -1;
}

// ================================================================================================
// STRING MODIFICATION
// ================================================================================================

String StringUtils::replaceAll(const String& input, const String& from, const String& to) {
    String result = input;
    int16_t pos = result.indexOf(from);
    
    while (pos != -1) {
        result = result.substring(0, pos) + to + result.substring(pos + from.length());
        pos = result.indexOf(from, pos + to.length());
    }
    
    return result;
}

String StringUtils::removeWhitespace(const String& input) {
    String result;
    for (uint16_t i = 0; i < input.length(); i++) {
        char c = input.charAt(i);
        if (!isspace(c)) {
            result += c;
        }
    }
    return result;
}

String StringUtils::removeChar(const String& input, char c) {
    String result;
    for (uint16_t i = 0; i < input.length(); i++) {
        char ch = input.charAt(i);
        if (ch != c) {
            result += ch;
        }
    }
    return result;
}

// ================================================================================================
// STRING VALIDATION
// ================================================================================================

bool StringUtils::isEmptyOrWhitespace(const String& input) {
    if (input.length() == 0) {
        return true;
    }
    for (uint16_t i = 0; i < input.length(); i++) {
        if (!isspace(input.charAt(i))) {
            return false;
        }
    }
    return true;
}

bool StringUtils::isNumeric(const String& input) {
    if (input.length() == 0) {
        return false;
    }
    
    uint16_t start = 0;
    // Check for leading sign
    if (input.charAt(0) == '-' || input.charAt(0) == '+') {
        start = 1;
        if (input.length() == 1) {
            return false;
        }
    }
    
    bool has_decimal = false;
    bool has_digit = false;
    
    for (uint16_t i = start; i < input.length(); i++) {
        char c = input.charAt(i);
        if (isdigit(c)) {
            has_digit = true;
        } else if (c == '.') {
            if (has_decimal) {
                return false; // Multiple decimal points
            }
            has_decimal = true;
        } else if (c == 'e' || c == 'E') {
            // Scientific notation
            if (i == start || i == input.length() - 1) {
                return false; // e at start or end
            }
            // Check if next character is + or -
            if (i + 1 < input.length() && (input.charAt(i + 1) == '+' || input.charAt(i + 1) == '-')) {
                i++;
            }
            if (i == input.length() - 1) {
                return false; // No exponent digits
            }
        } else {
            return false;
        }
    }
    
    return has_digit;
}

bool StringUtils::isInteger(const String& input) {
    if (input.length() == 0) {
        return false;
    }
    
    uint16_t start = 0;
    if (input.charAt(0) == '-' || input.charAt(0) == '+') {
        start = 1;
        if (input.length() == 1) {
            return false;
        }
    }
    
    for (uint16_t i = start; i < input.length(); i++) {
        if (!isdigit(input.charAt(i))) {
            return false;
        }
    }
    
    return true;
}

bool StringUtils::isFloat(const String& input) {
    if (input.length() == 0) {
        return false;
    }
    
    uint16_t start = 0;
    if (input.charAt(0) == '-' || input.charAt(0) == '+') {
        start = 1;
        if (input.length() == 1) {
            return false;
        }
    }
    
    bool has_decimal = false;
    bool has_digit = false;
    
    for (uint16_t i = start; i < input.length(); i++) {
        char c = input.charAt(i);
        if (isdigit(c)) {
            has_digit = true;
        } else if (c == '.') {
            if (has_decimal) {
                return false; // Multiple decimal points
            }
            has_decimal = true;
        } else {
            return false;
        }
    }
    
    return has_digit;
}

// ================================================================================================
// STRING FORMATTING
// ================================================================================================

String StringUtils::padLeft(int value, uint8_t width, char zero_char) {
    String result = String(value);
    while (result.length() < width) {
        result = String(zero_char) + result;
    }
    return result;
}

String StringUtils::padRight(int value, uint8_t width, char space_char) {
    String result = String(value);
    while (result.length() < width) {
        result += space_char;
    }
    return result;
}

String StringUtils::formatFloat(double value, uint8_t decimal_places) {
    // Simple float formatting without using String's built-in formatting
    // This avoids issues with locale and floating-point precision
    
    // Handle negative numbers
    bool negative = value < 0;
    value = abs(value);
    
    // Calculate integer part
    int integer_part = static_cast<int>(value);
    double fractional_part = value - integer_part;
    
    // Build result string
    String result = (negative ? "-" : "") + String(integer_part);
    
    // Add decimal part
    if (decimal_places > 0) {
        result += ".";
        for (uint8_t i = 0; i < decimal_places; i++) {
            fractional_part *= 10;
            int digit = static_cast<int>(fractional_part) % 10;
            result += String(digit);
        }
    }
    
    return result;
}

// ================================================================================================
// STRING PARSING
// ================================================================================================

uint8_t StringUtils::parseHexByte(const String& input) {
    if (input.length() != 2) {
        return 0;
    }
    
    uint8_t value = 0;
    for (uint8_t i = 0; i < 2; i++) {
        char c = toLowerCase(input).charAt(i);
        value <<= 4;
        if (c >= '0' && c <= '9') {
            value += c - '0';
        } else if (c >= 'a' && c <= 'f') {
            value += c - 'a' + 10;
        } else {
            return 0; // Invalid hex character
        }
    }
    return value;
}

uint32_t StringUtils::parseHex(const String& input) {
    uint32_t value = 0;
    for (uint16_t i = 0; i < input.length(); i++) {
        char c = toLowerCase(input).charAt(i);
        value <<= 4;
        if (c >= '0' && c <= '9') {
            value += c - '0';
        } else if (c >= 'a' && c <= 'f') {
            value += c - 'a' + 10;
        } else {
            return 0; // Invalid hex character
        }
    }
    return value;
}

bool StringUtils::parseBoolean(const String& input) {
    String lower = toLowerCase(input);
    return (lower == "true" || lower == "1" || lower == "yes" || lower == "y" || lower == "on");
}

// ================================================================================================
// IP ADDRESS AND MAC ADDRESS FUNCTIONS
// ================================================================================================

bool StringUtils::isValidIP(const String& ip_str) {
    uint8_t parts[4];
    uint8_t part_count = 0;
    uint16_t start_pos = 0;
    
    for (uint8_t i = 0; i < 3; i++) {
        uint16_t dot_pos = ip_str.indexOf('.', start_pos);
        if (dot_pos == -1) {
            return false;
        }
        
        String part_str = ip_str.substring(start_pos, dot_pos);
        if (part_str.length() == 0 || part_str.length() > 3) {
            return false;
        }
        
        parts[part_count] = part_str.toInt();
        if (parts[part_count] > 255) {
            return false;
        }
        part_count++;
        start_pos = dot_pos + 1;
    }
    
    // Get the last part
    String last_part = ip_str.substring(start_pos);
    if (last_part.length() == 0 || last_part.length() > 3) {
        return false;
    }
    
    parts[3] = last_part.toInt();
    if (parts[3] > 255) {
        return false;
    }
    
    return true;
}

bool StringUtils::stringToIPAddress(const String& ip_str, IPAddress& output) {
    if (!isValidIP(ip_str)) {
        return false;
    }
    
    uint8_t parts[4];
    uint8_t part_count = 0;
    uint16_t start_pos = 0;
    
    for (uint8_t i = 0; i < 3; i++) {
        uint16_t dot_pos = ip_str.indexOf('.', start_pos);
        String part_str = ip_str.substring(start_pos, dot_pos);
        parts[part_count] = part_str.toInt();
        part_count++;
        start_pos = dot_pos + 1;
    }
    
    String last_part = ip_str.substring(start_pos);
    parts[3] = last_part.toInt();
    
    output = IPAddress(parts[0], parts[1], parts[2], parts[3]);
    return true;
}

String StringUtils::ipAddressToString(const IPAddress& ip) {
    return String(ip[0]) + "." + String(ip[1]) + "." + String(ip[2]) + "." + String(ip[3]);
}

String StringUtils::formatMacAddress(const String& mac) {
    String cleaned = removeChar(mac, ':');
    cleaned = removeChar(cleaned, '-');
    cleaned = removeChar(cleaned, ' ');
    cleaned = toUpperCase(cleaned);
    
    if (cleaned.length() != 12) {
        return "";
    }
    
    return cleaned.substring(0, 2) + ":" + cleaned.substring(2, 4) + ":" + 
           cleaned.substring(4, 6) + ":" + cleaned.substring(6, 8) + ":" + 
           cleaned.substring(8, 10) + ":" + cleaned.substring(10, 12);
}

String StringUtils::removeMacColons(const String& mac) {
    String result;
    for (uint16_t i = 0; i < mac.length(); i++) {
        char c = mac.charAt(i);
        if (c != ':' && c != '-' && c != ' ') {
            result += toUpperCase(String(c));
        }
    }
    return result;
}

// ================================================================================================
// URL ENCODE/DECODE
// ================================================================================================

String StringUtils::urlEncode(const String& input) {
    String result;
    for (uint16_t i = 0; i < input.length(); i++) {
        char c = input.charAt(i);
        if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
            result += c;
        } else {
            result += '%';
            result += String(c, 16); // Convert to hex
        }
    }
    return result;
}

String StringUtils::urlDecode(const String& input) {
    String result;
    for (uint16_t i = 0; i < input.length(); i++) {
        char c = input.charAt(i);
        if (c == '%') {
            if (i + 2 < input.length()) {
                String hex_str = input.substring(i + 1, i + 3);
                uint8_t value = parseHexByte(hex_str);
                result += char(value);
                i += 2;
            }
        } else if (c == '+') {
            result += ' ';
        } else {
            result += c;
        }
    }
    return result;
}

// ================================================================================================
// COORDINATE FORMATTING
// ================================================================================================

String StringUtils::formatCoordinate(double value, bool is_latitude, uint8_t decimal_places) {
    String dir = (value >= 0) ? (is_latitude ? "N" : "E") : (is_latitude ? "S" : "W");
    value = abs(value);
    return String(value, static_cast<unsigned int>(decimal_places)) + " " + dir;
}

String StringUtils::formatCoordinateNMEA(double value, bool is_latitude, uint8_t decimal_places) {
    double abs_value = abs(value);
    uint8_t degrees = static_cast<uint8_t>(abs_value);
    double minutes = (abs_value - degrees) * 60.0;
    
    String dir = (value >= 0) ? (is_latitude ? "N" : "E") : (is_latitude ? "S" : "W");
    
    String result = String(degrees, 2); // 2 digits for degrees
    
    // Calculate minutes with decimal part
    uint32_t minutes_int = static_cast<uint32_t>(minutes);
    double minutes_frac = minutes - minutes_int;
    
    // Add leading zeros if needed
    if (minutes_int < 10) {
        result += "0";
    }
    result += String(minutes_int);
    
    // Add decimal part
    if (decimal_places > 0) {
        result += ".";
        for (uint8_t i = 0; i < decimal_places; i++) {
            minutes_frac *= 10;
            uint8_t digit = static_cast<uint8_t>(minutes_frac) % 10;
            result += String(digit);
        }
    }
    
    result += "," + dir;
    return result;
}
