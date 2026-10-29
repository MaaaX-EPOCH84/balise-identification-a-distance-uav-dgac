// ************************************************************************************************
// * BALISE D'IDENTIFICATION A DISTANCE POUR UAV (DGAC)                                           *
// ************************************************************************************************
// * Author:     Maxime MARCONATO (maxime@maaax.com, aka MaaaX^^, aka EPOCH84)
// * Repository: https://github.com/EPOCH84/Balise_Identification_A_Distance_UAV_DGAC
// * Based on:
// * - https://github.com/fanfanlatulipe26/BaliseDGAC_GPS_Logger
// * - https://github.com/michelep/ESP32_BeaconSniffer
// * - https://www.legifrance.gouv.fr/eli/arrete/2019/12/27/ECOI1934044A/jo/texte
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
// Bench receiver: listens to one WiFi channel in promiscuous mode and decodes the DGAC
// vendor-specific element (OUI 6A:5C:35, type 1) of every beacon frame it hears.
//
// The decoding follows the order's annex, not the beacon's own code, so that a mistake in the
// encoder shows up instead of being mirrored. Types 12 to 200 are reserved by the order and are
// reported as such.
// ================================================================================================

#include <Arduino.h>
#include <WiFi.h>
#include "esp_wifi.h"

// ================================================================================================
// CONFIGURATION
// ================================================================================================

static const uint8_t CHANNEL = 6;
static const uint32_t SUMMARY_PERIOD_MS = 10000;

// ================================================================================================
// CAPTURE STATE (written by the WiFi task, read by loop())
// ================================================================================================

static volatile bool g_ready = false;
static uint8_t g_ie[256];
static uint8_t g_ie_len = 0;
static uint8_t g_src[6];
static int8_t g_rssi = 0;
static volatile uint32_t g_beacons = 0;
static volatile uint32_t g_dgac = 0;

// ================================================================================================
// CAPTURE
// ================================================================================================

static void sniffer(void* buf, wifi_promiscuous_pkt_type_t type) {
    if (type != WIFI_PKT_MGMT) return;
    const wifi_promiscuous_pkt_t* pkt = static_cast<const wifi_promiscuous_pkt_t*>(buf);
    const uint8_t* p = pkt->payload;
    int len = pkt->rx_ctrl.sig_len - 4;  // without FCS
    if (len < 36 || p[0] != 0x80) return;  // beacon frames only
    g_beacons++;

    // Header 24 + timestamp 8 + interval 2 + capabilities 2
    int i = 36;
    while (i + 2 <= len) {
        uint8_t tag = p[i];
        uint8_t l = p[i + 1];
        if (i + 2 + l > len) break;
        if (tag == 0xDD && l >= 4 && p[i + 2] == 0x6A && p[i + 3] == 0x5C &&
            p[i + 4] == 0x35 && p[i + 5] == 0x01) {
            g_dgac++;
            // Keep the first frame until loop() has printed it
            if (!g_ready) {
                memcpy(g_ie, p + i + 6, l - 4);
                g_ie_len = l - 4;
                memcpy(g_src, p + 10, 6);
                g_rssi = pkt->rx_ctrl.rssi;
                g_ready = true;
            }
            return;
        }
        i += 2 + l;
    }
}

// ================================================================================================
// DECODING
// ================================================================================================

// Big-endian, two's complement on fewer than 4 bytes
static int32_t readSigned(const uint8_t* v, uint8_t n) {
    uint32_t u = 0;
    for (uint8_t k = 0; k < n; k++) u = (u << 8) | v[k];
    if (n > 0 && n < 4 && (u & (1UL << (8 * n - 1)))) u |= ~0UL << (8 * n);
    return static_cast<int32_t>(u);
}

static void decode() {
    Serial.printf("\n=== DGAC frame from %02X:%02X:%02X:%02X:%02X:%02X  RSSI %d dBm  "
                  "(beacons %u, DGAC frames %u)\n",
                  g_src[0], g_src[1], g_src[2], g_src[3], g_src[4], g_src[5],
                  g_rssi, g_beacons, g_dgac);
    Serial.print("Raw:");
    for (uint8_t k = 0; k < g_ie_len; k++) Serial.printf(" %02X", g_ie[k]);
    Serial.println();

    uint8_t i = 0;
    while (i + 2 <= g_ie_len) {
        uint8_t t = g_ie[i];
        uint8_t l = g_ie[i + 1];
        const uint8_t* v = g_ie + i + 2;
        if (i + 2 + l > g_ie_len) {
            Serial.println("  truncated TLV");
            break;
        }
        int32_t s = readSigned(v, l);
        // Coordinates are sent in degrees * 1e5 (annex: fixed precision of 5 decimals)
        switch (t) {
            case 1:  Serial.printf("  01 protocol version : %d\n", s); break;
            case 2:  Serial.printf("  02 FR identifier    : %.*s (%u chars)\n", l, reinterpret_cast<const char*>(v), l); break;
            case 3:  Serial.printf("  03 ANSI identifier  : %.*s\n", l, reinterpret_cast<const char*>(v)); break;
            case 4:  Serial.printf("  04 latitude         : %d -> %.5f deg\n", s, s * 1e-5); break;
            case 5:  Serial.printf("  05 longitude        : %d -> %.5f deg\n", s, s * 1e-5); break;
            case 6:  Serial.printf("  06 altitude         : %d m\n", s); break;
            case 7:  Serial.printf("  07 height           : %d m\n", s); break;
            case 8:  Serial.printf("  08 home latitude    : %d -> %.5f deg\n", s, s * 1e-5); break;
            case 9:  Serial.printf("  09 home longitude   : %d -> %.5f deg\n", s, s * 1e-5); break;
            case 10: Serial.printf("  0A ground speed     : %d m/s\n", s); break;
            case 11: Serial.printf("  0B true heading     : %d deg\n", s); break;
            default:
                if (t >= 12 && t <= 200) {
                    Serial.printf("  %02X RESERVED by the order (%u bytes, value %d)\n", t, l, s);
                } else {
                    Serial.printf("  %02X unknown (%u bytes)\n", t, l);
                }
                break;
        }
        i += 2 + l;
    }
}

// ================================================================================================
// SETUP / LOOP
// ================================================================================================

void setup() {
    Serial.begin(115200);
    delay(1500);

    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    wifi_promiscuous_filter_t filter = {.filter_mask = WIFI_PROMIS_FILTER_MASK_MGMT};
    esp_wifi_set_promiscuous_filter(&filter);
    esp_wifi_set_promiscuous_rx_cb(&sniffer);
    esp_wifi_set_promiscuous(true);
    esp_wifi_set_channel(CHANNEL, WIFI_SECOND_CHAN_NONE);

    Serial.printf("DGAC receiver listening on channel %u\n", CHANNEL);
}

void loop() {
    static uint32_t last_summary = 0;

    if (g_ready) {
        decode();
        g_ready = false;
    }

    if (millis() - last_summary > SUMMARY_PERIOD_MS) {
        last_summary = millis();
        Serial.printf("[%lus] beacons %u, DGAC frames %u\n", millis() / 1000, g_beacons, g_dgac);
    }

    delay(10);
}
