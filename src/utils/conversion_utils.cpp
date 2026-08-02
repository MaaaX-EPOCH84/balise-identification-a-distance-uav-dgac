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

#include "conversion_utils.h"

// ================================================================================================
// COORDINATE CONVERSION
// ================================================================================================

double ConversionUtils::degreesToRadians(double degrees) {
    return degrees * DEGREES_TO_RADIANS;
}

double ConversionUtils::radiansToDegrees(double radians) {
    return radians * RADIANS_TO_DEGREES;
}

void ConversionUtils::decimalDegreesToNMEA(double decimal_degrees, uint16_t& degrees, double& minutes) {
    degrees = static_cast<uint16_t>(abs(decimal_degrees));
    minutes = (abs(decimal_degrees) - degrees) * 60.0;
}

double ConversionUtils::nmeaToDecimalDegrees(uint16_t degrees, double minutes, bool is_latitude) {
    double result = degrees + (minutes / 60.0);
    // Note: For latitude, South is negative; for longitude, West is negative
    // The sign should be handled by the caller based on the hemisphere indicator
    return result;
}

// ================================================================================================
// DISTANCE CONVERSION
// ================================================================================================

double ConversionUtils::metersToFeet(double meters) {
    return meters / METERS_PER_FEET;
}

double ConversionUtils::feetToMeters(double feet) {
    return feet * METERS_PER_FEET;
}

double ConversionUtils::metersToNauticalMiles(double meters) {
    return meters / METERS_PER_NAUTICAL_MILE;
}

double ConversionUtils::nauticalMilesToMeters(double nautical_miles) {
    return nautical_miles * METERS_PER_NAUTICAL_MILE;
}

double ConversionUtils::metersToKilometers(double meters) {
    return meters * KILOMETERS_PER_METER;
}

double ConversionUtils::kilometersToMeters(double kilometers) {
    return kilometers / KILOMETERS_PER_METER;
}

// ================================================================================================
// SPEED CONVERSION
// ================================================================================================

double ConversionUtils::mpsToKph(double mps) {
    return mps * METERS_PER_SECOND_TO_KPH;
}

double ConversionUtils::kphToMps(double kph) {
    return kph / METERS_PER_SECOND_TO_KPH;
}

double ConversionUtils::mpsToKnots(double mps) {
    return mps * METERS_PER_SECOND_TO_KNOTS;
}

double ConversionUtils::knotsToMps(double knots) {
    return knots / METERS_PER_SECOND_TO_KNOTS;
}

// ================================================================================================
// ANGLE CONVERSION
// ================================================================================================

double ConversionUtils::normalizeAngle(double angle) {
    while (angle >= 360.0) {
        angle -= 360.0;
    }
    while (angle < 0.0) {
        angle += 360.0;
    }
    return angle;
}

String ConversionUtils::headingToCardinal(double heading) {
    heading = normalizeAngle(heading);
    
    const char* cardinals[] = {
        "N", "NNE", "NE", "ENE", "E", "ESE", "SE", "SSE",
        "S", "SSW", "SW", "WSW", "W", "WNW", "NW", "NNW"
    };
    
    uint8_t index = static_cast<uint8_t>((heading + 11.25) / 22.5);
    index %= 16;
    return String(cardinals[index]);
}

double ConversionUtils::cardinalToHeading(const String& direction) {
    struct CardinalMapping {
        const char* name;
        double heading;
    };
    
    static const CardinalMapping mappings[] = {
        {"N", 0.0}, {"NNE", 22.5}, {"NE", 45.0}, {"ENE", 67.5},
        {"E", 90.0}, {"ESE", 112.5}, {"SE", 135.0}, {"SSE", 157.5},
        {"S", 180.0}, {"SSW", 202.5}, {"SW", 225.0}, {"WSW", 247.5},
        {"W", 270.0}, {"WNW", 292.5}, {"NW", 315.0}, {"NNW", 337.5}
    };
    
    String upper = direction;
    upper.toUpperCase();
    
    for (const auto& map : mappings) {
        if (upper == map.name) {
            return map.heading;
        }
    }
    
    return -1.0; // Invalid
}

// ================================================================================================
// COORDINATE FORMAT CONVERSION
// ================================================================================================

int32_t ConversionUtils::decimalDegreesToCentidegreesE7(double decimal_degrees) {
    // Clamp to valid range for int32
    if (decimal_degrees < INT32_MIN_AS_DOUBLE) {
        decimal_degrees = INT32_MIN_AS_DOUBLE;
    }
    if (decimal_degrees > INT32_MAX_AS_DOUBLE) {
        decimal_degrees = INT32_MAX_AS_DOUBLE;
    }
    
    return static_cast<int32_t>(round(decimal_degrees * 1e7));
}

double ConversionUtils::centidegreesE7ToDecimalDegrees(int32_t centidegrees_e7) {
    return static_cast<double>(centidegrees_e7) / 1e7;
}

// ================================================================================================
// TEMPERATURE CONVERSION
// ================================================================================================

double ConversionUtils::celsiusToFahrenheit(double celsius) {
    return (celsius * 9.0 / 5.0) + 32.0;
}

double ConversionUtils::fahrenheitToCelsius(double fahrenheit) {
    return (fahrenheit - 32.0) * 5.0 / 9.0;
}

// ================================================================================================
// PRESSURE CONVERSION
// ================================================================================================

double ConversionUtils::hpaToMbar(double hpa) {
    return hpa; // 1 hPa = 1 mbar
}

double ConversionUtils::mbarToHpa(double mbar) {
    return mbar; // 1 mbar = 1 hPa
}

// ================================================================================================
// ALTITUDE CONVERSION
// ================================================================================================

double ConversionUtils::altitudeToFeet(int16_t altitude_msl) {
    return static_cast<double>(altitude_msl) / METERS_PER_FEET;
}

// ================================================================================================
// BYTE ORDER CONVERSION
// ================================================================================================

uint16_t ConversionUtils::hostToLE16(uint16_t value) {
    return ((value >> 8) & 0x00FF) | ((value << 8) & 0xFF00);
}

uint16_t ConversionUtils::le16ToHost(uint16_t value) {
    return hostToLE16(value);
}

uint32_t ConversionUtils::hostToLE32(uint32_t value) {
    return ((value >> 24) & 0x000000FF) | 
           ((value >> 8) & 0x0000FF00) | 
           ((value << 8) & 0x00FF0000) | 
           ((value << 24) & 0xFF000000);
}

uint32_t ConversionUtils::le32ToHost(uint32_t value) {
    return hostToLE32(value);
}

// ================================================================================================
// TWO'S COMPLEMENT HELPERS
// ================================================================================================

uint32_t ConversionUtils::get2ComplementInt32(int32_t value) {
    return static_cast<uint32_t>(value) & 0xFFFFFFFF;
}

uint16_t ConversionUtils::get2ComplementInt16(int16_t value) {
    return static_cast<uint16_t>(value) & 0xFFFF;
}
