// ************************************************************************************************
// * BALISE D'IDENTIFICATION A DISTANCE POUR UAV (DGAC)                                           *
// ***********************************************************************************************
// * Author:     Maxime MARCONATO (maxime@maaax.com, aka MaaaX^^, aka EPOCH84)
// * Repository: https://github.com/EPOCH84/Balise_Identification_A_Distance_UAV_DGAC
// * Based on:
// * - https://github.com/fanfanlatulipe26/BaliseDGAC_GPS_Logger
// * - https://github.com/dev-fred/GPS_Tracker_ESP8266
// * - https://github.com/khancyr/droneID_FR
// * - https://github.com/f5soh/balise_esp32
// * - https://www.tranquille-informatique.fr/modelisme/divers/balise-dgac-signalement-electronique-a-distance-drone-aeromodelisme.html
// ***********************************************************************************************
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

#ifndef CONVERSION_UTILS_H
#define CONVERSION_UTILS_H

#include <Arduino.h>
#include "../core/biadud_core.h"

// ================================================================================================
// CONVERSION UTILITY CLASS
// Provides unit conversion and coordinate transformation functions
// ================================================================================================

class ConversionUtils
{
public:
    // -------------------------------------------------------------------------------------------
    // COORDINATE CONVERSION
    // -------------------------------------------------------------------------------------------

    /**
     * Convert degrees to radians
     * 
     * @param degrees Value in degrees
     * @return Value in radians
     */
    static double degreesToRadians(double degrees);

    /**
     * Convert radians to degrees
     * 
     * @param radians Value in radians
     * @return Value in degrees
     */
    static double radiansToDegrees(double radians);

    /**
     * Convert decimal degrees to degrees and decimal minutes (NMEA format)
     * 
     * @param decimal_degrees Value in decimal degrees
     * @param degrees Output: degrees part
     * @param minutes Output: minutes part (including decimal fraction)
     */
    static void decimalDegreesToNMEA(double decimal_degrees, uint16_t& degrees, double& minutes);

    /**
     * Convert degrees and decimal minutes to decimal degrees
     * 
     * @param degrees Degrees part
     * @param minutes Minutes part (including decimal fraction)
     * @param is_latitude true for latitude (North/South), false for longitude (East/West)
     * @return Value in decimal degrees
     */
    static double nmeaToDecimalDegrees(uint16_t degrees, double minutes, bool is_latitude = true);

    // -------------------------------------------------------------------------------------------
    // DISTANCE CONVERSION
    // -------------------------------------------------------------------------------------------

    /**
     * Convert meters to feet
     * 
     * @param meters Value in meters
     * @return Value in feet
     */
    static double metersToFeet(double meters);

    /**
     * Convert feet to meters
     * 
     * @param feet Value in feet
     * @return Value in meters
     */
    static double feetToMeters(double feet);

    /**
     * Convert meters to nautical miles
     * 
     * @param meters Value in meters
     * @return Value in nautical miles
     */
    static double metersToNauticalMiles(double meters);

    /**
     * Convert nautical miles to meters
     * 
     * @param nautical_miles Value in nautical miles
     * @return Value in meters
     */
    static double nauticalMilesToMeters(double nautical_miles);

    /**
     * Convert meters to kilometers
     * 
     * @param meters Value in meters
     * @return Value in kilometers
     */
    static double metersToKilometers(double meters);

    /**
     * Convert kilometers to meters
     * 
     * @param kilometers Value in kilometers
     * @return Value in meters
     */
    static double kilometersToMeters(double kilometers);

    // -------------------------------------------------------------------------------------------
    // SPEED CONVERSION
    // -------------------------------------------------------------------------------------------

    /**
     * Convert meters per second to kilometers per hour
     * 
     * @param mps Value in meters per second
     * @return Value in kilometers per hour
     */
    static double mpsToKph(double mps);

    /**
     * Convert kilometers per hour to meters per second
     * 
     * @param kph Value in kilometers per hour
     * @return Value in meters per second
     */
    static double kphToMps(double kph);

    /**
     * Convert meters per second to knots
     * 
     * @param mps Value in meters per second
     * @return Value in knots
     */
    static double mpsToKnots(double mps);

    /**
     * Convert knots to meters per second
     * 
     * @param knots Value in knots
     * @return Value in meters per second
     */
    static double knotsToMps(double knots);

    // -------------------------------------------------------------------------------------------
    // ANGLE CONVERSION
    // -------------------------------------------------------------------------------------------

    /**
     * Normalize an angle to 0-360 degrees
     * 
     * @param angle Angle in degrees (can be negative)
     * @return Normalized angle (0-360 degrees)
     */
    static double normalizeAngle(double angle);

    /**
     * Convert compass heading to cardinal direction string
     * 
     * @param heading Heading in degrees (0-360)
     * @return Cardinal direction string (e.g., "N", "NE", "E", "S", etc.)
     */
    static String headingToCardinal(double heading);

    /**
     * Convert cardinal direction string to heading
     * 
     * @param direction Cardinal direction string (e.g., "N", "NE", "E", "S", etc.)
     * @return Heading in degrees (0-360), or -1 if invalid
     */
    static double cardinalToHeading(const String& direction);

    // -------------------------------------------------------------------------------------------
    // COORDINATE FORMAT CONVERSION (for TLV encoding)
    // -------------------------------------------------------------------------------------------

    /**
     * Convert decimal degrees to degrees * 1e7 (for integer storage)
     * This is used for TLV encoding in DGAC beacon frames
     * 
     * @param decimal_degrees Value in decimal degrees
     * @return Value in degrees * 10^7
     */
    static int32_t decimalDegreesToCentidegreesE7(double decimal_degrees);

    /**
     * Convert degrees * 1e7 back to decimal degrees
     * 
     * @param centidegrees_e7 Value in degrees * 10^7
     * @return Value in decimal degrees
     */
    static double centidegreesE7ToDecimalDegrees(int32_t centidegrees_e7);

    // -------------------------------------------------------------------------------------------
    // TEMPERATURE CONVERSION
    // -------------------------------------------------------------------------------------------

    /**
     * Convert Celsius to Fahrenheit
     * 
     * @param celsius Temperature in Celsius
     * @return Temperature in Fahrenheit
     */
    static double celsiusToFahrenheit(double celsius);

    /**
     * Convert Fahrenheit to Celsius
     * 
     * @param fahrenheit Temperature in Fahrenheit
     * @return Temperature in Celsius
     */
    static double fahrenheitToCelsius(double fahrenheit);

    // -------------------------------------------------------------------------------------------
    // PRESSURE CONVERSION
    // -------------------------------------------------------------------------------------------

    /**
     * Convert hPa to mbar
     * Note: 1 hPa = 1 mbar
     * 
     * @param hpa Pressure in hPa
     * @return Pressure in mbar
     */
    static double hpaToMbar(double hpa);

    /**
     * Convert mbar to hPa
     * Note: 1 mbar = 1 hPa
     * 
     * @param mbar Pressure in mbar
     * @return Pressure in hPa
     */
    static double mbarToHpa(double mbar);

    // -------------------------------------------------------------------------------------------
    // ALTITUDE CONVERSION
    // -------------------------------------------------------------------------------------------

    /**
     * Convert altitude from meters to feet
     * 
     * @param altitude_msl Altitude in meters (MSL)
     * @return Altitude in feet (MSL)
     */
    static double altitudeToFeet(int16_t altitude_msl);

    // -------------------------------------------------------------------------------------------
    // BYTE ORDER CONVERSION (ENDIANNESS)
    // -------------------------------------------------------------------------------------------

    /**
     * Convert a 16-bit value from host to little-endian byte order
     * 
     * @param value The value to convert
     * @return Value in little-endian byte order
     */
    static uint16_t hostToLE16(uint16_t value);

    /**
     * Convert a 16-bit value from little-endian to host byte order
     * 
     * @param value The value to convert
     * @return Value in host byte order
     */
    static uint16_t le16ToHost(uint16_t value);

    /**
     * Convert a 32-bit value from host to little-endian byte order
     * 
     * @param value The value to convert
     * @return Value in little-endian byte order
     */
    static uint32_t hostToLE32(uint32_t value);

    /**
     * Convert a 32-bit value from little-endian to host byte order
     * 
     * @param value The value to convert
     * @return Value in host byte order
     */
    static uint32_t le32ToHost(uint32_t value);

    // -------------------------------------------------------------------------------------------
    // TWO'S COMPLEMENT HELPERS
    // -------------------------------------------------------------------------------------------

    /**
     * Get 2's complement representation of int32_t (for TLV encoding)
     * 
     * @param value The value to convert
     * @return 32-bit unsigned representation
     */
    static uint32_t get2ComplementInt32(int32_t value);

    /**
     * Get 2's complement representation of int16_t (for TLV encoding)
     * 
     * @param value The value to convert
     * @return 16-bit unsigned representation
     */
    static uint16_t get2ComplementInt16(int16_t value);

    // -------------------------------------------------------------------------------------------
    // CALIBRATION/CONVERSION CONSTANTS
    // -------------------------------------------------------------------------------------------

    // Constants for coordinate and distance conversion
    static constexpr double METERS_PER_DEGREE_LATITUDE = 111319.0; // Approximate meters per degree of latitude
    static constexpr double EARTH_RADIUS_METERS = 6371000.0; // Earth radius in meters
    static constexpr double PI_VALUE = 3.14159265358979323846;
    static constexpr double DEGREES_TO_RADIANS = PI_VALUE / 180.0;
    static constexpr double RADIANS_TO_DEGREES = 180.0 / PI_VALUE;
    static constexpr double METERS_PER_FEET = 0.3048;
    static constexpr double METERS_PER_NAUTICAL_MILE = 1852.0;
    static constexpr double KILOMETERS_PER_METER = 0.001;
    static constexpr double SECONDS_PER_HOUR = 3600.0;
    static constexpr double METERS_PER_SECOND_TO_KPH = 3.6;
    static constexpr double METERS_PER_SECOND_TO_KNOTS = 1.94384;
    static constexpr double INT32_MAX_AS_DOUBLE = 2147483647.0;
    static constexpr double INT32_MIN_AS_DOUBLE = -2147483648.0;
};

#endif // #ifndef CONVERSION_UTILS_H
