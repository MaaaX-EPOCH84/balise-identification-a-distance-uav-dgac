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

#ifndef WEB_SERVER_H
#define WEB_SERVER_H

#include <Arduino.h>
#include "../core/biadud_platform.h"
#include "wifi_manager.h"

// Forward declaration
class BeaconManager;
class GpsManager;
class EepromManager;

// ================================================================================================
// WEB SERVER CLASS
// ================================================================================================

class WebServerManager
{
public:
    // -------------------------------------------------------------------------------------------
    // CONSTRUCTOR / DESTRUCTOR
    // -------------------------------------------------------------------------------------------
    
    WebServerManager();
    ~WebServerManager();

    // -------------------------------------------------------------------------------------------
    // INITIALIZATION
    // -------------------------------------------------------------------------------------------

    /**
     * Initialize web server
     * 
     * @param port Port to listen on (default: 80)
     */
    void begin(uint16_t port = 80);

    /**
     * Start web server
     */
    void start();

    /**
     * Stop web server
     */
    void stop();

    // -------------------------------------------------------------------------------------------
    // REQUEST HANDLING
    // -------------------------------------------------------------------------------------------

    /**
     * Process incoming client requests
     * Call this in loop()
     */
    void handleClient();

    // -------------------------------------------------------------------------------------------
    // ROUTE HANDLERS
    // -------------------------------------------------------------------------------------------

    /**
     * Set handler for root path ("/")
     * 
     * @param handler Function to call when root is requested
     */
    void onRoot(void (*handler)());

    /**
     * Set handler for favicon.ico
     * 
     * @param handler Function to call when favicon is requested
     */
    void onFavicon(void (*handler)());

    /**
     * Set handler for 404 (not found)
     * 
     * @param handler Function to call when path is not found
     */
    void onNotFound(void (*handler)());

    // -------------------------------------------------------------------------------------------
    // STATUS CHECKS
    // -------------------------------------------------------------------------------------------

    /**
     * Check if server is running
     * 
     * @return true if server is running
     */
    bool isRunning() const { return _running; }

    /**
     * Check if server has active clients
     * 
     * @return true if there are active clients
     */
    bool hasClients() const;

    /**
     * Get number of active clients
     * 
     * @return Number of active clients
     */
    uint8_t getClientCount() const;

    // -------------------------------------------------------------------------------------------
    // RESPONSE HELPERS
    // -------------------------------------------------------------------------------------------

    /**
     * Send HTTP response with status code and content
     * 
     * @param code HTTP status code
     * @param content_type Content-Type header
     * @param content Response body
     */
    void send(int code, const String& content_type, const String& content);

    /**
     * Send HTTP response with status code and PROGMEM content
     * 
     * @param code HTTP status code
     * @param content_type Content-Type header (PROGMEM string)
     * @param content Response body (PROGMEM string)
     */
    void send_P(int code, PGM_P content_type, PGM_P content);

    /**
     * Send JSON response
     * 
     * @param code HTTP status code
     * @param json JSON string to send
     */
    void sendJson(int code, const String& json);

    /**
     * Send plain text response
     * 
     * @param code HTTP status code
     * @param text Text to send
     */
    void sendText(int code, const String& text);

    /**
     * Send HTML response
     * 
     * @param code HTTP status code
     * @param html HTML to send
     */
    void sendHtml(int code, const String& html);

    /**
     * Send 200 OK response
     * 
     * @param content_type Content-Type
     * @param content Response body
     */
    void sendOk(const String& content_type, const String& content);

    /**
     * Send redirect response
     * 
     * @param url URL to redirect to
     */
    void sendRedirect(const String& url);

    /**
     * Send 404 Not Found response
     */
    void sendNotFound();

    /**
     * Send 500 Server Error response
     * 
     * @param message Error message
     */
    void sendServerError(const String& message = "Internal Server Error");

    // -------------------------------------------------------------------------------------------
    // REQUEST INFORMATION
    // -------------------------------------------------------------------------------------------

    /**
     * Get current request method (GET, POST, etc.)
     * 
     * @return Request method string
     */
    String getRequestMethod() const;

    /**
     * Get current request URI
     * 
     * @return Request URI string
     */
    String getRequestURI() const;

    /**
     * Get current request path
     * 
     * @return Request path string
     */
    String getRequestPath() const;

    /**
     * Check if current request is GET
     * 
     * @return true if GET request
     */
    bool isGetRequest() const;

    /**
     * Check if current request is POST
     * 
     * @return true if POST request
     */
    bool isPostRequest() const;

    /**
     * Get query parameter value
     * 
     * @param name Parameter name
     * @return Parameter value (empty if not found)
     */
    String getQueryParam(const String& name) const;

    /**
     * Get form parameter value (from POST body)
     * 
     * @param name Parameter name
     * @return Parameter value (empty if not found)
     */
    String getFormParam(const String& name) const;

    // -------------------------------------------------------------------------------------------
    // FILE SERVING
    // -------------------------------------------------------------------------------------------

    /**
     * Serve a file from LittleFS
     * 
     * @param path File path in LittleFS
     * @return true if file was served successfully
     */
    bool serveFile(const String& path);

    /**
     * Check if a file exists in LittleFS
     * 
     * @param path File path
     * @return true if file exists
     */
    bool fileExists(const String& path) const;

    // -------------------------------------------------------------------------------------------
    // STATIC CONTENT
    // -------------------------------------------------------------------------------------------

    /**
     * Serve favicon
     */
    void serveFavicon();

    /**
     * Generate favicon.ico content (base64 encoded)
     * Note: This is a placeholder, actual favicon should be stored in LittleFS
     */
    static const char* getFaviconContent();

    // -------------------------------------------------------------------------------------------
    // HELPER FUNCTIONS FOR WEB PAGES
    // -------------------------------------------------------------------------------------------

    /**
     * Generate HTML header
     * 
     * @param title Page title
     * @return HTML header string
     */
    static String generateHtmlHeader(const String& title);

    /**
     * Generate HTML footer
     * 
     * @return HTML footer string
     */
    static String generateHtmlFooter();

    /**
     * Generate HTML page with beacon status
     * 
     * @param beacon_manager Reference to beacon manager
     * @param gps_manager Reference to GPS manager
     * @return HTML page string
     */
    static String generateStatusPage(BeaconManager& beacon_manager, 
                                     GpsManager& gps_manager, 
                                     EepromManager& eeprom_manager);

    /**
     * Generate HTML page for configuration
     * 
     * @param eeprom_manager Reference to EEPROM manager
     * @return HTML page string
     */
    static String generateConfigPage(EepromManager& eeprom_manager);

    /**
     * Generate JSON response with current status
     * 
     * @param beacon_manager Reference to beacon manager
     * @param gps_manager Reference to GPS manager
     * @return JSON string
     */
    static String generateStatusJson(BeaconManager& beacon_manager, 
                                     GpsManager& gps_manager);

private:
    // -------------------------------------------------------------------------------------------
    // INTERNAL STATE
    // -------------------------------------------------------------------------------------------

    bool _running = false;
    uint16_t _port = 80;
    
    // Server instance (platform-specific)
    // We use void* to store the server instance to avoid type conflicts
    void* _server_ptr = nullptr;
    
    // Handler function pointers
    void (*_root_handler)() = nullptr;
    void (*_favicon_handler)() = nullptr;
    void (*_not_found_handler)() = nullptr;
    
    // Current client tracking
    mutable bool _has_current_client = false;

    // -------------------------------------------------------------------------------------------
    // INTERNAL HELPER FUNCTIONS
    // -------------------------------------------------------------------------------------------

    /**
     * Process current client request
     */
    void processCurrentClient();

    /**
     * Handle root request
     */
    void handleRoot();

    /**
     * Handle favicon request
     */
    void handleFaviconRequest();

    /**
     * Handle not found request
     */
    void handleNotFound();

    /**
     * Parse query parameters from URI
     */
    void parseQueryParams(const String& uri);

    /**
     * Parse form parameters from POST body
     */
    void parseFormParams();
};

// Global instance
// extern WebServerManager webServer;

#endif // #ifndef WEB_SERVER_H
