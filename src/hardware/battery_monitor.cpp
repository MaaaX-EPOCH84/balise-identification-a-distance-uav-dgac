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

#include "battery_monitor.h"

// Global instance
BatteryMonitor batteryMonitor;

// ================================================================================================
// CONSTRUCTOR
// ================================================================================================

BatteryMonitor::BatteryMonitor()
    : _pin(-1), _pin_mv(0), _cell_mv(0), _last_read_ms(0) {
}

// ================================================================================================
// INITIALIZATION
// ================================================================================================

void BatteryMonitor::begin(int8_t pin) {
    _pin = pin;
    if (!isAvailable()) {
        return;
    }

    analogSetPinAttenuation(_pin, ADC_11db);
    read();
}

// ================================================================================================
// MEASUREMENT
// ================================================================================================

void BatteryMonitor::update() {
    if (!isAvailable() || !hasElapsed(_last_read_ms, BATTERY_READ_PERIOD_MS)) {
        return;
    }
    read();
}

void BatteryMonitor::read() {
    uint32_t sum = 0;
    for (uint8_t i = 0; i < BATTERY_SAMPLES; i++) {
        sum += analogReadMilliVolts(_pin);
    }

    _pin_mv = (sum + BATTERY_SAMPLES / 2) / BATTERY_SAMPLES;
    _cell_mv = static_cast<uint32_t>(_pin_mv * BATTERY_DIVIDER_RATIO + 0.5);
    _last_read_ms = millis();
}
