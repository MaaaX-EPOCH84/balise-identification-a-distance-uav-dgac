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

#ifndef BATTERY_MONITOR_H
#define BATTERY_MONITOR_H

#include <Arduino.h>
#include "../core/biadud_platform.h"

// ================================================================================================
// BATTERY MEASUREMENT CONFIGURATION
// ================================================================================================

// The cell is read through a divider of two equal resistors: the pin sees half the voltage
#define BATTERY_DIVIDER_RATIO       2.0

// Samples averaged per reading: the ADC is noisy, and more so while the radio transmits
#define BATTERY_SAMPLES             16

// Time between two readings: a Li-ion cell discharges over hours, not seconds
#define BATTERY_READ_PERIOD_MS      1000

// ================================================================================================
// BATTERY MONITOR CLASS
// ================================================================================================

/**
 * Reads the cell voltage on an ADC1 pin.
 *
 * ADC1 only: ADC2 is shared with the WiFi radio, which never stops here. With 11 dB of
 * attenuation the ESP32-C3 measures 0 to 2500 mV, enough for the 1.5 to 2.1 V the divider
 * delivers from a 3.0 to 4.2 V cell. analogReadMilliVolts() applies the eFuse calibration.
 */
class BatteryMonitor {
public:
    BatteryMonitor();

    /**
     * Configure the pin and take a first reading
     *
     * @param pin ADC1 pin wired to the divider, or -1 when nothing is wired
     */
    void begin(int8_t pin);

    /**
     * Take a new reading when the read period has elapsed. Call it on every loop iteration.
     */
    void update();

    /**
     * Is a battery measurement wired on this board?
     */
    bool isAvailable() const { return _pin >= 0; }

    /**
     * Cell voltage of the last reading, in millivolts (0 when unavailable)
     */
    uint32_t getVoltageMv() const { return _cell_mv; }

    /**
     * Voltage seen on the pin by the last reading, in millivolts, before the divider correction
     */
    uint32_t getPinVoltageMv() const { return _pin_mv; }

private:
    void read();

    int8_t _pin;
    uint32_t _pin_mv;
    uint32_t _cell_mv;
    uint32_t _last_read_ms;
};

// Global instance
extern BatteryMonitor batteryMonitor;

#endif // BATTERY_MONITOR_H
