//*************************************************************************************************
// BALISE D'IDENTIFICATION A DISTANCE POUR UAV (DGAC)
//*************************************************************************************************
// Author:     Maxime MARCONATO (maxime@maaax.com, aka MaaaX^^, aka EPOCH84)
// Repository: https://github.com/EPOCH84/Balise_Identification_A_Distance_UAV_DGAC
// Based on:
// - https://github.com/fanfanlatulipe26/BaliseDGAC_GPS_Logger
// - https://github.com/dev-fred/GPS_Tracker_ESP8266
// - https://github.com/khancyr/droneID_FR
// - https://github.com/f5soh/balise_esp32/blob/master/droneID_FR.h (version 1 https://discuss.ardupilot.org/t/open-source-french-drone-identification/56904/98 )
// - https://github.com/f5soh/balise_esp32
// - https://www.tranquille-informatique.fr/modelisme/divers/balise-dgac-signalement-electronique-a-distance-drone-aeromodelisme.html
//*************************************************************************************************
// LICENCE GPL V3:
// This program is free software: you can redistribute it and/or modify it under the terms of the
// GNU General Public License as published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.
// This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
// without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
//=================================================================================================

#include <Arduino.h>

#include "main.h"
#include "options.h"
#ifdef TARGET_ESP32
	#ifdef TARGET_ESP32C3
		#pragma message "Build for ESP32-C3 board."
	#else
		#ifdef TARGET_ESP32S3
			#pragma message "Build for ESP32-S3 board."
		#else
			#pragma message "Build for ESP32 board."
		#endif
	#endif
	#include <WiFi.h>
//	#include "fs_WebServer.h"
#else
	#ifdef TARGET_ESP8266
		#pragma message "Build for ESP8266 board."
		#include <ESP8266WebServer.h>
		#include <ESP8266WiFi.h>
		#include <SoftwareSerial.h>
	#else
		#error "You can build for ESP32, ESP32-C3, ESP32-S3 or ESP8266 boards only !"
	#endif
#endif

#include <LittleFS.h>
//#include "TinyGPS++.h"
#include <EEPROM.h>
#include <DNSServer.h>

const char	ssid_prefix[]			= "BALISE";	// SSID prefix
char		ssid[33];							// SSID = SSID prefix + MAC address

double		gps_status				= 1;		// GPS status
												// 1	GPS not detected, no GPS data received.
												// 2	Invalid GPS position (fix in progress?).
												// 3	GPS fix lost (fix in progress?).
												// 4	Bad GPS precision : number of satellites must be greater then 3 => gps.satellites.value().
												// 5	Bad GPS precision : 2D precision must be lower than 5.0 => gps.hdop.hdop().
												// 6	Bad GPS precision : Unknown altitude => gps.altitude.meters().
												// 7	Saving home position
												// 8	GPS OK, no error
bool		gps_home_saved			= false;	// True if home position is known and saved.
//double	gps_home_altitude		= 0;		// GPS home altitude
double		gps_previous_latitude	= 0;		// Previous GPS latitude
double		gps_previous_longitude	= 0;		// Previous GPS longitude
int16_t		gps_previous_altitude	= 0;		// Previous GPS altitude
int16_t		gps_previous_direction	= 0;		// Previous GPS direction
double		gps_previous_speed		= 0;		// Previous GPS speed (m/s)

// UAV ID
char		uav_id[31]				= "000MB1000000000000000000000001";
									// CCC								Constructor code (must be set to "000" for DIY maker)
									//    MMM							Beacon model (you can modify it) (e.g.: MB1 here is for "MaaaX's Beacon 1")
									//       111111111111				First mid part of the serial number (you can modify it)
									//                   222222222222	Last mid part of the serial number (this part will be replaced by MAC address of the board to be unique)

struct eeprom_preference	preferences_factory;	// Custom preferences
struct eeprom_preference	preferences_custom;		// Default factory preferences

#if defined(TARGET_ESP32)
	extern "C" {
		#include "esp_wifi.h"
		esp_err_t esp_wifi_80211_tx(wifi_interface_t ifx, const void *buffer, int len, bool en_sys_seq);
		esp_err_t esp_wifi_internal_set_fix_rate(wifi_interface_t ifx, bool en, wifi_phy_rate_t rate);
	}
#else
	extern "C" {
		#include "user_interface.h"
		int wifi_send_pkt_freedom(uint8 *buf, int len, bool sys_seq);
	}
#endif


//*************************************************************************************************
// savePreferences()
//*************************************************************************************************
// Save custom preferences into EEPROM
//=================================================================================================
void savePreferences() {
  dbgHeap("debut");
  EEPROM.put(0, preferences_custom);
  EEPROM.commit();
  dbgHeap("fin");
}


//*************************************************************************************************
// readPreferences()
//*************************************************************************************************
// Read preferences from EEPROM
//=================================================================================================
void readPreferences() {
  EEPROM.get(0, preferences_custom);
  if (strcmp(preferences_factory.signature, preferences_custom.signature) != 0) {
    Serial.print(F("Bad EEPROM data signature => reseting preferences to factory default values !"));
    preferences_custom = preferences_factory;
    savePreferences();
  }
}


//*************************************************************************************************
// checkPreferencesResetToFactory()
//*************************************************************************************************
// Check if the OPTION_BOARD_PIN_FACTORY_RESET pin is enabled.
// If enabled => reset preferences to factory default values and save them into EEPROM.
// A call to checkPreferenceReset() is done at startup.
//=================================================================================================
void checkPreferencesResetToFactory() {
	#ifdef OPTION_BOARD_PIN_FACTORY_RESET
		pinMode(OPTION_BOARD_PIN_FACTORY_RESET, INPUT_PULLUP);
		if (digitalRead(OPTION_BOARD_PIN_FACTORY_RESET) == HIGH) return;
	    preferences_custom = preferences_factory;
		savePreferences();
		// Wait in infinite loop for a true rebootto avoid a short circuit if
		// OPTION_BOARD_PIN_FACTORY_RESET is also used later as an ouput pin
		// (as on the pin 2 on ESP8266)
		while (true) {
			delay(1000);
		};
	#endif
}


//*************************************************************************************************
// listPreferences()
//*************************************************************************************************
// Display current custom preferences to Serial port
//=================================================================================================
void listPreferences() {
	Serial.print(F("signature : "));
	Serial.println(preferences_custom.signature);
	Serial.print(F("password : "));
	Serial.println(preferences_custom.password);
	Serial.print(F("ssid_access_point : "));
	Serial.println(preferences_custom.ssid_access_point);
	Serial.print(F("uav_id : "));
	Serial.println(preferences_custom.uav_id);
//	Serial.print(F("SMSCommand : "));
//	Serial.println(preferences_custom.SMSCommand);
//	Serial.print(F("logOn : "));
//	Serial.println(preferences_custom.logOn ? "TRUE" : "FALSE");
//	Serial.print(F("logToujours : "));
//	Serial.println(preferences_custom.logToujours ? "TRUE" : "FALSE");
//	Serial.print(F("logAfter : "));
//	Serial.println(preferences_custom.logAfter);
//	Serial.print(F("formatTrace : "));
//	Serial.println(preferences_custom.formatTrace);
//	Serial.print(F("logVitesse : "));
//	Serial.println(preferences_custom.logVitesse ? "TRUE" : "FALSE");
//	Serial.print(F("logAltitude : "));
//	Serial.println(preferences_custom.logAltitude ? "TRUE" : "FALSE");
//	Serial.print(F("logHeure : "));
//	Serial.println(preferences_custom.logHeure ? "TRUE" : "FALSE");
//	Serial.print(F("nbrMaxTraces : "));
//	Serial.println(preferences_custom.nbrMaxTraces);
//	Serial.print(F("baud : "));
//	Serial.println(preferences_custom.baud);
//	Serial.print(F("hz : "));
//	Serial.println(preferences_custom.hz);
//	Serial.print(F("arretWifi : "));
//	Serial.println(preferences_custom.arretWifi ? "TRUE" : "FALSE");
//	Serial.print(F("timeoutWifi : "));
//	Serial.println(preferences_custom.timeoutWifi);
//	Serial.print(F("basseConso : "));
//	Serial.println(preferences_custom.basseConso ? "TRUE" : "FALSE");
//	Serial.print(F("iBusActif : "));
//	Serial.println(preferences_custom.iBusActif ? "TRUE" : "FALSE");
}


//*************************************************************************************************
// setup()
//*************************************************************************************************
// Called first once at startup, before loop().
//=================================================================================================
void setup() {
	#if defined(OPTION_BOARD_PIN_LED)
		pinMode(OPTION_BOARD_PIN_LED, OUTPUT);
	#endif

	Serial.begin(115200);
	Serial.setDebugOutput(true);
	while (!Serial)
	;
	Serial.printf("Serial debug output enabled\n\n");

	Serial.print(F("\n\nBALISE_IDENTIFICATION_A_DISTANCE_UAV_DGAC v"));
	Serial.print(software_version);
	Serial.println(F("  " __DATE__ " " __TIME__));
	Serial.print(F("GPS: "));
#ifdef GPS_quectel
	Serial.println(F("Quectel"));
#elif defined(GPS_ublox)
	Serial.println(F("Ublox"));
#else
	Serial.println(F("undefined"));
#endif
	Serial.printf_P(PSTR("Pin %i => GPS RX\nPin %i <= GPS TX\n"), OPTION_BOARD_PIN_GPS_TX, OPTION_BOARD_PIN_GPS_RX);

	// calcul des valeurs par defaut de l'identificateur de la balise et du nom du réseau point d'acces Wifi
	// Ces noms sont basés sur l'adresse MAC mais peuvent être changé par l'utilisateur.

	// Read the Wifi MAC address of the board
	String WifiMacAddress = WiFi.macAddress();
	// Define the default ssid_access_port
	String ssid_ap_default = String(ssid_prefix)+"_"+WifiMacAddress;
	ssid_ap_default.toCharArray(preferences_factory.ssid_access_point, 32);
	Serial.print(F("Default ssid_access_point : "));
	Serial.println(preferences_factory.ssid_access_point);
	// Define the last mid part of the beacon_id serial number
	WifiMacAddress.replace(":", "");					// 12 chars after remove of ":"
	strcpy(&uav_id[18], WifiMacAddress.c_str());		// placed at the las part of the beacon_id serial number
	strcpy(preferences_factory.uav_id, uav_id);
	Serial.print(F("Default uav_id : "));
	Serial.println(preferences_factory.uav_id);

	// Preferences in EEPROM
	EEPROM.begin(512);
	checkPreferencesResetToFactory();
	readPreferences();
	Serial.println(F("Preferences readen from EEPROM : "));
	listPreferences();

 	// // Initialize the GPS at 9600 Bauds. This speed will be changed later according preferences.
	// #if defined(TARGET_ESP32)
	//  	serialGPS.begin(9600, SERIAL_8N1, OPTION_BOARD_PIN_GPS_RX, OPTION_BOARD_PIN_GPS_TX);
	// #else
	//  	serialGPS.begin(9600, SWSERIAL_8N1, OPTION_BOARD_PIN_GPS_RX, OPTION_BOARD_PIN_GPS_TX);
	//  	if (!serialGPS) {  // If the object did not initialize, then its configuration is invalid
	// 	 	Serial.println(F("Invalid SoftwareSerial pin configuration, check config"));
	// 		while (1) {  // Don't continue with invalid configuration
	// 			delay(1000);
	// 		}
 	// 	}
	// #endif



	log_d("Setup done");
	Serial.printf("Setup done\n");
}

//*************************************************************************************************
// loop()
//*************************************************************************************************
// Called in infinitly, after setup().
//=================================================================================================
void loop() {
	Serial.printf("Loop begin\n");
	// "O" in morse code
	digitalWrite(GPIO_NUM_8, LOW); // turn the LED on
	Serial.printf("LED : on\n");
	delay(1000);
	digitalWrite(GPIO_NUM_8, HIGH); // turn the LED off
	Serial.printf("LED : off\n");
	delay(500);
	digitalWrite(GPIO_NUM_8, LOW); // turn the LED on
	Serial.printf("LED : on\n");
	delay(1000);
	digitalWrite(GPIO_NUM_8, HIGH); // turn the LED off
	Serial.printf("LED : off\n");
	delay(500);
	digitalWrite(GPIO_NUM_8, LOW); // turn the LED on
	Serial.printf("LED : on\n");
	delay(1000);
	digitalWrite(GPIO_NUM_8, HIGH); // turn the LED off
	Serial.printf("LED : off\n");
	delay(1500);

	// "K" in morse code
	digitalWrite(GPIO_NUM_8, LOW); // turn the LED on
	Serial.printf("LED : on\n");
	delay(1000);
	digitalWrite(GPIO_NUM_8, HIGH); // turn the LED off
	Serial.printf("LED : off\n");
	delay(500);
	digitalWrite(GPIO_NUM_8, LOW); // turn the LED on
	Serial.printf("LED : on\n");
	delay(300);
	digitalWrite(GPIO_NUM_8, HIGH); // turn the LED off
	Serial.printf("LED : off\n");
	delay(500);
	digitalWrite(GPIO_NUM_8, LOW); // turn the LED on
	Serial.printf("LED : on\n");
	delay(1000);
	digitalWrite(GPIO_NUM_8, HIGH); // turn the LED off
	Serial.printf("LED : off\n");
	delay(3000);

	// Serial.println(F("Preferences readen from EEPROM : "));
	// listPreferences();

	Serial.printf("Loop end\n");
}
