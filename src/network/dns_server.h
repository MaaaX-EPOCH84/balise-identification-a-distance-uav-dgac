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

#ifndef DNS_SERVER_H
#define DNS_SERVER_H

#include <Arduino.h>
#include "../core/biadud_platform.h"

// ================================================================================================
// DNS SERVER CLASS
// Implements a simple DNS server for captive portal functionality
// ================================================================================================

class DnsServerManager
{
public:
    // -------------------------------------------------------------------------------------------
    // CONSTRUCTOR / DESTRUCTOR
    // -------------------------------------------------------------------------------------------
    
    DnsServerManager();
    ~DnsServerManager();

    // -------------------------------------------------------------------------------------------
    // INITIALIZATION
    // -------------------------------------------------------------------------------------------

    /**
     * Initialize DNS server
     * 
     * @param port Port to listen on (default: 53)
     */
    void begin(uint16_t port = DNS_SERVER_PORT);

    /**
     * Start DNS server
     * 
     * @param redirect_ip IP address to redirect all DNS requests to (typically the device's own IP)
     */
    void start(const IPAddress& redirect_ip = IPAddress(8, 8, 4, 4));

    /**
     * Stop DNS server
     */
    void stop();

    // -------------------------------------------------------------------------------------------
    // REQUEST HANDLING
    // -------------------------------------------------------------------------------------------

    /**
     * Process incoming DNS requests
     * Call this in loop()
     */
    void handleRequests();

    // -------------------------------------------------------------------------------------------
    // CONFIGURATION
    // -------------------------------------------------------------------------------------------

    /**
     * Set the IP address to redirect all DNS requests to
     * 
     * @param ip IP address to redirect to
     */
    void setRedirectIP(const IPAddress& ip);

    /**
     * Get the current redirect IP
     * 
     * @return Current redirect IP
     */
    IPAddress getRedirectIP() const { return _redirect_ip; }

    /**
     * Add a domain name to redirect
     * 
     * @param domain Domain name to redirect
     */
    void addRedirectDomain(const String& domain);

    /**
     * Remove a domain name from redirection
     * 
     * @param domain Domain name to remove
     */
    void removeRedirectDomain(const String& domain);

    /**
     * Clear all custom redirect domains
     */
    void clearRedirectDomains();

    // -------------------------------------------------------------------------------------------
    // STATUS CHECKS
    // -------------------------------------------------------------------------------------------

    /**
     * Check if DNS server is running
     * 
     * @return true if DNS server is running
     */
    bool isRunning() const { return _running; }

    /**
     * Check if DNS server has been initialized
     * 
     * @return true if initialized
     */
    bool isInitialized() const { return _initialized; }

    // -------------------------------------------------------------------------------------------
    // STATISTICS
    // -------------------------------------------------------------------------------------------

    /**
     * Get number of DNS requests processed
     * 
     * @return Number of requests
     */
    uint32_t getRequestCount() const { return _request_count; }

    /**
     * Get number of successful responses sent
     * 
     * @return Number of successful responses
     */
    uint32_t getResponseCount() const { return _response_count; }

    /**
     * Reset statistics
     */
    void resetStats();

    // -------------------------------------------------------------------------------------------
    // UTILITY FUNCTIONS
    // -------------------------------------------------------------------------------------------

    /**
     * Generate a random TTL (Time To Live) value for DNS responses
     * 
     * @return Random TTL value in seconds
     */
    uint32_t generateRandomTTL() const;

private:
    // -------------------------------------------------------------------------------------------
    // INTERNAL STATE
    // -------------------------------------------------------------------------------------------

    bool _initialized = false;
    bool _running = false;
    uint16_t _port = DNS_SERVER_PORT;
    
    IPAddress _redirect_ip;
    
    uint32_t _request_count = 0;
    uint32_t _response_count = 0;
    
    // Server instance (platform-specific)
    DNS_SERVER_CLASS _dns_server;

    // -------------------------------------------------------------------------------------------
    // INTERNAL HELPER FUNCTIONS
    // -------------------------------------------------------------------------------------------

    /**
     * Initialize DNS server internals
     */
    void initializeDnsServer();

    /**
     * Process a DNS request
     * 
     * @return true if request was processed successfully
     */
    bool processDnsRequest();

    /**
     * Send a DNS response
     * 
     * @param request_id Request ID from the DNS query
     * @param query Query details
     * @return true if response was sent successfully
     */
    bool sendDnsResponse(uint16_t request_id, const uint8_t* query);

    /**
     * Check if a domain should be redirected
     * 
     * @param domain Domain name to check
     * @return true if domain should be redirected
     */
    bool shouldRedirectDomain(const String& domain) const;
};

// Global instance
// extern DnsServerManager dnsServer;

#endif // #ifndef DNS_SERVER_H
