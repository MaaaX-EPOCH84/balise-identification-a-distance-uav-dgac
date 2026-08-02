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

#include "gps_ublox.h"
#include "../core/biadud_platform.h"

// Out-of-class definition required (pre-C++17) since COMMON_BAUD_RATES is odr-used via indexing.
constexpr uint32_t UbloxGpsManager::COMMON_BAUD_RATES[];

// ================================================================================================
// CONSTRUCTOR / DESTRUCTOR
// ================================================================================================

UbloxGpsManager::UbloxGpsManager() : _parser() {
    _status = GpsStatus::GPS_NOT_DETECTED;
    _initialized = false;
    _current_baud = 0;
    _current_rate = 0;
}

UbloxGpsManager::~UbloxGpsManager() {
    // Nothing to clean up
}

// ================================================================================================
// INITIALIZATION
// ================================================================================================

bool UbloxGpsManager::initialize(uint32_t baud_rate, uint8_t refresh_rate) {
    _current_baud = baud_rate;
    _current_rate = refresh_rate;
    
    // Initialize GPS serial port at 9600 baud first
    // Most UBLOX GPS modules default to 9600 baud
    #if HAS_HARDWARE_SERIAL
        serialGPS.begin(9600, SERIAL_8N1, PIN_GPS_RX, PIN_GPS_TX);
    #else
        serialGPS.begin(9600);
    #endif
    
    // Wait for GPS to power up (UBLOX needs ~600ms)
    delay(600);
    
    // Set the desired baud rate
    setBaudRate(baud_rate);
    
    // Change our serial port to the new baud rate
    changeSerialBaudRate(baud_rate);
    
    // Configure NMEA sentences to limit output
    configureNmeaSentences();
    
    // Configure refresh rate
    configureUbxRate(refresh_rate);
    
    // Wait a bit for configuration to take effect
    delay(250);
    
    // Flush any remaining data
    flushSerial();
    
    _initialized = true;
    _status = GpsStatus::GPS_OK;
    
    return true;
}

void UbloxGpsManager::setBaudRate(uint32_t baud_rate) {
    _current_baud = baud_rate;
    
    // UBLOX command to set baud rate
    // $PUBX,41,portId,inProto,outProto,baudrate,autobauding*cs<CR><LF>
    // portId: 1 for UART1
    // inProto: 0x0007 for NMEA
    // outProto: 0x0003 for UBX + NMEA
    // autobauding: 0 to disable
    
    char baud_cmd[48];
    snprintf(baud_cmd, sizeof(baud_cmd), "$PUBX,41,1,0007,0003,%u,0", baud_rate);
    addNmeaChecksum(baud_cmd);
    strcat(baud_cmd, "\r\n");
    
    // Try sending the command at all common baud rates
    // This ensures the GPS will receive the command regardless of current baud rate
    for (uint8_t i = 0; i < NUM_COMMON_BAUD_RATES; i++) {
        uint32_t try_baud = COMMON_BAUD_RATES[i];
        
        // Change to this baud rate
        changeSerialBaudRate(try_baud);
        delay(100);
        
        // Send the command multiple times to ensure it's received
        for (uint8_t j = 0; j < 3; j++) {
            serialGPS.write(baud_cmd, strlen(baud_cmd));
            delay(50);
        }
        
        // Wait a bit between baud rate attempts
        delay(150);
    }
    
    // Restore to the target baud rate
    changeSerialBaudRate(baud_rate);
}

void UbloxGpsManager::setRefreshRate(uint8_t rate) {
    _current_rate = rate;
    configureUbxRate(rate);
}

// ================================================================================================
// SERIAL DATA PROCESSING
// ================================================================================================

void UbloxGpsManager::processSerialData(char c) {
    _parser.parse(c);
}

void UbloxGpsManager::processSerialData(const char* buffer, uint16_t length) {
    for (uint16_t i = 0; i < length; i++) {
        _parser.parse(buffer[i]);
    }
}

// ================================================================================================
// DIAGNOSTIC FUNCTIONS
// ================================================================================================

void UbloxGpsManager::echoGpsData(uint32_t duration_ms) {
    uint32_t end_time = millis() + duration_ms;
    
    while (millis() < end_time) {
        while (serialGPS.available()) {
            char c = serialGPS.read();
            Serial.write(c);
        }
        yield();
    }
}

void UbloxGpsManager::sendCommand(const char* command) {
    if (command == nullptr || *command == '\0') {
        return;
    }
    
    serialGPS.write(command, strlen(command));
}

void UbloxGpsManager::sendUbxPacket(const uint8_t* packet, uint16_t length) {
    // Ensure packet has checksum
    if (length > 2 && packet[0] == UBX_SYNC_CHAR_1 && packet[1] == UBX_SYNC_CHAR_2) {
        // Packet already has sync chars, calculate checksum
        calculateUbxChecksum(const_cast<uint8_t*>(packet), length);
    }
    
    // Send the packet
    for (uint16_t i = 0; i < length; i++) {
        serialGPS.write(packet[i]);
    }
}

// ================================================================================================
// UTILITY FUNCTIONS
// ================================================================================================

void UbloxGpsManager::calculateUbxChecksum(uint8_t* packet, uint16_t length) {
    // UBX checksum is calculated over the payload (bytes 2 to length-1)
    // and stored in the last 2 bytes (CK_A and CK_B)
    
    if (length < 4) {
        return; // Packet too short
    }
    
    uint8_t CK_A = 0;
    uint8_t CK_B = 0;
    
    // Calculate checksum over payload (bytes 2 to length-2)
    for (uint16_t i = 2; i < length - 2; i++) {
        CK_A += packet[i];
        CK_B += CK_A;
    }
    
    // Store checksum at the end
    packet[length - 2] = CK_A;
    packet[length - 1] = CK_B;
}

void UbloxGpsManager::addNmeaChecksum(char* buffer) {
    if (buffer == nullptr || buffer[0] != '$') {
        return;
    }
    
    // Find the end of the string (before the checksum)
    uint16_t length = strlen(buffer);
    
    // Calculate checksum (XOR of all bytes between $ and *)
    uint8_t checksum = 0;
    for (uint16_t i = 1; i < length; i++) {
        checksum ^= buffer[i];
    }
    
    // Append checksum
    sprintf(buffer + length, "*%02X", checksum);
}

// ================================================================================================
// INTERNAL FUNCTIONS
// ================================================================================================

void UbloxGpsManager::changeSerialBaudRate(uint32_t baud_rate) {
    #if HAS_HARDWARE_SERIAL
        // For ESP32, we need to flush and update the baud rate
        serialGPS.flush();
        serialGPS.updateBaudRate(baud_rate);
    #else
        // For ESP8266 SoftwareSerial
        serialGPS.flush();
        serialGPS.end();
        serialGPS.begin(baud_rate);
    #endif
    
    delay(10); // Small delay to allow UART to reconfigure
}

void UbloxGpsManager::flushSerial() {
    #if HAS_HARDWARE_SERIAL
        while (serialGPS.available()) {
            serialGPS.read();
        }
        serialGPS.flush();
    #else
        while (serialGPS.available()) {
            serialGPS.read();
        }
    #endif
}

void UbloxGpsManager::configureNmeaSentences() {
    // Send commands to disable unnecessary NMEA sentences
    // This is based on the original code in biadud_gps.cpp (lines 252-258)
    
    // Limit to GGA and RMC only
    sendCommand(UBX_NMEA_LIMIT_SENTENCES);
    
    // Also send PQTXT command to disable text messages
    char txt_cmd[] = "$PQTXT,W,0,0*22\r\n";
    sendCommand(txt_cmd);
    
    delay(100);
}

void UbloxGpsManager::configureUbxRate(uint8_t rate) {
    // UBX-CFG-RATE message to set measurement rate
    // Format: B5 62 06 08 06 00 F4 01 01 00 00 00 xx xx
    // measRate: 0xF4 0x01 = 500ms = 2Hz (default)
    // navRate: 0x01 = 1 (must match measRate)
    // timeRef: 0x00 = UTC time
    
    if (rate == 0) {
        rate = 1; // Minimum 1 Hz
    }
    
    uint32_t period_ms = 1000 / rate;
    
    // UBX packet structure:
    // [0-1]: Sync chars (0xB5, 0x62)
    // [2-3]: Message class and ID (CFG-RATE = 0x06, 0x08)
    // [4-5]: Length (6 bytes of payload)
    // [6-7]: measRate (little-endian)
    // [8-9]: navRate (little-endian)
    // [10-11]: timeRef (little-endian, 0=UTC, 1=GPS time)
    // [12-13]: Checksum
    
    uint8_t ubx_rate_packet[] = {
        UBX_SYNC_CHAR_1, UBX_SYNC_CHAR_2,
        UBX_CLASS_CFG, UBX_MSG_CFG_RATE,
        0x06, 0x00,                     // Length: 6 bytes
        0x00, 0x00,                     // measRate (will be set below)
        0x01, 0x00,                     // navRate = 1
        0x00, 0x00,                     // timeRef = 0 (UTC time)
        0x00, 0x00                      // Checksum (will be calculated)
    };
    
    // Set measurement rate (little-endian)
    ubx_rate_packet[6] = period_ms & 0xFF;
    ubx_rate_packet[7] = (period_ms >> 8) & 0xFF;
    
    // Calculate and set checksum
    calculateUbxChecksum(ubx_rate_packet, sizeof(ubx_rate_packet));
    
    // Send the packet multiple times to ensure it's received
    for (uint8_t i = 0; i < 3; i++) {
        sendUbxPacket(ubx_rate_packet, sizeof(ubx_rate_packet));
        delay(50);
    }
    
    delay(200);
}

bool UbloxGpsManager::waitForResponse(uint32_t timeout_ms) {
    uint32_t start_time = millis();
    
    while (millis() - start_time < timeout_ms) {
        if (serialGPS.available()) {
            return true;
        }
        delay(10);
    }
    
    return false;
}
