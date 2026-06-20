// ************************************************************************************************
// * BALISE D'IDENTIFICATION A DISTANCE POUR UAV (DGAC)                                           *
// ************************************************************************************************
// * Author:     Maxime MARCONATO (maxime@maaax.com, aka MaaaX^^, aka EPOCH84)
// * Repository: https://github.com/EPOCH84/Balise_Identification_A_Distance_UAV_DGAC
// * Based on: 
// * - https://github.com/fanfanlatulipe26/BaliseDGAC_GPS_Logger
// * - https://github.com/dev-fred/GPS_Tracker_ESP8266
// * - https://github.com/khancyr/droneID_FR
// * - https://github.com/f5soh/balise_esp32/blob/master/droneID_FR.h (version 1 https://discuss.ardupilot.org/t/open-source-french-drone-identification/56904/98 )
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
// ================================================================================================


#include "biadud_preferences.h"


//*************************************************************************************************
// preferencesSave()
//*************************************************************************************************
// Save custom preferences into EEPROM
//=================================================================================================
void preferencesSave() {
	dbgHeap("dbgHeap begin");
	Serial.print(F("Save preferences : "));
  	EEPROM.put(0, preferences_custom);
  	EEPROM.commit();
	Serial.print(F("done\n"));
  	dbgHeap("dbgHeap end");
}


//*************************************************************************************************
// preferencesRead()
//*************************************************************************************************
// Read preferences from EEPROM
//=================================================================================================
void preferencesRead() {
	Serial.print(F("Read preferences : "));
	EEPROM.get(0, preferences_custom);
	if (strcmp(preferences_factory.signature, preferences_custom.signature) != 0) {
		Serial.print(F("failed !\n"));
		Serial.print(F("Bad EEPROM data!\nReset preferences to factory default : "));
    	preferences_custom = preferences_factory;
		Serial.print(F("done\n"));
    	preferencesSave();
  	}
	else {
		Serial.print(F("done\n"));
	}
}


//*************************************************************************************************
// preferencesCheckResetToFactory()
//*************************************************************************************************
// Check if the OPTION_BOARD_PIN_FACTORY_RESET pin is enabled.
// If enabled => reset preferences to factory default values and save them into EEPROM.
// A call to checkPreferenceReset() is done once at startup.
//=================================================================================================
void preferencesCheckResetToFactory() {
	#ifdef OPTION_BOARD_PIN_FACTORY_RESET
		pinMode(OPTION_BOARD_PIN_FACTORY_RESET, INPUT_PULLUP);
		if (digitalRead(OPTION_BOARD_PIN_FACTORY_RESET) == HIGH) return;
		Serial.print(F("Factory reset asked!\nReset preferences to factory default : "));
	    preferences_custom = preferences_factory;
		preferencesSave();
		Serial.print(F("done\n"));
		Serial.print(F("STOPPED => PLEASE REBOOT !!!"));
		#if defined(OPTION_BOARD_PIN_LED)
			digitalWrite(OPTION_BOARD_PIN_LED, HIGH); // turn the LED off
		#endif
		// Wait in infinite loop for a true reboot to avoid a short circuit if
		// OPTION_BOARD_PIN_FACTORY_RESET is also used later as an ouput pin
		// (as on the pin 2 on ESP8266)
		while (true) {
			delay(1000);
		};
	#endif
}


//*************************************************************************************************
// preferencesList()
//*************************************************************************************************
// Display current custom preferences to Serial port
//=================================================================================================
void preferencesList() {
	Serial.println(F("Current preferences : "));
	Serial.print(F(" - signature : "));
	Serial.println(preferences_custom.signature);
	Serial.print(F(" - password : "));
	if (strcmp(preferences_custom.password, "") == 0) {
		Serial.print(F("<empty>\n"));
	}
	else {
		Serial.println(preferences_custom.password);
	}
	Serial.print(F(" - ssid_access_point : "));
	Serial.println(preferences_custom.ssid_access_point);
	Serial.print(F(" - uav_id : "));
	Serial.println(preferences_custom.uav_id);
	Serial.print(F(" - ip_localaddress : "));
	Serial.println(preferences_custom.ip_localaddress);
	Serial.print(F(" - ip_gatewayaddress : "));
	Serial.println(preferences_custom.ip_gatewayaddress);
	Serial.print(F(" - ip_subnetmask : "));
	Serial.println(preferences_custom.ip_subnetmask);
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
	Serial.print(F(" - gps_baud : "));
	Serial.println(preferences_custom.gps_baud);
	Serial.print(F(" - gps_rate : "));
	Serial.println(preferences_custom.gps_rate);
//	Serial.print(F("arretWifi : "));
//	Serial.println(preferences_custom.arretWifi ? "TRUE" : "FALSE");
//	Serial.print(F("timeoutWifi : "));
//	Serial.println(preferences_custom.timeoutWifi);
//	Serial.print(F("basseConso : "));
//	Serial.println(preferences_custom.basseConso ? "TRUE" : "FALSE");
//	Serial.print(F("iBusActif : "));
//	Serial.println(preferences_custom.iBusActif ? "TRUE" : "FALSE");
}
