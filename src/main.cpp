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
// See the GNU General License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
// ================================================================================================

// ================================================================================================
// INCLUDES
// ================================================================================================

#include "core/biadud_core.h"
#include "core/biadud_platform.h"
#include "beacon/beacon_manager.h"
#include "gps/gps_manager.h"
#include "storage/eeprom_manager.h"
#include "hardware/led_manager.h"
#include "hardware/battery_monitor.h"
#include "network/wifi_manager.h"
#include "network/web_server.h"
#include "network/status_page.h"
#include "utils/debug_utils.h"
#include "utils/string_utils.h"
#ifdef TELEMETRIE_BATTERIE
#include "beacon/bench_telemetry.h"
#endif

// ================================================================================================
// DEFINES
// ================================================================================================

// GPS Configuration
#define GPS_BAUD_RATE     9600
#define GPS_REFRESH_RATE  1    // 1 Hz

// WiFi Configuration
#define WIFI_AP_CHANNEL   6    // canal imposé par l'arrêté du 27/12/2019 (art. 2, I, 9°)

// Timing Configuration
#define GPS_UPDATE_INTERVAL_MS   100

// Home position: wait for a settled fix, so the home altitude is not taken from the first
// readings of a fix (a height of -37 m was seen at rest). If the sky never gets that good, the
// beacon must still transmit, so the home is set anyway after the timeout.
#define HOME_HDOP_MAX            2.0
#define HOME_STABLE_MS           10000
#define HOME_TIMEOUT_MS          60000

// Fault indication: the GPS link is dead when no valid sentence was decoded for this long, and
// the radio is considered lost after this many refused frames in a row (about 9 s at the 3 s
// period). Emission is never interrupted by either: the last position keeps being sent.
#define GPS_SILENCE_MS           5000
#define WIFI_TX_FAIL_LIMIT       3
#define LED_TEST_MS              1000

// ================================================================================================
// GLOBAL STATE
// ================================================================================================

// Timing state
static uint32_t _last_gps_update_ms = 0;

// GPS diagnostics — raw bytes pulled from the GPS UART by the main loop
static uint32_t _gps_bytes_read = 0;

// System state
static bool _system_initialized = false;
static bool _wifi_ready = false;
static bool _gps_ready = false;

// Status page. The server runs in a task of its own: in the emission loop, a request taking
// more than 200 ms when the frame was due pushed the gap past 3 s (5 times in 10 min on
// 11 November 2026). The loop builds the values, the server task only sends a copy of them,
// so no manager is ever read from two tasks.
static volatile uint32_t _web_max_us = 0;      // Longest handleClient() call since boot
static volatile uint32_t _web_slow_count = 0;  // Calls above WEB_SLOW_MS
static volatile uint32_t _web_status_count = 0; // /status.json answers: proves the page still polls
#define WEB_SLOW_MS              100
#define WEB_TASK_STACK           6144
#define WEB_TASK_PRIORITY        1     // Same as loopTask: time-sliced, never ahead of it
#define WEB_TASK_PERIOD_MS       5
#define STATUS_JSON_SIZE         640
#define STATUS_REFRESH_MS        500

static SemaphoreHandle_t _status_mutex = nullptr;
static char _status_body[STATUS_JSON_SIZE];    // JSON members after "age" and the closing brace
static volatile uint32_t _status_last_send_ms = 0;
static volatile bool _status_has_frame = false;

static void handleStatusPage();
static void handleStatusJson();
static void refreshStatusJson();
static void webTask(void* parameters);

// Link supervision: last time the decoded sentence count moved
static uint32_t _gps_sentences_seen = 0;
static uint32_t _gps_last_sentence_ms = 0;

// Last states handed to the LEDs, so setState() is only called on a change
static bool _led_states_known = false;
static LedState _green_state = LedState::LED_OFF;
static LedState _yellow_state = LedState::LED_OFF;

// Home position settling: since when the fix is valid, and since when its HDOP is good
static bool _fix_seen = false;
static uint32_t _fix_since_ms = 0;
static bool _hdop_good = false;
static uint32_t _hdop_good_since_ms = 0;

// ================================================================================================
// SETUP
// ================================================================================================

void setup() {
    // Initialize debug output
    DebugUtils::begin(DEBUG_SERIAL_BAUD);
    delay(1000);
    
    DEBUG_INFO("==============================================");
    DEBUG_INFO("BIADUD - Balise Identification UAV DGAC");
    DEBUG_INFO("Version: " + String(BIADUD_VERSION_STRING));
    DEBUG_INFO("==============================================");
    
    // -------------------------------------------------------------------------------------------
    // Initialize EEPROM and load preferences
    // -------------------------------------------------------------------------------------------
    DEBUG_INFO("Initializing EEPROM...");
    eepromManager.begin(EEPROM_SIZE);
    
    // Check for factory reset pin
    bool factory_reset = eepromManager.checkFactoryReset();
    if (factory_reset) {
        DEBUG_INFO("Factory reset requested via pin!");
    }
    
    // Load preferences
    bool preferences_loaded = eepromManager.load();
    if (!preferences_loaded || !eepromManager.isValid() || factory_reset) {
        DEBUG_WARN("Preferences not loaded or invalid, using defaults");
        eepromManager.resetToFactory();
        
        // Generate default UAV ID from MAC address
        String default_uav_id = EepromManager::generateUavIdFromMac();
        String default_ssid = EepromManager::generateSsidFromMac();
        
        Preferences& prefs = eepromManager.get();
        strncpy(prefs.uav_id, default_uav_id.c_str(), UAV_ID_LENGTH);
        strncpy(prefs.ssid, default_ssid.c_str(), sizeof(prefs.ssid) - 1);
        
        eepromManager.save();
    }
    
    // Display preferences
    DEBUG_INFO("Current Preferences:");
    eepromManager.print();
    
    // -------------------------------------------------------------------------------------------
    // Initialize LED
    // -------------------------------------------------------------------------------------------
    DEBUG_INFO("Initializing LED...");
    // This board wires the LED active low: anode to 3V3, cathode to the pin.
    ledManager.begin(PIN_LED, false);
    // Warning LED, same wiring
    ledWarning.begin(PIN_LED_WARNING, false);

    // Lamp test: both LEDs on for a second, which also shows a LED with a bad contact. The red
    // power LED is wired to the rail and needs no test.
    ledManager.setState(LedState::LED_ON);
    ledWarning.setState(LedState::LED_ON);
    delay(LED_TEST_MS);
    // Then initialization: the yellow LED says so until the GPS is heard. It cannot blink
    // during the blocking GPS and WiFi setup below, only once the loop runs.
    ledManager.setState(LedState::LED_OFF);
    ledWarning.setState(LedState::LED_BLINK_SLOW);
    
    // -------------------------------------------------------------------------------------------
    // Initialize battery measurement
    // -------------------------------------------------------------------------------------------
    batteryMonitor.begin(PIN_BATTERY);
#ifdef TELEMETRIE_BATTERIE
    benchTelemetry.begin();
    DEBUG_INFO("Bench telemetry ON (bench build, never fly it): boot " +
               String(benchTelemetry.getBootCount()));
#endif
    
    // -------------------------------------------------------------------------------------------
    // Initialize GPS
    // -------------------------------------------------------------------------------------------
    DEBUG_INFO("Initializing GPS...");
    
    // Initialize serial GPS port
    #if HAS_HARDWARE_SERIAL
        serialGPS.begin(GPS_BAUD_RATE, SERIAL_8N1, PIN_GPS_RX, PIN_GPS_TX);
    #else
        serialGPS.begin(GPS_BAUD_RATE);
    #endif
    
    // Initialize GPS manager
    gpsManager.initialize(GPS_BAUD_RATE, GPS_REFRESH_RATE);
    _gps_last_sentence_ms = millis();   // Silence is measured from here
    DEBUG_INFO("GPS navigation settings (UBX-CFG-NAV5): " + String(gpsManager.getNavigationStatus()));
    
    // -------------------------------------------------------------------------------------------
    // Initialize WiFi: visible open AP, carrying the DGAC frames
    // -------------------------------------------------------------------------------------------
    // NOTE: DGAC beacon frames are sent via esp_wifi_80211_tx() on the AP interface
    // (see biadud_platform.cpp:sendBeaconFrame), so WiFi must stay active at all times.
    // The AP is visible and open so that a phone can join it; the password, when set,
    // protects the configuration pages, not the connection.
    // TODO Phase 2: config UI (web server + captive portal DNS) — code exists in
    // network/web_server.h and network/dns_server.h, just not wired up yet.
    DEBUG_INFO("Initializing WiFi (beacon mode)...");
    wifiManager.begin();

    // Get preferences
    Preferences& prefs = eepromManager.get();

    String ssid = String(prefs.ssid[0] ? prefs.ssid : WIFI_SSID_PREFIX);

    // AP address from the preferences. The default, 8.8.4.4 (Android's secondary DNS), comes
    // from the reference sketch, which uses it for its captive portal.
    IPAddress ap_ip, ap_gateway, ap_subnet;
    if (StringUtils::stringToIPAddress(prefs.ip_local, ap_ip) &&
        StringUtils::stringToIPAddress(prefs.ip_gateway, ap_gateway) &&
        StringUtils::stringToIPAddress(prefs.ip_subnet, ap_subnet)) {
        wifiManager.setStaticIP(ap_ip, ap_gateway, ap_subnet);
    } else {
        DEBUG_WARN("Invalid AP address in preferences, keeping the system default");
    }

    _wifi_ready = wifiManager.startBeaconMode(ssid, WIFI_AP_CHANNEL, false);
    if (_wifi_ready) {
        DEBUG_INFO("WiFi beacon mode started: " + ssid + " at " + wifiManager.getLocalIPString());
    } else {
        DEBUG_ERROR("Failed to start WiFi beacon mode!");
    }

    // -------------------------------------------------------------------------------------------
    // Initialize Beacon Manager
    // -------------------------------------------------------------------------------------------
    DEBUG_INFO("Initializing beacon manager...");
    beaconManager.initialize(prefs.uav_id, ssid.c_str(), WIFI_AP_CHANNEL);

    // -------------------------------------------------------------------------------------------
    // Status page, read-only, on the AP
    // -------------------------------------------------------------------------------------------
    if (_wifi_ready) {
        _status_mutex = xSemaphoreCreateMutex();
        if (_status_mutex) {
            refreshStatusJson();
        }
        webServer.begin(80);
        webServer.onRoot(handleStatusPage);
        webServer.onStatus(handleStatusJson);
        webServer.start();
        if (_status_mutex &&
            xTaskCreate(webTask, "web", WEB_TASK_STACK, nullptr, WEB_TASK_PRIORITY, nullptr) == pdPASS) {
            DEBUG_INFO("Status page on http://" + wifiManager.getLocalIPString() + "/");
        } else {
            webServer.stop();
            DEBUG_ERROR("Status page not started: no task");
        }
    }
    
    _system_initialized = true;
    DEBUG_INFO("==============================================");
    DEBUG_INFO("System initialized successfully!");
    DEBUG_INFO("==============================================");
}

// ================================================================================================
// LOOP
// ================================================================================================

// ================================================================================================
// LED INDICATION
// ================================================================================================

/**
 * Track the GPS link: it is mute when no valid sentence was decoded for GPS_SILENCE_MS.
 * "Mute" means no communication with the module, not the absence of a fix.
 *
 * @return true while the GPS is mute
 */
static bool isGpsMute() {
    uint32_t sentences = gpsManager.getSentenceCount();
    if (sentences != _gps_sentences_seen) {
        _gps_sentences_seen = sentences;
        _gps_last_sentence_ms = millis();
    }
    return (millis() - _gps_last_sentence_ms) >= GPS_SILENCE_MS;
}

// ================================================================================================
// STATUS PAGE
// ================================================================================================

/**
 * Append formatted text to a fixed buffer. Once full, n stays at or above size, which the
 * caller checks: a truncated answer is never sent.
 */
static void appendf(char* buffer, size_t size, size_t* n, const char* format, ...)
    __attribute__((format(printf, 4, 5)));

static void appendf(char* buffer, size_t size, size_t* n, const char* format, ...) {
    if (*n >= size) {
        return;
    }
    va_list args;
    va_start(args, format);
    const int written = vsnprintf(buffer + *n, size - *n, format, args);
    va_end(args);
    if (written < 0) {
        *n = size;
    } else {
        *n += static_cast<size_t>(written);
    }
}

static void handleStatusPage() {
    webServer.send_P(200, PSTR("text/html; charset=utf-8"), STATUS_PAGE_HTML);
}

/**
 * Build the values polled by the status page, in the loop task. The frame fields are those of
 * the last frame sent, in the units a receiver decodes (coordinates in 1e-5 degree), so that
 * the page matches it. The age of that frame is added by the server task when it answers.
 */
static void refreshStatusJson() {
    static char json[STATUS_JSON_SIZE];
    size_t n = 0;

    const char* state = "wait";
    if (beaconManager.isReady()) {
        state = beaconManager.isPositionFrozen() ? "frozen" : "live";
    }
    const uint32_t frames = beaconManager.getFrameCount();

    appendf(json, sizeof(json), &n, "\"up\":%lu,\"state\":\"%s\",\"frames\":%lu,\"maxgap\":%lu,\"over\":%lu",
        static_cast<unsigned long>(millis() / 1000), state,
        static_cast<unsigned long>(frames),
        static_cast<unsigned long>(beaconManager.getMaxIntervalMs()),
        static_cast<unsigned long>(beaconManager.getOverIntervalCount()));

    DroneIdFrValues frame;
    if (beaconManager.getLastSentFrame(frame)) {
        // The identifier comes from the preferences: keep it to characters that need no
        // escaping in JSON
        for (char* c = frame.id; *c; c++) {
            if (!isalnum(static_cast<unsigned char>(*c))) {
                *c = '?';
            }
        }
        appendf(json, sizeof(json), &n, ",\"frame\":{\"id\":\"%s\",\"lat\":%ld,\"lon\":%ld,\"alt\":%d,\"h\":%d,"
            "\"hlat\":%ld,\"hlon\":%ld,\"spd\":%u,\"hdg\":%u}",
            frame.id, static_cast<long>(frame.latitude), static_cast<long>(frame.longitude),
            frame.altitude, frame.height,
            static_cast<long>(frame.home_latitude), static_cast<long>(frame.home_longitude),
            static_cast<unsigned>(frame.ground_speed), static_cast<unsigned>(frame.heading));
    } else {
        appendf(json, sizeof(json), &n, ",\"frame\":null");
    }

    const bool gps_mute = isGpsMute();
    const GpsData& gps = gpsManager.getData();
    appendf(json, sizeof(json), &n, ",\"gps\":{\"fix\":%s,\"sats\":%u,\"hdop\":%.2f,\"mute\":%s}",
        (gps.has_fix && !gps_mute) ? "true" : "false",
        static_cast<unsigned>(gps.satellites),
        isfinite(gps.hdop) ? static_cast<double>(gps.hdop) : 99.99,   // nan is not JSON
        gps_mute ? "true" : "false");

    if (batteryMonitor.isAvailable()) {
        appendf(json, sizeof(json), &n, ",\"bat\":{\"mv\":%u,\"low\":%s}",
            static_cast<unsigned>(batteryMonitor.getVoltageMv()),
            batteryMonitor.isLow() ? "true" : "false");
    } else {
        appendf(json, sizeof(json), &n, ",\"bat\":null");
    }

    if (beaconManager.hasMaxima()) {
        appendf(json, sizeof(json), &n, ",\"max\":{\"spd\":%.1f,\"h\":%d}",
            beaconManager.getMaxGroundSpeed(), beaconManager.getMaxHeight());
    } else {
        appendf(json, sizeof(json), &n, ",\"max\":null");
    }
    appendf(json, sizeof(json), &n, "}");

    if (n >= sizeof(json)) {
        snprintf(json, sizeof(json), "\"error\":\"status too long\"");
    }

    if (xSemaphoreTake(_status_mutex, pdMS_TO_TICKS(10)) == pdTRUE) {
        memcpy(_status_body, json, sizeof(_status_body));
        _status_last_send_ms = beaconManager.getLastSendTime();
        _status_has_frame = frames > 0;
        xSemaphoreGive(_status_mutex);
    }
}

/**
 * Answer /status.json, in the server task: a copy of what the loop built, plus the age of the
 * last frame computed now, so that it stays right between two refreshes.
 */
static void handleStatusJson() {
    static char json[STATUS_JSON_SIZE + 32];
    const uint32_t age_ms = _status_has_frame ? millis() - _status_last_send_ms : 0;
    const int head = snprintf(json, sizeof(json), "{\"age\":%lu,", static_cast<unsigned long>(age_ms));
    if (head < 0 || xSemaphoreTake(_status_mutex, pdMS_TO_TICKS(100)) != pdTRUE) {
        webServer.sendServerError("status unavailable");
        return;
    }
    strlcpy(json + head, _status_body, sizeof(json) - head);
    xSemaphoreGive(_status_mutex);
    _web_status_count = _web_status_count + 1;
    webServer.sendBuffer(200, "application/json", json);
}

/**
 * Server task. Same priority as loopTask, so the scheduler shares the processor tick by tick
 * when both run, and gives it back to the loop as soon as the server waits on the network.
 */
static void webTask(void* parameters) {
    (void)parameters;
    for (;;) {
        const uint32_t start_us = micros();
        webServer.handleClient();
        const uint32_t elapsed_us = micros() - start_us;
        if (elapsed_us > _web_max_us) {
            _web_max_us = elapsed_us;
        }
        if (elapsed_us > WEB_SLOW_MS * 1000UL) {
            _web_slow_count = _web_slow_count + 1;
        }
        vTaskDelay(pdMS_TO_TICKS(WEB_TASK_PERIOD_MS));
    }
}

/**
 * Drive the two LEDs from the system state, every loop iteration. They are independent.
 *
 * Green says what is sent: off when nothing is, fast blink while a fix settles before the home
 * position is set, slow blink while the last position is sent again for want of a live fix,
 * steady when a live position is sent.
 *
 * Yellow says what is wrong: off when nothing is, slow blink until the GPS is first heard,
 * fast blink while searching for a fix, steady when the GPS is mute or the radio lost. A low
 * battery will get its own pattern, a double flash.
 */
static void updateIndicators() {
    const bool gps_mute = isGpsMute();
    const bool gps_heard = _gps_sentences_seen > 0;
    const bool wifi_lost = !_wifi_ready ||
                           beaconManager.getConsecutiveTxErrors() >= WIFI_TX_FAIL_LIMIT;
    const bool has_fix = !gps_mute && gpsManager.getData().has_fix;

    LedState green;
    if (wifi_lost) {
        green = LedState::LED_OFF;
    } else if (beaconManager.isReady()) {
        green = beaconManager.isPositionFrozen() ? LedState::LED_BLINK_SLOW : LedState::LED_ON;
    } else if (has_fix && gpsManager.isReady()) {
        green = LedState::LED_BLINK_FAST;
    } else {
        green = LedState::LED_OFF;
    }

    // Low battery outranks every other state: it means landing within about 10 minutes
    LedState yellow;
    if (batteryMonitor.isLow()) {
        yellow = LedState::LED_BATTERY_LOW;
    } else if (gps_mute || wifi_lost) {
        yellow = LedState::LED_ON;
    } else if (!gps_heard) {
        yellow = LedState::LED_BLINK_SLOW;
    } else if (!has_fix) {
        yellow = LedState::LED_BLINK_FAST;
    } else {
        yellow = LedState::LED_OFF;
    }

    // setState() restarts a blink pattern, so it is only called when the state changes
    if (!_led_states_known || green != _green_state) {
        ledManager.setState(green);
        _green_state = green;
    }
    if (!_led_states_known || yellow != _yellow_state) {
        ledWarning.setState(yellow);
        _yellow_state = yellow;
    }
    _led_states_known = true;
}

void loop() {
    // LedManager keeps its own timing, so it is updated every iteration. Gating this
    // call capped every pattern at the caller's rate: fast blink became slow blink,
    // and the pulse could not render at all.
    ledManager.update();
    ledWarning.update();
    updateIndicators();
    batteryMonitor.update();
    
    // -------------------------------------------------------------------------------------------
    // Update GPS data
    // -------------------------------------------------------------------------------------------
    if (hasElapsed(_last_gps_update_ms, GPS_UPDATE_INTERVAL_MS)) {
        _last_gps_update_ms = millis();
        
        // Process GPS data from serial port
        while (serialGPS.available()) {
            char c = serialGPS.read();
            _gps_bytes_read++;
            gpsManager.processChar(c);
        }
        
        // Update GPS manager
        gpsManager.update();
        
        // Check if GPS is now ready (isReady() is the quality criteria: fix,
        // 4+ satellites, HDOP below 5.0 and a valid altitude), then wait for the fix to settle
        // before taking the home position
        if (!_gps_ready) {
            GpsData gps_data = gpsManager.getData();
            uint32_t now_ms = millis();

            if (gpsManager.isReady() && gps_data.has_fix) {
                if (!_fix_seen) {
                    _fix_seen = true;
                    _fix_since_ms = now_ms;
                    DEBUG_INFO("GPS has valid fix, waiting for it to settle...");
                }
                if (gps_data.hdop < HOME_HDOP_MAX) {
                    if (!_hdop_good) {
                        _hdop_good = true;
                        _hdop_good_since_ms = now_ms;
                    }
                } else {
                    _hdop_good = false;
                }

                bool settled = _hdop_good && (now_ms - _hdop_good_since_ms) >= HOME_STABLE_MS;
                bool timed_out = (now_ms - _fix_since_ms) >= HOME_TIMEOUT_MS;
                if (settled || timed_out) {
                    _gps_ready = true;
                    if (settled) {
                        DEBUG_INFO("GPS fix settled, home position set");
                    } else {
                        DEBUG_WARN("GPS fix never settled (HDOP " + String(gps_data.hdop, 2) +
                                   "), home position set after timeout");
                    }
                    beaconManager.setHomePosition(gps_data.position.latitude,
                                                   gps_data.position.longitude,
                                                   gps_data.altitude);
                }
            } else {
                // Fix lost before the home was set: the HDOP run starts over, but the timeout
                // keeps counting from the first fix, so a fix that flickers cannot keep the
                // beacon silent for good
                _hdop_good = false;
            }
        }
        
        // Update beacon manager with latest GPS data. A mute GPS leaves its last fix flag set,
        // so it is cleared here: the beacon then sends the last position, frozen.
        if (_gps_ready) {
            GpsData beacon_data = gpsManager.getData();
            if (isGpsMute()) {
                beacon_data.has_fix = false;
            }
            beaconManager.updateGpsData(beacon_data);
        }
    }
    
    // -------------------------------------------------------------------------------------------
    // Send beacon frames
    // -------------------------------------------------------------------------------------------
    // sendBeaconFrame() applies the DGAC conditions itself through shouldSend():
    // initialization, home position, a known position and the 3s / 30m period. No extra
    // guard here, so the very first frame can go out.
    if (_gps_ready) {
        beaconManager.sendBeaconFrame();
    }

    // -------------------------------------------------------------------------------------------
    // Values for the status page, served by its own task
    // -------------------------------------------------------------------------------------------
    static uint32_t _last_status_json_ms = 0;
    if (_status_mutex && hasElapsed(_last_status_json_ms, STATUS_REFRESH_MS)) {
        _last_status_json_ms = millis();
        refreshStatusJson();
    }

#ifdef TELEMETRIE_BATTERIE
    // Bench build only: battery figures in a frame of their own, from boot on, fix or not
    if (_wifi_ready) {
        benchTelemetry.update(gpsManager.getData().has_fix && !isGpsMute());
    }
#endif
    
    // -------------------------------------------------------------------------------------------
    // Periodic status update (every 5 seconds)
    // -------------------------------------------------------------------------------------------
    static uint32_t _last_status_update_ms = 0;
    if (hasElapsed(_last_status_update_ms, 5000)) {
        _last_status_update_ms = millis();
        
        // Print status to serial
        DEBUG_INFO("--- Status Update ---");
        DEBUG_INFO("Uptime: " + DebugUtils::getUptime());
        DEBUG_INFO("Free Heap: " + String(DebugUtils::getFreeHeap()) + " bytes");
        if (batteryMonitor.isAvailable()) {
            DEBUG_INFO("Battery: cell=" + String(batteryMonitor.getVoltageMv()) + " mV" +
                       " pin=" + String(batteryMonitor.getPinVoltageMv()) + " mV" +
                       " low=" + String(batteryMonitor.isLow() ? "YES" : "NO"));
        }
        DEBUG_INFO(beaconManager.getStateString());
        DEBUG_INFO(beaconManager.getTxStatsString());
#ifdef TELEMETRIE_BATTERIE
        DEBUG_INFO("Bench telemetry: sent=" + String(benchTelemetry.getSentCount()) +
                   " txerr=" + String(benchTelemetry.getErrorCount()));
#endif

        // GPS link diagnostics: separates a dead RX line (bytes stuck at 0) from
        // sentences that arrive but get rejected, and from a receiver still searching.
        const GpsData& gps_diag = gpsManager.getData();
        DEBUG_INFO("GPS link: bytes=" + String(_gps_bytes_read) +
                   " sentences=" + String(gpsManager.getSentenceCount()) +
                   " badcrc=" + String(gpsManager.getFailedChecksumCount()) +
                   " last=" + gpsManager.getLastSentenceType());
        DEBUG_INFO("GPS data: fix=" + String(gps_diag.has_fix ? "YES" : "NO") +
                   " sats=" + String(gps_diag.satellites) +
                   " hdop=" + String(gps_diag.hdop, 2) +
                   " alt=" + String(gps_diag.altitude, 1) +
                   " lat=" + String(gps_diag.position.latitude, 6) +
                   " lon=" + String(gps_diag.position.longitude, 6) +
                   " speed=" + String(gps_diag.speed, 2) +
                   " course=" + String(gps_diag.heading, 1));

        if (_wifi_ready) {
            DEBUG_INFO(wifiManager.getNetworkInfo());
        }
        if (_status_mutex) {
            DEBUG_INFO("Web: max=" + String(_web_max_us / 1000.0, 1) + " ms" +
                       " slow=" + String(_web_slow_count) +
                       " status=" + String(_web_status_count));
            // Written by this task only: read without the lock. Compared with the bench
            // receiver to check that the page shows what was sent.
            DEBUG_INFO(String("Status JSON: {") + _status_body);
        }
    }
}
