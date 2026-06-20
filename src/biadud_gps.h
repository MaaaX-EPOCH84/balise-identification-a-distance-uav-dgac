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

#if !defined(BIADUD_GPS_H)
#define BIADUD_GPS_H


#include <Arduino.h>
#ifdef TARGET_ESP32
	#pragma message "Using HardwareSerial(1) for GPS."
#else
	SoftwareSerial serialGPS;							// ESP8266 uses SoftwareSerial only
	#pragma message "Using SoftwareSerial for GPS !"
	#include <SoftwareSerial>
#endif
#include "biadud_options.h"
//#include "TinyGPS++.h"


double		gps_status				= 1;		// GPS status
												// 	1 GPS not detected, no GPS data received.
												// 	2 Invalid GPS position (fix in progress?).
												// 	3 GPS fix lost (fix in progress?).
												// 	4 Bad GPS precision : number of satellites must be greater then 3 => gps.satellites.value().
												// 	5 Bad GPS precision : 2D precision must be lower than 5.0 => gps.hdop.hdop().
												// 	6 Bad GPS precision : Unknown altitude => gps.altitude.meters().
												// 	7 Saving home position
												// 	8 GPS OK, no error
bool		gps_home_saved			= false;	// True if home position is known and saved.
//double	gps_home_altitude		= 0;		// GPS home altitude
double		gps_previous_latitude	= 0;		// Previous GPS latitude
double		gps_previous_longitude	= 0;		// Previous GPS longitude
int16_t		gps_previous_altitude	= 0;		// Previous GPS altitude
int16_t		gps_previous_direction	= 0;		// Previous GPS direction
double		gps_previous_speed		= 0;		// Previous GPS speed (m/s)

#ifdef TARGET_ESP32
	HardwareSerial serialGPS(1);				// ESP32 uses UART 1 for GPS (UART 0 debug)
#else
	SoftwareSerial serialGPS;					// ESP8266 uses SoftwareSerial only
#endif
//TinyGPSPlus gps;

void	gpsEcho(long longDuration);
void	gpsSetSerialSpeed(uint32_t uint32Baud);

// float distanceSimple(float lat1, float lon1, float lat2, float lon2);
// void setGpsRate(uint32_t rate);
// void limitNmeaSentences();
// void fs_initGPS(uint32_t baud, uint32_t rate) ;
// void setGpsSpeed(uint32_t baud);
// void addChecksum(char *buff);
// void sendPacket(byte * packet, byte len);
// void crc_ublox( byte *msg, uint32_t size);

#endif // #if !defined(BIADUD_GPS_H)
