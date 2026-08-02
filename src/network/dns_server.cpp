// ************************************************************************************************
// * BALISE D'IDENTIFICATION A DISTANCE POUR UAV (DGAC)                                           *
// ***********************************************************************************************
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

#include "dns_server.h"

// Global instance
DnsServerManager dnsServer;

// ================================================================================================
// CONSTRUCTOR / DESTRUCTOR
// ================================================================================================

DnsServerManager::DnsServerManager() {
    _initialized = false;
    _running = false;
    _port = DNS_SERVER_PORT;
    _redirect_ip = IPAddress(8, 8, 4, 4); // Default redirect to Google DNS (for captive portal)
    _request_count = 0;
    _response_count = 0;
}

DnsServerManager::~DnsServerManager() {
    stop();
}

// ================================================================================================
// INITIALIZATION
// ================================================================================================

void DnsServerManager::begin(uint16_t port) {
    _port = port;
    initializeDnsServer();
    _initialized = true;
}

void DnsServerManager::initializeDnsServer() {
    // DNS server initialization is handled by the library
    // We just need to create the instance with the port
    // Note: The actual initialization happens in start()
}

void DnsServerManager::start(const IPAddress& redirect_ip) {
    if (!_initialized) {
        begin();
    }
    
    if (_running) {
        stop();
    }
    
    _redirect_ip = redirect_ip;
    
    // Start DNS server on the configured port
    _dns_server.start(_port, "*", _redirect_ip);
    _running = true;
}

void DnsServerManager::stop() {
    if (_running) {
        _dns_server.stop();
        _running = false;
    }
}

// ================================================================================================
// REQUEST HANDLING
// ================================================================================================

void DnsServerManager::handleRequests() {
    if (_running) {
        _dns_server.processNextRequest();
        _request_count++;
    }
}

// ================================================================================================
// CONFIGURATION
// ================================================================================================

void DnsServerManager::setRedirectIP(const IPAddress& ip) {
    _redirect_ip = ip;
    
    // If server is running, restart it with new IP
    if (_running) {
        stop();
        start(_redirect_ip);
    }
}

void DnsServerManager::addRedirectDomain(const String& domain) {
    // Note: The standard DNSServer library redirects all requests
    // This function is for future expansion if we need selective redirection
    // For now, all domains are redirected to the configured IP
}

void DnsServerManager::removeRedirectDomain(const String& domain) {
    // For future use
}

void DnsServerManager::clearRedirectDomains() {
    // For future use
}

// ================================================================================================
// STATISTICS
// ================================================================================================

void DnsServerManager::resetStats() {
    _request_count = 0;
    _response_count = 0;
}

// ================================================================================================
// UTILITY FUNCTIONS
// ================================================================================================

uint32_t DnsServerManager::generateRandomTTL() const {
    // Generate a random TTL between 1 and 3600 seconds
    // This helps with DNS caching
    return random(1, 3600);
}

// ================================================================================================
// INTERNAL HELPER FUNCTIONS
// ================================================================================================

bool DnsServerManager::shouldRedirectDomain(const String& domain) const {
    // For captive portal, we want to redirect all domains
    // This allows the device to capture all DNS requests
    return true;
}
