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

#include "web_server.h"
#include "../beacon/beacon_manager.h"
#include "../gps/gps_manager.h"
#include "../storage/eeprom_manager.h"

// Platform-specific includes for WebServer
// Note: These are included via platformio lib_deps
// For ESP32: WebServer library provides WEBSERVER_CLASS
// For ESP8266: ESP8266WebServer library provides ESP8266WebServer
// No need to include explicitly here as biadud_platform.h handles it

// Global instance
WebServerManager webServer;

// Helper function to get server reference (defined in .cpp to avoid header issues)
#if defined(PLATFORM_ESP32C3) || defined(PLATFORM_ESP32S3) || defined(PLATFORM_ESP32)
    inline WEBSERVER_CLASS& getServer(void* ptr) { return *static_cast<WEBSERVER_CLASS*>(ptr); }
    inline const WEBSERVER_CLASS& getServerConst(const void* ptr) { return *static_cast<const WEBSERVER_CLASS*>(ptr); }
#elif defined(PLATFORM_ESP8266)
    inline ESP8266WebServer& getServer(void* ptr) { return *static_cast<ESP8266WebServer*>(ptr); }
    inline const ESP8266WebServer& getServerConst(const void* ptr) { return *static_cast<const ESP8266WebServer*>(ptr); }
#endif

// ================================================================================================
// CONSTRUCTOR / DESTRUCTOR
// ================================================================================================

WebServerManager::WebServerManager() {
    _running = false;
    _port = 80;
    _server_ptr = nullptr;
}

WebServerManager::~WebServerManager() {
    stop();
}

// ================================================================================================
// INITIALIZATION
// ================================================================================================

void WebServerManager::begin(uint16_t port) {
    _port = port;
}

void WebServerManager::start() {
    if (_running) {
        return;
    }
    
    // Create server instance based on platform
    #if defined(PLATFORM_ESP32C3) || defined(PLATFORM_ESP32S3) || defined(PLATFORM_ESP32)
        _server_ptr = new WEBSERVER_CLASS(_port);
    #elif defined(PLATFORM_ESP8266)
        _server_ptr = new ESP8266WebServer(_port);
    #endif
    
    // Setup default handlers
    getServer(_server_ptr).on("/", [this]() { handleRoot(); });
    getServer(_server_ptr).on("/favicon.ico", [this]() { handleFaviconRequest(); });
    getServer(_server_ptr).onNotFound([this]() { handleNotFound(); });
    
    // Start server
    getServer(_server_ptr).begin();
    _running = true;
}

void WebServerManager::stop() {
    if (_running && _server_ptr) {
        getServer(_server_ptr).stop();
        #if defined(PLATFORM_ESP32C3) || defined(PLATFORM_ESP32S3) || defined(PLATFORM_ESP32)
            delete static_cast<WEBSERVER_CLASS*>(_server_ptr);
        #elif defined(PLATFORM_ESP8266)
            delete static_cast<ESP8266WebServer*>(_server_ptr);
        #endif
        _server_ptr = nullptr;
        _running = false;
    }
}

// ================================================================================================
// REQUEST HANDLING
// ================================================================================================

void WebServerManager::handleClient() {
    if (_running) {
        getServer(_server_ptr).handleClient();
    }
}

// ================================================================================================
// ROUTE HANDLERS
// ================================================================================================

void WebServerManager::onRoot(void (*handler)()) {
    _root_handler = handler;
}

void WebServerManager::onFavicon(void (*handler)()) {
    _favicon_handler = handler;
}

void WebServerManager::onNotFound(void (*handler)()) {
    _not_found_handler = handler;
}

// ================================================================================================
// STATUS CHECKS
// ================================================================================================

bool WebServerManager::hasClients() const {
    return _has_current_client;
}

uint8_t WebServerManager::getClientCount() const {
    // Note: This is a simplified implementation
    // Actual client count would require more complex tracking
    return _has_current_client ? 1 : 0;
}

// ================================================================================================
// RESPONSE HELPERS
// ================================================================================================

void WebServerManager::send(int code, const String& content_type, const String& content) {
    getServer(_server_ptr).send(code, content_type, content);
}

void WebServerManager::send_P(int code, PGM_P content_type, PGM_P content) {
    #if defined(PLATFORM_ESP32C3) || defined(PLATFORM_ESP32S3) || defined(PLATFORM_ESP32)
        getServer(_server_ptr).send_P(code, content_type, content);
    #elif defined(PLATFORM_ESP8266)
        getServer(_server_ptr).send_P(code, content_type, content);
    #endif
}

void WebServerManager::sendJson(int code, const String& json) {
    getServer(_server_ptr).send(code, "application/json", json);
}

void WebServerManager::sendText(int code, const String& text) {
    getServer(_server_ptr).send(code, "text/plain", text);
}

void WebServerManager::sendHtml(int code, const String& html) {
    getServer(_server_ptr).send(code, "text/html", html);
}

void WebServerManager::sendOk(const String& content_type, const String& content) {
    getServer(_server_ptr).send(200, content_type, content);
}

void WebServerManager::sendRedirect(const String& url) {
    getServer(_server_ptr).sendHeader("Location", url);
    getServer(_server_ptr).send(302, "text/plain", "Redirecting...");
}

void WebServerManager::sendNotFound() {
    getServer(_server_ptr).send(404, "text/plain", "404: Not Found");
}

void WebServerManager::sendServerError(const String& message) {
    getServer(_server_ptr).send(500, "text/plain", "500: " + message);
}

// ================================================================================================
// REQUEST INFORMATION
// ================================================================================================

String WebServerManager::getRequestMethod() const {
    // Note: This would require access to the current request
    // For simplicity, we'll return a placeholder
    return "GET";
}

String WebServerManager::getRequestURI() const {
    return getServer(_server_ptr).uri();
}

String WebServerManager::getRequestPath() const {
    return getServer(_server_ptr).uri();
}

bool WebServerManager::isGetRequest() const {
    return getServer(_server_ptr).method() == HTTP_GET;
}

bool WebServerManager::isPostRequest() const {
    return getServer(_server_ptr).method() == HTTP_POST;
}

String WebServerManager::getQueryParam(const String& name) const {
    if (getServer(_server_ptr).hasArg(name)) {
        return getServer(_server_ptr).arg(name);
    }
    return "";
}

String WebServerManager::getFormParam(const String& name) const {
    if (getServer(_server_ptr).hasArg(name)) {
        return getServer(_server_ptr).arg(name);
    }
    return "";
}

// ================================================================================================
// FILE SERVING
// ================================================================================================

bool WebServerManager::serveFile(const String& path) {
    #if defined(PLATFORM_ESP32C3) || defined(PLATFORM_ESP32S3) || defined(PLATFORM_ESP32)
        if (FILESYSTEM.exists(path)) {
            File file = FILESYSTEM.open(path, "r");
            if (file) {
                String contentType = "text/plain";
                if (path.endsWith(".html") || path.endsWith(".htm")) {
                    contentType = "text/html";
                } else if (path.endsWith(".css")) {
                    contentType = "text/css";
                } else if (path.endsWith(".js")) {
                    contentType = "application/javascript";
                } else if (path.endsWith(".ico")) {
                    contentType = "image/x-icon";
                } else if (path.endsWith(".json")) {
                    contentType = "application/json";
                }
                
                getServer(_server_ptr).streamFile(file, contentType);
                file.close();
                return true;
            }
        }
    #elif defined(PLATFORM_ESP8266)
        if (FILESYSTEM.exists(path)) {
            File file = FILESYSTEM.open(path, "r");
            if (file) {
                String contentType = "text/plain";
                if (path.endsWith(".html") || path.endsWith(".htm")) {
                    contentType = "text/html";
                } else if (path.endsWith(".css")) {
                    contentType = "text/css";
                } else if (path.endsWith(".js")) {
                    contentType = "application/javascript";
                } else if (path.endsWith(".ico")) {
                    contentType = "image/x-icon";
                } else if (path.endsWith(".json")) {
                    contentType = "application/json";
                }
                
                getServer(_server_ptr).streamFile(file, contentType);
                file.close();
                return true;
            }
        }
    #endif
    return false;
}

bool WebServerManager::fileExists(const String& path) const {
    #if defined(PLATFORM_ESP32C3) || defined(PLATFORM_ESP32S3) || defined(PLATFORM_ESP32)
        return FILESYSTEM.exists(path);
    #elif defined(PLATFORM_ESP8266)
        return FILESYSTEM.exists(path);
    #else
        return false;
    #endif
}

// ================================================================================================
// STATIC CONTENT
// ================================================================================================

void WebServerManager::serveFavicon() {
    // Check if favicon exists in LittleFS
    if (fileExists("/favicon.ico")) {
        serveFile("/favicon.ico");
        return;
    }
    
    // Fallback: serve empty favicon
    const char empty_favicon[] PROGMEM = {
        0x00, 0x00, 0x01, 0x00, 0x01, 0x00, 0x10, 0x10, 0x00, 0x00, 0x01, 0x00,
        0x08, 0x00, 0x00, 0x00, 0x00, 0x00
    };
    
    getServer(_server_ptr).send_P(200, PSTR("image/x-icon"), reinterpret_cast<const char*>(empty_favicon));
}

// ================================================================================================
// HELPER FUNCTIONS FOR WEB PAGES
// ================================================================================================

String WebServerManager::generateHtmlHeader(const String& title) {
    String header = "<!DOCTYPE html>";
    header += "<html lang=\"fr\">";
    header += "<head>";
    header += "<meta charset=\"UTF-8\">";
    header += "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">";
    header += "<title>" + title + "</title>";
    header += "<style>";
    header += "body { font-family: Arial, sans-serif; margin: 20px; background: #f5f5f5; }";
    header += ".container { max-width: 800px; margin: 0 auto; background: white; padding: 20px; border-radius: 8px; box-shadow: 0 0 10px rgba(0,0,0,0.1); }";
    header += "h1 { color: #333; border-bottom: 2px solid #6A5C35; padding-bottom: 10px; }";
    header += ".status { background: #e8f5e8; padding: 15px; border-radius: 4px; margin: 10px 0; }";
    header += ".error { background: #ffe8e8; padding: 15px; border-radius: 4px; margin: 10px 0; }";
    header += ".info { background: #e8f0ff; padding: 15px; border-radius: 4px; margin: 10px 0; }";
    header += "table { width: 100%; border-collapse: collapse; margin: 10px 0; }";
    header += "th, td { padding: 10px; text-align: left; border-bottom: 1px solid #ddd; }";
    header += "th { background: #6A5C35; color: white; }";
    header += "tr:nth-child(even) { background: #f2f2f2; }";
    header += "a { color: #6A5C35; text-decoration: none; }";
    header += "a:hover { text-decoration: underline; }";
    header += "</style>";
    header += "</head>";
    header += "<body>";
    header += "<div class=\"container\">";
    header += "<h1>" + title + "</h1>";
    
    return header;
}

String WebServerManager::generateHtmlFooter() {
    String footer = "</div>";
    footer += "<div style=\"text-align: center; margin: 20px; color: #888; font-size: 12px;\">";
    footer += "BALISE UAV DGAC - " + String(BIADUD_VERSION_STRING);
    footer += "</div>";
    footer += "</body></html>";
    return footer;
}

String WebServerManager::generateStatusPage(BeaconManager& beacon_manager, 
                                             GpsManager& gps_manager, 
                                             EepromManager& eeprom_manager) {
    String html = generateHtmlHeader("BIADUD - Status");
    
    // System info
    html += "<div class=\"info\">";
    html += "<h2>System Information</h2>";
    html += "<p><strong>Firmware:</strong> " + String(BIADUD_VERSION_STRING) + "</p>";
    html += "<p><strong>UAV ID:</strong> " + String(eeprom_manager.getPreferences().uav_id) + "</p>";
    html += "<p><strong>MAC:</strong> " + wifiManager.getMacAddress() + "</p>";
    html += "</div>";
    
    // Beacon status
    html += "<div class=\"status\">";
    html += "<h2>Beacon Status</h2>";
    html += "<p><strong>State:</strong> " + beacon_manager.getStateString() + "</p>";
    html += "<p><strong>Frames sent:</strong> " + String(beacon_manager.getFrameCount()) + "</p>";
    html += "<p><strong>Home set:</strong> " + String(beacon_manager.hasHomeSet() ? "YES" : "NO") + "</p>";
    html += "<p><strong>GPS Status:</strong> " + beacon_manager.getGpsStatusString() + "</p>";
    html += "</div>";
    
    // GPS data
    html += "<div class=\"info\">";
    html += "<h2>GPS Data</h2>";
    
    GpsData gps_data = gps_manager.getData();
    
    html += "<table>";
    html += "<tr><th>Parameter</th><th>Value</th></tr>";
    html += "<tr><td>Latitude</td><td>" + String(gps_data.position.latitude, 6) + "</td></tr>";
    html += "<tr><td>Longitude</td><td>" + String(gps_data.position.longitude, 6) + "</td></tr>";
    html += "<tr><td>Altitude</td><td>" + String(gps_data.altitude, 2) + " m</td></tr>";
    html += "<tr><td>Speed</td><td>" + String(gps_data.speed, 2) + " m/s</td></tr>";
    html += "<tr><td>Heading</td><td>" + String(gps_data.heading, 1) + " &deg;</td></tr>";
    html += "<tr><td>Satellites</td><td>" + String(gps_data.satellites) + "</td></tr>";
    html += "<tr><td>HDOP</td><td>" + String(gps_data.hdop, 2) + "</td></tr>";
    html += "<tr><td>Has Fix</td><td>" + String(gps_data.has_fix ? "YES" : "NO") + "</td></tr>";
    html += "</table>";
    html += "</div>";
    
    // Network info
    html += "<div class=\"info\">";
    html += "<h2>Network Information</h2>";
    html += "<p>" + wifiManager.getNetworkInfo() + "</p>";
    html += "</div>";
    
    html += "<p><a href=\"/\">Refresh</a> | <a href=\"/config\">Configuration</a></p>";
    
    html += generateHtmlFooter();
    
    return html;
}

String WebServerManager::generateConfigPage(EepromManager& eeprom_manager) {
    String html = generateHtmlHeader("BIADUD - Configuration");
    
    Preferences& prefs = eeprom_manager.get();
    
    html += "<form method='post' action='/save'>";
    html += "<div class=\"info\">";
    html += "<h2>Configuration</h2>";
    
    html += "<table>";
    html += "<tr><th colspan='2'>UAV Settings</th></tr>";
    html += "<tr><td><label for='uav_id'>UAV ID:</label></td>";
    html += "<td><input type='text' id='uav_id' name='uav_id' value='" + String(prefs.uav_id) + "' maxlength='30' required></td></tr>";
    
    html += "<tr><th colspan='2'>WiFi Settings</th></tr>";
    html += "<tr><td><label for='ssid'>SSID:</label></td>";
    html += "<td><input type='text' id='ssid' name='ssid' value='" + String(prefs.ssid) + "' maxlength='32'></td></tr>";
    
    html += "<tr><td><label for='password'>Password:</label></td>";
    html += "<td><input type='password' id='password' name='password' value='" + String(prefs.password) + "' maxlength='64'></td></tr>";
    
    html += "<tr><th colspan='2'>Network Settings</th></tr>";
    html += "<tr><td><label for='ip_local'>Local IP:</label></td>";
    html += "<td><input type='text' id='ip_local' name='ip_local' value='" + String(prefs.ip_local) + "' maxlength='15' placeholder='192.168.1.1'></td></tr>";
    
    html += "<tr><td><label for='ip_gateway'>Gateway:</label></td>";
    html += "<td><input type='text' id='ip_gateway' name='ip_gateway' value='" + String(prefs.ip_gateway) + "' maxlength='15' placeholder='192.168.1.1'></td></tr>";
    
    html += "<tr><td><label for='ip_subnet'>Subnet:</label></td>";
    html += "<td><input type='text' id='ip_subnet' name='ip_subnet' value='" + String(prefs.ip_subnet) + "' maxlength='15' placeholder='255.255.255.0'></td></tr>";
    
    html += "<tr><th colspan='2'>GPS Settings</th></tr>";
    html += "<tr><td><label for='gps_baud'>GPS Baud Rate:</label></td>";
    html += "<td><select id='gps_baud' name='gps_baud'>";
    html += "<option value='9600'" + String(prefs.gps_baud == 9600 ? " selected" : "") + ">9600</option>";
    html += "<option value='19200'" + String(prefs.gps_baud == 19200 ? " selected" : "") + ">19200</option>";
    html += "<option value='38400'" + String(prefs.gps_baud == 38400 ? " selected" : "") + ">38400</option>";
    html += "<option value='57600'" + String(prefs.gps_baud == 57600 ? " selected" : "") + ">57600</option>";
    html += "<option value='115200'" + String(prefs.gps_baud == 115200 ? " selected" : "") + ">115200</option>";
    html += "</select></td></tr>";
    
    html += "<tr><td><label for='gps_rate'>GPS Update Rate (Hz):</label></td>";
    html += "<td><select id='gps_rate' name='gps_rate'>";
    html += "<option value='1'" + String(prefs.gps_rate == 1 ? " selected" : "") + ">1</option>";
    html += "<option value='2'" + String(prefs.gps_rate == 2 ? " selected" : "") + ">2</option>";
    html += "<option value='4'" + String(prefs.gps_rate == 4 ? " selected" : "") + ">4</option>";
    html += "<option value='5'" + String(prefs.gps_rate == 5 ? " selected" : "") + ">5</option>";
    html += "<option value='10'" + String(prefs.gps_rate == 10 ? " selected" : "") + ">10</option>";
    html += "</select></td></tr>";
    
    html += "<tr><th colspan='2'>Options</th></tr>";
    html += "<tr><td><label for='wifi_enabled'>WiFi Enabled:</label></td>";
    html += "<td><input type='checkbox' id='wifi_enabled' name='wifi_enabled'" + String(prefs.wifi_enabled ? " checked" : "") + "></td></tr>";
    
    html += "<tr><td><label for='wifi_timeout'>WiFi Timeout (s):</label></td>";
    html += "<td><input type='number' id='wifi_timeout' name='wifi_timeout' value='" + String(prefs.wifi_timeout) + "' min='5' max='120'></td></tr>";
    
    html += "</table>";
    
    html += "<p><button type='submit'>Save Configuration</button></p>";
    html += "</div>";
    html += "</form>";
    
    html += "<p><a href=\"/\">Back to Status</a></p>";
    
    html += generateHtmlFooter();
    
    return html;
}

String WebServerManager::generateStatusJson(BeaconManager& beacon_manager, 
                                             GpsManager& gps_manager) {
    GpsData gps_data = gps_manager.getData();
    
    String json = "{";
    json += "\"beacon\": {";
    json += "\"state\": \"" + beacon_manager.getStateString() + "\",";
    json += "\"frames\": " + String(beacon_manager.getFrameCount()) + ",";
    json += "\"home_set\": " + String(beacon_manager.hasHomeSet() ? "true" : "false") + ",";
    json += "\"gps_status\": \"" + beacon_manager.getGpsStatusString() + "\"";
    json += "},";
    
    json += "\"gps\": {";
    json += "\"lat\": " + String(gps_data.position.latitude, 6) + ",";
    json += "\"lon\": " + String(gps_data.position.longitude, 6) + ",";
    json += "\"alt\": " + String(gps_data.altitude, 2) + ",";
    json += "\"speed\": " + String(gps_data.speed, 2) + ",";
    json += "\"heading\": " + String(gps_data.heading, 1) + ",";
    json += "\"sats\": " + String(gps_data.satellites) + ",";
    json += "\"hdop\": " + String(gps_data.hdop, 2) + ",";
    json += "\"has_fix\": " + String(gps_data.has_fix ? "true" : "false");
    json += "},";
    
    json += "\"network\": {";
    json += "\"mode\": \"" + String(wifiManager.isAccessPoint() ? "AP" : wifiManager.isStation() ? "STA" : "IDLE") + "\",";
    json += "\"ip\": \"" + wifiManager.getLocalIPString() + "\",";
    json += "\"ssid\": \"" + wifiManager.getSSID() + "\",";
    json += "\"rssi\": " + String(wifiManager.getRSSI());
    json += "}";
    
    json += "}";
    
    return json;
}

// ================================================================================================
// INTERNAL HANDLER FUNCTIONS
// ================================================================================================

void WebServerManager::handleRoot() {
    if (_root_handler) {
        _root_handler();
        return;
    }
    
    // Default handler: redirect to /status
    sendRedirect("/status");
}

void WebServerManager::handleFaviconRequest() {
    if (_favicon_handler) {
        _favicon_handler();
        return;
    }
    
    serveFavicon();
}

void WebServerManager::handleNotFound() {
    if (_not_found_handler) {
        _not_found_handler();
        return;
    }
    
    sendNotFound();
}
