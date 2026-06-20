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

#if !defined(BIADUD_OPTIONS_H)
#define BIADUD_OPTIONS_H


//#define OPTION_UPDATE_OTA				// Pour permettre une mise a jour OTA pour logiciel

//#define OPTION_RECEIVER				// Pour inclure aussi le code récepteur (uniquement pour une carte à base ESP32 / ESP32C3)

//#define OPTION_CELLULAR				// Pour envoyer dans un SMS la position avec une module GSM SIM800L (uniquement pour une carte à base ESP32 / ESP32C3)et incompatible avec fs_iBus
										// Il faudra aussi définir GSM_RX et GSM_TX (voir exemples plus loin)

//#define OPTION_STATISTICS				// Pour avoir une page de statisques,durée exécution, erreurs GPS, etc ...Utile en phase développement ..

//#define OPTION_IBUS					// Pour envois informations GPS par telemetrie type FlySky / iBUS (incompatible avec repondeurGSM)
										// Il faudra aussi définir iBus_RX et iBus_TX (voir exemples plus loin)


//--------------------------------------------------------------------------------------------------------
//  GPS TYPE
//--------------------------------------------------------------------------------------------------------
// Select only one GPS option.
// If OPTION_GPS_QUECTEL and OPTION_GPS_UBLOX are not selected, the GPS chipset is supposed to be already configured and initialized.
//#define OPTION_GPS_QUECTEL	TODO	// GPS Quectel L80 and GPS based on MediaTek MT3339 chipset
#define OPTION_GPS_UBLOX				// GPS Beitian BN-220, BN-180, BN-880 and GPS based on u-blox M8030-KT chipset 


//--------------------------------------------------------------------------------------------------------
//  ESP8266 BOARD CONFIG
//--------------------------------------------------------------------------------------------------------
#if defined(TARGET_ESP8266)
// TODO
//	#define OPTION_BOARD_PIN_LED			8	// Optional builtin LED pin
//	#define OPTION_BOARD_PIN_GPS_RX			0	// Connect the board D1 pin on the GPS TX
//	#define OPTION_BOARD_PIN_GPS_TX			1	// Connect the board D2 pin on the GPS RX
//	#define OPTION_BOARD_PIN_FACTORY_RESET	2
#endif

//--------------------------------------------------------------------------------------------------------
//  ESP32 BOARD CONFIG
//--------------------------------------------------------------------------------------------------------
#if defined(TARGET_ESP32)
// TODO
//	#define OPTION_BOARD_PIN_LED			8	// Optional builtin LED pin
//	#define OPTION_BOARD_PIN_GPS_RX			0	// Connect the board D1 pin on the GPS TX
//	#define OPTION_BOARD_PIN_GPS_TX			1	// Connect the board D2 pin on the GPS RX
//	#define OPTION_BOARD_PIN_FACTORY_RESET	2
#endif

//--------------------------------------------------------------------------------------------------------
//  ESP32-C3 BOARD CONFIG
//--------------------------------------------------------------------------------------------------------
#if defined(TARGET_ESP32C3)
	#define OPTION_BOARD_PIN_LED			8	// Optional builtin LED pin
	#define OPTION_BOARD_PIN_GPS_RX			0	// Connect the board D1 pin on the GPS TX
	#define OPTION_BOARD_PIN_GPS_TX			1	// Connect the board D2 pin on the GPS RX
	#define OPTION_BOARD_PIN_FACTORY_RESET	2
#endif

//--------------------------------------------------------------------------------------------------------
//  ESP32-S3 BOARD CONFIG
//--------------------------------------------------------------------------------------------------------
#if defined(TARGET_ESP32S3)
// TODO
//	#define OPTION_BOARD_PIN_LED			8	// Optional builtin LED pin
//	#define OPTION_BOARD_PIN_GPS_RX			0	// Connect the board D1 pin on the GPS TX
//	#define OPTION_BOARD_PIN_GPS_TX			1	// Connect the board D2 pin on the GPS RX
//	#define OPTION_BOARD_PIN_FACTORY_RESET	2
#endif


#endif // #if !defined(BIADUD_OPTIONS_H)
