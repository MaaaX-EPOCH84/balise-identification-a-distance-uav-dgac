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

#ifndef DEBUG_UTILS_H
#define DEBUG_UTILS_H

#include <Arduino.h>
#include "../core/biadud_platform.h"

// ================================================================================================
// DEBUG MACROS AND CONFIGURATION
// ================================================================================================

// Debug levels
#define DEBUG_LEVEL_NONE     0
#define DEBUG_LEVEL_ERROR    1
#define DEBUG_LEVEL_WARNING  2
#define DEBUG_LEVEL_INFO     3
#define DEBUG_LEVEL_VERBOSE  4
#define DEBUG_LEVEL_ALL      5

// Default debug level (can be overridden in build flags)
#ifndef DEBUG_LEVEL
    #define DEBUG_LEVEL DEBUG_LEVEL_INFO
#endif

// Debug output macros
#if DEBUG_LEVEL >= DEBUG_LEVEL_ERROR
    #define DEBUG_ERROR(x)      Serial.print("[ERROR] "); Serial.println(x)
    #define DEBUG_ERRORF(...)   Serial.print("[ERROR] "); Serial.printf(__VA_ARGS__); Serial.println()
#else
    #define DEBUG_ERROR(x)
    #define DEBUG_ERRORF(...)
#endif

#if DEBUG_LEVEL >= DEBUG_LEVEL_WARNING
    #define DEBUG_WARN(x)       Serial.print("[WARN] "); Serial.println(x)
    #define DEBUG_WARNF(...)    Serial.print("[WARN] "); Serial.printf(__VA_ARGS__); Serial.println()
#else
    #define DEBUG_WARN(x)
    #define DEBUG_WARNF(...)
#endif

#if DEBUG_LEVEL >= DEBUG_LEVEL_INFO
    #define DEBUG_INFO(x)       Serial.print("[INFO] "); Serial.println(x)
    #define DEBUG_INFOF(...)    Serial.print("[INFO] "); Serial.printf(__VA_ARGS__); Serial.println()
#else
    #define DEBUG_INFO(x)
    #define DEBUG_INFOF(...)
#endif

#if DEBUG_LEVEL >= DEBUG_LEVEL_VERBOSE
    #define DEBUG_VERBOSE(x)    Serial.print("[VERBOSE] "); Serial.println(x)
    #define DEBUG_VERBOSEF(...) Serial.print("[VERBOSE] "); Serial.printf(__VA_ARGS__); Serial.println()
#else
    #define DEBUG_VERBOSE(x)
    #define DEBUG_VERBOSEF(...)
#endif

// ================================================================================================
// DEBUG UTILITY CLASS
// ================================================================================================

class DebugUtils
{
public:
    // -------------------------------------------------------------------------------------------
    // INITIALIZATION
    // -------------------------------------------------------------------------------------------

    /**
     * Initialize debug utilities
     * 
     * @param baud_rate Serial baud rate for debug output
     * @param wait_for_serial Wait for serial connection (for USB debugging)
     */
    static void begin(uint32_t baud_rate = DEBUG_SERIAL_BAUD, bool wait_for_serial = false);

    // -------------------------------------------------------------------------------------------
    // DEBUG OUTPUT
    // -------------------------------------------------------------------------------------------

    /**
     * Print a debug message with timestamp
     * 
     * @param message The message to print
     */
    static void print(const String& message);

    /**
     * Print a debug message with timestamp and newline
     * 
     * @param message The message to print
     */
    static void println(const String& message);

    /**
     * Print a formatted debug message with timestamp
     * 
     * @param format Format string
     * @param ... Arguments
     */
    static void printf(const char* format, ...);

    // -------------------------------------------------------------------------------------------
    // TIMESTAMP FUNCTIONS
    // -------------------------------------------------------------------------------------------

    /**
     * Get current timestamp as formatted string
     * 
     * @return Timestamp string (HH:MM:SS.mmm)
     */
    static String getTimestamp();

    /**
     * Get uptime as formatted string
     * 
     * @return Uptime string (DDd HH:MM:SS)
     */
    static String getUptime();

    /**
     * Get uptime in milliseconds
     * 
     * @return Uptime in milliseconds
     */
    static uint32_t getUptimeMs();

    // -------------------------------------------------------------------------------------------
    // MEMORY STATISTICS
    // -------------------------------------------------------------------------------------------

    /**
     * Print memory statistics
     */
    static void printMemoryStats();

    /**
     * Get free heap size
     * 
     * @return Free heap size in bytes
     */
    static size_t getFreeHeap();

    /**
     * Get maximum heap size
     * 
     * @return Maximum heap size in bytes
     */
    static size_t getMaxHeap();

    // -------------------------------------------------------------------------------------------
    // BINARY/HEX OUTPUT
    // -------------------------------------------------------------------------------------------

    /**
     * Print data in hexadecimal format
     * 
     * @param data Pointer to data
     * @param length Number of bytes to print
     * @param width Bytes per line (default: 16)
     */
    static void printHex(const uint8_t* data, uint16_t length, uint8_t width = 16);

    /**
     * Print data in binary format
     * 
     * @param data Pointer to data
     * @param length Number of bytes to print
     */
    static void printBinary(const uint8_t* data, uint16_t length);

    /**
     * Convert a value to hexadecimal string
     * 
     * @param value The value to convert
     * @param width Minimum width (pad with leading zeros)
     * @return Hexadecimal string
     */
    static String toHex(uint32_t value, uint8_t width = 8);

    /**
     * Convert a byte to hexadecimal string
     * 
     * @param value The byte value to convert
     * @return Two-character hexadecimal string
     */
    static String toHexByte(uint8_t value);

    // -------------------------------------------------------------------------------------------
    // DUMP FUNCTIONS
    // -------------------------------------------------------------------------------------------

    /**
     * Dump a buffer to serial
     * 
     * @param buffer Pointer to buffer
     * @param length Number of bytes
     * @param label Optional label to print before the dump
     */
    static void dumpBuffer(const uint8_t* buffer, uint16_t length, const char* label = nullptr);

    // -------------------------------------------------------------------------------------------
    // PROGRESS INDICATION
    // -------------------------------------------------------------------------------------------

    /**
     * Print a progress bar
     * 
     * @param current Current progress value
     * @param total Total value (100%)
     * @param width Width of the progress bar in characters
     * @param label Optional label
     */
    static void printProgress(uint32_t current, uint32_t total, uint8_t width = 40, const char* label = nullptr);

    /**
     * Print a spinner
     * 
     * @param index Spinner index (0-3)
     * @param message Message to print with spinner
     */
    static void printSpinner(uint8_t index, const String& message);
};

#endif // #ifndef DEBUG_UTILS_H
