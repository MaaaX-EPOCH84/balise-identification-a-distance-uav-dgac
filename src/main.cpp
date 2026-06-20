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

#include "main.h"


//*************************************************************************************************
// setup()
//*************************************************************************************************
// First function called once at startup, before loop().
//=================================================================================================
void setup() {
	#if defined(OPTION_BOARD_PIN_LED)
		pinMode(OPTION_BOARD_PIN_LED, OUTPUT);
		digitalWrite(OPTION_BOARD_PIN_LED, HIGH); // turn the LED off
	#endif
	
	Serial.begin(115200);
	Serial.setDebugOutput(true);
	while (!Serial) {
		delay(250);
	};					// Waiting until serial is ready
	delay(3000);		// Wait 3 additional seconds to let the time at the user to connect a serial debug console
	#if defined(OPTION_BOARD_PIN_LED)
		digitalWrite(OPTION_BOARD_PIN_LED, LOW); // turn the LED on
	#endif
	
	Serial.print(F("\n===== BALISE_IDENTIFICATION_A_DISTANCE_UAV_DGAC v"));
	Serial.print(beacon_version);
	Serial.print(F(" (" __DATE__ " " __TIME__ ") =====\n\n"));

	// Read the Wifi MAC address of the board
	Serial.print(F("Wifi MAC address : "));
	String WifiMacAddress = WiFi.macAddress();
	Serial.print(WifiMacAddress);
	Serial.print(F("\n\n"));
	WifiMacAddress.replace(":", "");					// 12 chars after remove of ":" for later use

	// Define the default ssid_access_port
	Serial.print(F("Default preferences : \n"));
	Serial.print(F(" - ssid_access_point : "));
	String ssid_ap_default = String(ssid_prefix)+"_"+WifiMacAddress;
	ssid_ap_default.toCharArray(preferences_factory.ssid_access_point, 32);
	Serial.println(preferences_factory.ssid_access_point);
	// Define the last mid part of the beacon_id serial number
	Serial.print(F(" - uav_id : "));
	strcpy(&uav_id[18], WifiMacAddress.c_str());		// placed at the las part of the beacon_id serial number
	strcpy(preferences_factory.uav_id, uav_id);
	Serial.print(preferences_factory.uav_id);
	Serial.print(F("\n\n"));

	// Init/Read preferences in EEPROM
	EEPROM.begin(512);
	preferencesCheckResetToFactory();
	preferencesRead();
	strcpy(uav_id, preferences_custom.uav_id);
	strcpy(ssid, preferences_custom.ssid_access_point);
	ip_localaddress.fromString(preferences_custom.ip_localaddress);
	ip_gatewayaddress.fromString(preferences_custom.ip_gatewayaddress);
	ip_subnetmask.fromString(preferences_custom.ip_subnetmask);
	preferencesList();
	Serial.print(F("\n"));

	// Initialize the GPS at 9600 Bauds. This speed will be changed later according preferences.
	Serial.print(F("GPS type : "));
	#if defined(OPTION_GPS_QUECTEL)
		Serial.println(F("Quectel"));
	#elif defined(OPTION_GPS_UBLOX)
		Serial.println(F("Ublox"));
	#else
		Serial.println(F("undefined"));
	#endif
	Serial.printf_P(PSTR(" - Board pin %i => GPS RX\n - Board pin %i <= GPS TX\n"), OPTION_BOARD_PIN_GPS_TX, OPTION_BOARD_PIN_GPS_RX);

	Serial.print(F("Init GPS (9600 bauds)...\n"));
	#if defined(TARGET_ESP32)
		Serial.print(F("GPS uses HardwareSerial(1)...\n"));
	 	serialGPS.begin(9600, SERIAL_8N1, OPTION_BOARD_PIN_GPS_RX, OPTION_BOARD_PIN_GPS_TX);
//	 	if (!xxx) {	// How to check if init is correctly done ?
//			Serial.print(F("GPG Init : failed!\n"));
//			Serial.print(F("STOPPED => PLEASE REBOOT !!!"));
//			#if defined(OPTION_BOARD_PIN_LED)
//				digitalWrite(OPTION_BOARD_PIN_LED, HIGH); // turn the LED off
//			#endif
//			while (true) {		// Don't continue with invalid configuration
//				delay(1000);
//			}
//		}
	#else
		Serial.print(F("GPS uses SoftwareSerial...\n"));
	 	serialGPS.begin(9600, SWSERIAL_8N1, OPTION_BOARD_PIN_GPS_RX, OPTION_BOARD_PIN_GPS_TX);
	 	if (!serialGPS) {	// If the object did not initialize, then its configuration is invalid
			Serial.print(F("GPG Init : failed !\n"));
			Serial.print(F("STOPPED => PLEASE REBOOT !!!"));
			#if defined(OPTION_BOARD_PIN_LED)
				digitalWrite(OPTION_BOARD_PIN_LED, HIGH); // turn the LED off
			#endif
			while (true) {		// Don't continue with invalid configuration
				delay(1000);
			}
		}
	#endif
	Serial.print(F("...Init GPS : done\n\n"));

	delay(1500);														// Quectel GPS needs ~1.1s to be ready (it doesn't accept any command before). Ubloxd GPS needs ~600ms only.
	gpsInit(preferences_custom.gps_baud, preferences_custom.gps_rate);	// init du GPS (vitesse, refresh) et serialGPS
	drone_idfr.set_drone_id(preferences_custom.uav_id);



	// Start WiFi
	Serial.print(F("Init Wifi...\n"));
	WiFi.mode(WIFI_AP);
	delay(250);
	Serial.print(F("... Init Wifi : done\n"));
	Serial.print(F("Configure Wifi Access Point...\n"));
	if (!WiFi.softAPConfig(ip_localaddress, ip_gatewayaddress, ip_subnetmask)) {
		Serial.print(F("...Configure Wifi Access Point : failed !\n"));
		Serial.print(F("STOPPED => PLEASE REBOOT !!!"));
		#if defined(OPTION_BOARD_PIN_LED)
			digitalWrite(OPTION_BOARD_PIN_LED, HIGH); // turn the LED off
		#endif
		while (true) {		// Don't continue with invalid configuration
			delay(1000);
		}
	}
	delay(250);
	Serial.print(F("...Configure Wifi Access Point : done\n"));
	// ssid, pwd, channel, hidden, max_cnx
	Serial.print(F("Init Wifi Access Point...\n"));
	if (!WiFi.softAP(ssid, preferences_custom.password, 6, false, 4)) {	// 4 users maximum
		Serial.print(F("...Init Wifi Access Point : failed !\n"));
		Serial.print(F("STOPPED => PLEASE REBOOT !!!"));
		#if defined(OPTION_BOARD_PIN_LED)
			digitalWrite(OPTION_BOARD_PIN_LED, HIGH); // turn the LED off
		#endif
		while (true) {		// Don't continue with invalid configuration
			delay(1000);
		}
	}
	delay(250);
	Serial.print(F("...Init Wifi Access Point : done\n"));
	Serial.print(F("IP address for Wifi Access Point network "));
	Serial.print(ssid);
	Serial.print(F(" : "));
	Serial.print(WiFi.softAPIP());
	Serial.print(F(" / "));
	Serial.println(WiFi.softAPSubnetMask());
	Serial.print(F("\n"));

	serverStart();									// Start WEB server, DNS server, file system...

	#if defined(repondeurGSM)
		GSMInit();
	#endif

	#if defined(fs_iBus)
		// init et stop evenntuel: on sera pret pour relancer de façon simple la télémétrie
		iBusInit(); 
		delay(200);
		if (!preferences.iBusActif) iBusStop();
	#endif

	#if defined(fs_STAT)
		razStatistics();
	#endif
	Serial.println(F("Attente du fix & Co"));







	log_d("Setup done");
	Serial.printf("Setup done\n\n\n");
	Serial.printf("=== Setup done ===\n\n\n");
	#if defined(OPTION_BOARD_PIN_LED)
		digitalWrite(OPTION_BOARD_PIN_LED, HIGH); // turn the LED off
	#endif
}

//*************************************************************************************************
// loop()
//*************************************************************************************************
// Called in infinitly, after setup().
//=================================================================================================
void loop() {
	Serial.printf("=== Loop begin ===\n");

	#if defined(OPTION_BOARD_PIN_LED)
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
	#endif

	// Serial.println(F("Preferences readen from EEPROM : "));
	// listPreferences();

	Serial.printf("=== Loop end ===\n");
}
