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

#ifndef BENCH_TELEMETRY_H
#define BENCH_TELEMETRY_H

#include <Arduino.h>
#include "../core/biadud_platform.h"

// ================================================================================================
// BENCH TELEMETRY - BENCH BUILDS ONLY
// ================================================================================================
// Battery and emission figures sent over the air, so that a run on battery can be logged with
// the board's USB unplugged. It goes out in a frame of its own, never inside the DGAC element:
// the order reserves TLV types 12 to 200. Built only with -D TELEMETRIE_BATTERIE, which only
// the esp32-c3-telemetrie environment sets: a flight build carries none of it.
//
// Element layout, big-endian like the DGAC TLVs:
//   DD len | 02 42 49 (locally administered OUI) | 01 (type) | 01 (version)
//   seq u32 | boot u32 | uptime s u32 | cell mV u16 | pin mV u16
//   DGAC frames u32 | longest DGAC gap ms u16 | gaps over 3 s u16 | flags u8
// Flags: bit 0 live fix, bit 1 home set, bit 2 position frozen.
// ================================================================================================

#ifdef TELEMETRIE_BATTERIE

// One frame every 10 s: about 2 000 samples over a full discharge
#define BENCH_TELEMETRY_PERIOD_MS   10000

#define BENCH_TELEMETRY_VERSION     1

class BenchTelemetry {
public:
    BenchTelemetry();

    /**
     * Count this boot in NVS, so that a reset shows in the log as a new boot number
     */
    void begin();

    /**
     * Send a frame when the period has elapsed. Call it on every loop iteration once the radio
     * is up; the first frame goes out on the first call.
     *
     * @param live_fix true when the GPS currently has a fix
     */
    void update(bool live_fix);

    uint32_t getBootCount() const { return _boot; }
    uint32_t getSentCount() const { return _seq; }
    uint32_t getErrorCount() const { return _errors; }

private:
    uint32_t _boot;
    uint32_t _seq;
    uint32_t _errors;
    uint32_t _last_ms;
    bool _started;
};

// Global instance
extern BenchTelemetry benchTelemetry;

#endif // TELEMETRIE_BATTERIE

#endif // BENCH_TELEMETRY_H
