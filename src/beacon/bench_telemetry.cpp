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

#include "bench_telemetry.h"

#ifdef TELEMETRIE_BATTERIE

#include <Preferences.h>
#include "beacon_manager.h"
#include "../hardware/battery_monitor.h"

// Global instance
BenchTelemetry benchTelemetry;

// Locally administered OUI (bit 1 of the first byte set), so no vendor's range is borrowed
static const uint8_t BENCH_OUI[3] = {0x02, 0x42, 0x49};
static const uint8_t BENCH_TYPE = 0x01;

// ================================================================================================
// ENCODING HELPERS
// ================================================================================================

static uint8_t putU32(uint8_t* p, uint32_t v) {
    p[0] = static_cast<uint8_t>(v >> 24);
    p[1] = static_cast<uint8_t>(v >> 16);
    p[2] = static_cast<uint8_t>(v >> 8);
    p[3] = static_cast<uint8_t>(v);
    return 4;
}

static uint8_t putU16(uint8_t* p, uint32_t v) {
    // Saturated rather than wrapped: a gap of 70 s must not read as 4 s
    const uint16_t s = v > 0xFFFF ? 0xFFFF : static_cast<uint16_t>(v);
    p[0] = static_cast<uint8_t>(s >> 8);
    p[1] = static_cast<uint8_t>(s);
    return 2;
}

// ================================================================================================
// CONSTRUCTOR AND INITIALIZATION
// ================================================================================================

BenchTelemetry::BenchTelemetry()
    : _boot(0), _seq(0), _errors(0), _last_ms(0), _started(false) {
}

void BenchTelemetry::begin() {
    Preferences prefs;
    if (prefs.begin("bench", false)) {
        _boot = prefs.getUInt("boots", 0) + 1;
        prefs.putUInt("boots", _boot);
        prefs.end();
    }
}

// ================================================================================================
// SENDING
// ================================================================================================

void BenchTelemetry::update(bool live_fix) {
    if (_started && !hasElapsed(_last_ms, BENCH_TELEMETRY_PERIOD_MS)) {
        return;
    }
    _started = true;
    _last_ms = millis();

    uint8_t element[2 + 4 + 26];
    uint8_t i = 0;
    element[i++] = 0xDD;
    element[i++] = 0;  // length, set below
    element[i++] = BENCH_OUI[0];
    element[i++] = BENCH_OUI[1];
    element[i++] = BENCH_OUI[2];
    element[i++] = BENCH_TYPE;
    element[i++] = BENCH_TELEMETRY_VERSION;

    i += putU32(element + i, _seq);
    i += putU32(element + i, _boot);
    i += putU32(element + i, millis() / 1000);
    i += putU16(element + i, batteryMonitor.getVoltageMv());
    i += putU16(element + i, batteryMonitor.getPinVoltageMv());
    i += putU32(element + i, beaconManager.getFrameCount());
    i += putU16(element + i, beaconManager.getMaxIntervalMs());
    i += putU16(element + i, beaconManager.getOverIntervalCount());

    uint8_t flags = 0;
    if (live_fix) flags |= 0x01;
    if (beaconManager.hasHomeSet()) flags |= 0x02;
    if (beaconManager.isPositionFrozen()) flags |= 0x04;
    if (batteryMonitor.isLow()) flags |= 0x08;
    element[i++] = flags;

    element[1] = static_cast<uint8_t>(i - 2);

    if (beaconManager.sendVendorFrame(element, i)) {
        _seq++;
    } else {
        _errors++;
    }
}

#endif // TELEMETRIE_BATTERIE
