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

#include "debug_utils.h"

// ================================================================================================
// INITIALIZATION
// ================================================================================================

void DebugUtils::begin(uint32_t baud_rate, bool wait_for_serial) {
    Serial.begin(baud_rate);
    
    if (wait_for_serial) {
        while (!Serial) {
            delay(10);
        }
    }
    
    delay(100); // Give serial some time to initialize
}

// ================================================================================================
// DEBUG OUTPUT
// ================================================================================================

void DebugUtils::print(const String& message) {
    Serial.print(getTimestamp());
    Serial.print(" ");
    Serial.print(message);
}

void DebugUtils::println(const String& message) {
    print(message);
    Serial.println();
}

// ================================================================================================
// TIMESTAMP FUNCTIONS
// ================================================================================================

String DebugUtils::getTimestamp() {
    uint32_t ms = millis();
    uint32_t seconds = ms / 1000;
    uint32_t minutes = seconds / 60;
    uint32_t hours = minutes / 60;
    
    uint8_t hour = hours % 24;
    uint8_t minute = minutes % 60;
    uint8_t second = seconds % 60;
    uint16_t millisecond = ms % 1000;
    
    char buffer[16];
    snprintf(buffer, sizeof(buffer), "%02u:%02u:%02u.%03u", hour, minute, second, millisecond);
    return String(buffer);
}

String DebugUtils::getUptime() {
    uint32_t ms = millis();
    uint32_t seconds = ms / 1000;
    uint32_t minutes = seconds / 60;
    uint32_t hours = minutes / 60;
    uint32_t days = hours / 24;
    
    hours %= 24;
    minutes %= 60;
    seconds %= 60;
    
    char buffer[32];
    if (days > 0) {
        snprintf(buffer, sizeof(buffer), "%ud %02u:%02u:%02u", days, hours, minutes, seconds);
    } else {
        snprintf(buffer, sizeof(buffer), "%02u:%02u:%02u", hours, minutes, seconds);
    }
    return String(buffer);
}

uint32_t DebugUtils::getUptimeMs() {
    return millis();
}

// ================================================================================================
// MEMORY STATISTICS
// ================================================================================================

void DebugUtils::printMemoryStats() {
    DEBUG_INFOF("Free Heap: %u bytes", getFreeHeap());
}

size_t DebugUtils::getFreeHeap() {
    return ::getFreeHeap();
}

size_t DebugUtils::getMaxHeap() {
    #if defined(PLATFORM_ESP32C3) || defined(PLATFORM_ESP32S3) || defined(PLATFORM_ESP32)
        return ESP.getHeapSize();
    #elif defined(PLATFORM_ESP8266)
        return ESP.getHeapSize();
    #else
        return 0;
    #endif
}

// ================================================================================================
// BINARY/HEX OUTPUT
// ================================================================================================

void DebugUtils::printHex(const uint8_t* data, uint16_t length, uint8_t width) {
    for (uint16_t i = 0; i < length; i++) {
        if (i % width == 0) {
            if (i > 0) {
                Serial.println();
            }
            Serial.print(toHex(i, 4));
            Serial.print(": ");
        }
        
        Serial.print(toHexByte(data[i]));
        Serial.print(" ");
        
        if (i % width == width - 1) {
            Serial.println();
        }
    }
    
    if (length % width != 0) {
        Serial.println();
    }
}

void DebugUtils::printBinary(const uint8_t* data, uint16_t length) {
    for (uint16_t i = 0; i < length; i++) {
        Serial.print("0x");
        Serial.print(toHexByte(data[i]));
        Serial.print(": ");
        
        for (int8_t j = 7; j >= 0; j--) {
            Serial.print((data[i] >> j) & 1);
        }
        Serial.println();
    }
}

String DebugUtils::toHex(uint32_t value, uint8_t width) {
    String result;
    for (uint8_t i = 0; i < width; i++) {
        uint8_t nibble = (value >> ((width - 1 - i) * 4)) & 0x0F;
        char c = (nibble < 10) ? ('0' + nibble) : ('A' + nibble - 10);
        result += c;
    }
    return result;
}

String DebugUtils::toHexByte(uint8_t value) {
    char buffer[3];
    snprintf(buffer, sizeof(buffer), "%02X", value);
    return String(buffer);
}

// ================================================================================================
// DUMP FUNCTIONS
// ================================================================================================

void DebugUtils::dumpBuffer(const uint8_t* buffer, uint16_t length, const char* label) {
    if (label) {
        Serial.print("=== ");
        Serial.print(label);
        Serial.print(" (");
        Serial.print(length);
        Serial.println(" bytes) ===");
    }
    
    printHex(buffer, length);
    
    if (label) {
        Serial.println("========================");
    }
}

// ================================================================================================
// PROGRESS INDICATION
// ================================================================================================

void DebugUtils::printProgress(uint32_t current, uint32_t total, uint8_t width, const char* label) {
    if (total == 0) {
        total = 1;
    }
    
    if (label) {
        Serial.print(label);
        Serial.print(": ");
    }
    
    uint32_t filled = (current * width) / total;
    
    Serial.print("[");
    for (uint8_t i = 0; i < width; i++) {
        Serial.print(i < filled ? "=" : " ");
    }
    Serial.print("] ");
    Serial.print(current);
    Serial.print("/");
    Serial.print(total);
    Serial.print(" (");
    Serial.print((current * 100) / total);
    Serial.println(")");
}

void DebugUtils::printSpinner(uint8_t index, const String& message) {
    const char spinners[] = "|/-\\|";
    char spinner = spinners[index % 4];
    
    Serial.print("\r");
    Serial.print(message);
    Serial.print(" ");
    Serial.print(spinner);
    Serial.print("  ");
    Serial.flush();
}
