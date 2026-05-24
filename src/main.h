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
// ================================================================================================

#ifndef MAIN_H
#define MAIN_H

const char software_version[] = "0.01.000";

// Preferences saved in EEPROM
struct eeprom_preference {
	char    signature[9]    		= "BIADUD01";	// Signature for this data format
	char    password[10]     		= "";			// The access point is open by defaut
//	char    sms_command[9]  		= "";			// Par défaut on met toujours les coordonnées dans le SMS envoyé
	char    ssid_access_point[33]	= "";			// By default the ssid of the access point is based on the MAC address
	char	uav_id[31]				= "000MB1000000000000000000000001";
									// CCC								Constructor code (must be set to "000" for DIY maker)
									//    MMM							Beacon model (you can modify it) (e.g.: MB1 here is for "MaaaX's Beacon 1")
									//       111111111111				First mid part of the serial number (you can modify it)
									//                   222222222222	Last mid part of the serial number (this part will be replaced by MAC address of the board to be unique)
	char	ip_address[16]			= "8.8.4.4";	// The default android DNS
	char	ip_gateway[16]			= "8.8.4.4";
	char	ip_subnetmask[16]		= "255.255.255.0";

//	int timeoutWifi = 45;  // delais deconnection AP wifi
//	int logAfter = 5 ;   // enregistrement d'un point si déplacement de 5m  (si <0, enregistrement après n millis sec)
//	int baud = 9600;  // vitesse de transmission avec le GPS   19200 ((OK avec ESP8266), 38400
//	int nbrMaxTraces = 10; // nobre maximal de traces à conserver
//	byte hz = 1;  //  taux de rafraichissement du GPS
//	char formatTrace[4] = "csv";  //  csv gpx
//	bool logOn = true;  // true: on enregistre; false: pas de trace enregistrée
//	bool logToujours = true; // enregistrerla trace même si on ne se déplace pas
//	bool logVitesse = false;
//	bool logAltitude = true;
//	bool logHeure = true;
//	bool arretWifi = true;  // false: ne pas arreter le point d'accès Wifi en vol;  true: arreter le point d'accès
//	bool basseConso = false;  // true: couper le wifi entre 2 trames.
//	bool iBusActif = true ; // pour telémesure style FlySky. L'activation dépend de toute façon de l'option define fs_iBus dans fs_option.h
};

#define DEBUG_HEAP false
//#define DEBUG_HEAP true
#ifndef TARGET_ESP32
	#define dbgHeap(mes) \
	do { if (DEBUG_HEAP) Serial.printf_P(PSTR("%s %s():%d  FreeContStack: %5d  free: %5d - max: %5d - frag: %3d%% \n "),\
											mes, __func__, __LINE__,ESP.getFreeContStack(),\
											ESP.getFreeHeap(), ESP.getMaxFreeBlockSize(),\
											ESP.getHeapFragmentation());} while (0)
#else
	#define dbgHeap(mes) \
	do { if (DEBUG_HEAP) Serial.printf_P(PSTR("%s %s():%d free: %5d \n "),\
											mes, __func__, __LINE__,\
											ESP.getFreeHeap());} while (0)
#endif

#endif // #ifndef MAIN_H
