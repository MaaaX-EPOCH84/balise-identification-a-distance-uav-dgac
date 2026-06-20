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

#include "biadud_web.h"


//*************************************************************************************************
// loadFromSPIFFS()
//*************************************************************************************************
// Load file data from SPIF filesystem.
// Parameter path (String) : file path formated like server.uri(), then it begins always by "/".
// Returned value (bool)   : True if the file data are loaded, else False.
//=================================================================================================
bool loadFromSPIFFS(String path) {

	// Les autres demandes sont a tous les coups des "notFound"
	// Les autres (fichiers log erreur sont simplement directement téléchargés.
	
	// "/errorlog.txt" is loaded directly
	if (path.substring(0, 13) == "/errorlog.txt") {
		File dataFile = LittleFS.open(path.c_str(), "r");
		if (!dataFile) return false;
		server_web.streamFile(dataFile, "text/plain");
		dataFile.close();
		return true;
	}

	// Every trace file begin by "/trace" and can't be loaded here
	if (path.substring(0, 2) != "/trace") return false;
	
	// download du fichier "path". On va générer un ficher .csv ou .gpx suivant les option,
	// a partir du log binaire
	// structure servant à stocker en binaire une ligne (un point) du log de trace
	trackLigne_t trackLigne;
	//   /yyyy-mm-dd
	//   01234678901
	// dateDuFichier va servir à construire le champ date/heure du fichier GPX
	String dateDuFichier = path.substring(1, 11);  // on oublit le / du debut de l'uri yyyy-mm-dd
	if (strcmp(traceFileName, path.c_str()) == 0) {
		Serial.println(F("Fermeture fichier trace en cours"));
		traceFile.close();
		traceFileOpened = false;
	}

	File dataFile = LittleFS.open(path.c_str(), "r");
	if (!dataFile) return false;
	Serial.print(F("download fichier "));
	Serial.println(path);
	server_web.chunkedResponseModeStart(200, "text/plain");
	// creer le header
	if (strcmp(preferences_custom.traceFormat, "csv") == 0) {
		String logMessage;
		if (preferences_custom.logTime) logMessage += "Heure,";
		logMessage += "Latitude,Longitude";
		if (preferences_custom.logSpeed) logMessage += ",Vitesse,Vitesse_max_segment";
		if (preferences_custom.logAltitude) logMessage += ",Altitude,Altitude_max_segment";
		logMessage += "\n";
		server_web.sendContent(logMessage);
	}
	else {
		server_web.sendContent_P(contentHeaderGPX);
	}
	char buf[1024];
	int deb;
	deb = 0;
	while (dataFile.available()) {
		dataFile.read((uint8_t *)&trackLigne, sizeof(trackLigne));
		if (strcmp(preferences_custom.traceFormat, "csv") == 0) {  //generation CSV
			//  12:30:31,-90.00000,-180.00000,1000.00,1000.00,99999.99,99999.99
			//  123456789012345678901234567890123456789012345678901234567890123
			//  Soit un total de 63 + 2 (cr/lf) + 1 = 65 caractères par point CSV max
			if (sizeof(buf) - deb < 70) {
				// Serial.printf("Ecriture buffer download %i\n", deb);
				server_web.sendContent((const char *)buf, deb);
				deb = 0;
				buf[0] = 0;  // chaine vide
			}
			if (preferences_custom.logTime) deb += sprintf_P(&buf[deb], PSTR("%02u:%02u:%02u.%02u,"), trackLigne.hour, trackLigne.minute, trackLigne.second, trackLigne.centisecond);
			deb += sprintf_P(&buf[deb], PSTR("%.6f,%.6f"), trackLigne.lat, trackLigne.lng);
			if (preferences_custom.logSpeed) {
				deb += sprintf_P(&buf[deb], PSTR(",%.2f"), trackLigne.speed);
				deb += sprintf_P(&buf[deb], PSTR(",%.2f"), trackLigne.VmaxSegment);
			}
			if (preferences_custom.logAltitude) {
				deb += sprintf_P(&buf[deb], PSTR(",%.2f"), trackLigne.altitude);
				deb += sprintf_P(&buf[deb], PSTR(",%.2f"), trackLigne.AltMaxSegment);
			}
			deb += sprintf_P(&buf[deb], PSTR("\r\n"));
		}
		else {  // generation GPX
			// <trkpt lat=" - 90.20169" lon=" - 180.67096"><ele>99999.40</ele><time>2002-02-10T16:56:22.00Z</time></trkpt>
			// 123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345
			// 103 + 2(cr/lf) + 1     106 caractères max pour une entrée
			if (sizeof(buf) - deb < 120) {
				// Serial.printf_P(PSTR("Ecriture buffer download %i\n"), deb);
				server_web.sendContent((const char *)buf, deb);
				deb = 0;
				buf[0] = 0;
			}
			// speed n'existe pas en GPX 1.1: géré par les boutons de sélection du format dans les preferences.
			//    <time> + ou - obligatoire pour OpenStreetMap
			deb += sprintf_P(&buf[deb], PSTR("<trkpt lat=\"%.6f\" lon=\"%.6f\">"), trackLigne.lat, trackLigne.lng);
			if (preferences_custom.logAltitude) deb += sprintf_P(&buf[deb], PSTR("<ele>%.2f</ele>"), trackLigne.altitude);
			//                         +++++++++++++++++++++   attention ecriture des centiseconde ??
			//<time>2002-02-10T21:01:29.250Z</time> Conforms to ISO 8601 specification for date/time representation.
			if (preferences_custom.logTime) {
				// On a que l'heure dans le fichier binaire . YYYY/MM/DD sont connus par le nom du fichier 2021-03-03_16-56
				deb += sprintf_P(&buf[deb], PSTR("<time>"));
				dateDuFichier.toCharArray(&buf[deb], 11);  // 10 carac + le null
				deb += 10;
				deb += sprintf_P(&buf[deb], PSTR("%T%02u:%02u:%02u.%02uZ</time>"), trackLigne.hour, trackLigne.minute, trackLigne.second, trackLigne.centisecond);
			}
			deb += sprintf_P(&buf[deb], PSTR("</trkpt>\r\n"));
		}
	}
	// fin fichier. vider le bufer et fermer le GPX
	if (strcmp(preferences_custom.traceFormat, "gpx") == 0) {
		// Bien terminer le fichier GPX
		// </trkseg></trk></gpx>
		// 12345678901234567890123
		//   21 + 1 + 2 (cr/lf) = 24 carca
		if (sizeof(buf) - deb < 26) {
			server_web.sendContent((const char *)buf, deb);
			deb = 0;
			buf[0] = 0;
		}
		deb += sprintf_P(&buf[deb], PSTR("</trkseg></trk></gpx>"));
	}
	server_web.sendContent((const char *)buf, deb);
	server_web.chunkedResponseFinalize();
	dataFile.close();

	return true;
}


//*************************************************************************************************
// sendChunkDebut()
//*************************************************************************************************
// 
//=================================================================================================
void sendChunkDebut(bool withTopMenu) {
	server_web.chunkedResponseModeStart(200, "text/html");
	server_web.sendContent_P(contentStyle);
	if (withTopMenu) server_web.sendContent_P(contentTopMenu);
}


//*************************************************************************************************
// handleCockpit()
//*************************************************************************************************
// 
//=================================================================================================
void handleCockpit() {
	// Reset of the timeout to turn off the Wifi
	wifiShutdownTime = millis() + preferences_custom.wifiTimeout * 1000;
	wifiShutdownCountdownRunning = true;
	#if defined(OPTION_STATS)
		statCockpit.T0 = millis();
		handleCockpitNotFound();
		calculerStat(true, &statCockpit);
	#else
		handleCockpitNotFound();
	#endif
}

//*************************************************************************************************
// handleCockpitNotFound()
//*************************************************************************************************
// 
//=================================================================================================
void handleCockpitNotFound() {
//	Serial.print("handleCockpitNotFound "); Serial.print (server.hostHeader()); Serial.print("  ");Serial.println (server.uri());
	sendChunkDebut(true);  // debut HTML style, avec topMenu
	server_web.sendContent_P(contentCockpit);
	server_web.sendContent_P(contentCockpitFooter);
	server_web.chunkedResponseFinalize();
}


//*************************************************************************************************
// handleFavicon()
//*************************************************************************************************
// Return the favicon
//=================================================================================================
void handleFavicon() {
  // Serial.print(F("Favicon ")); Serial.println(sizeof(favicon));
  server_web.send_P(200, "image/x-icon", favicon, sizeof(favicon));
}


//*************************************************************************************************
// handleGenerate204()
//*************************************************************************************************
// Redirect browser to a fictive URL.
// For Android captive portal. See https://gitlab.com/defcronyke/wifi-captive-portal-esp-idf
//=================================================================================================
void handleGenerate204() {
  // Serial.print(F("generate_204 ")); Serial.print (server.hostHeader()); Serial.print("  ");
  // Serial.println (server.uri());
  server_web.sendHeader("Location", String("http://") + String(uav_id), true);
  server_web.send(302, "text/plain", "Found");
}


//*************************************************************************************************
// handleNotFound()
//*************************************************************************************************
// 
//=================================================================================================
void handleNotFound() {
//	Serial.printf_P(PSTR("------------handleNotFound:%s  milli:%u\n"), server.uri().c_str(), millis());
	if (loadFromSPIFFS(server_web.uri())) return;	// Permit a direct file download
	#ifdef fs_STAT
		statNotFound.T0 = millis();
		handleCockpitNotFound();
		calculerStat(true, &statNotFound);
	#else
		handleCockpitNotFound();
	#endif
}


//*************************************************************************************************
// serverInit()
//*************************************************************************************************
// Init Web server entry points mapping to handle functions.
//=================================================================================================
void serverInit() {
	// For Android captive portal. See https://gitlab.com/defcronyke/wifi-captive-portal-esp-idf
	server_web.on("/", handleCockpitNotFound);
	server_web.on("/favicon.ico", handleFavicon);
	server_web.on("/redirect", handleGenerate204);      // For windows: www.msftconnecttest.com//redirect
	server_web.on("/generate_204", handleGenerate204);  // For Android: connectivitycheck.gstatic.com/generate_204
	server_web.onNotFound(handleNotFound);

	//
	server_web.on("/cockpit", handleCockpit);
	server_web.on("/readValues", handleReadValues);
	server_web.on("/giveMeTime", handleGiveMeTime);
	server_web.on("/razVMaxHMAx", handleRazVMaxHMAx);
	server_web.on("/spiff", handleGestionSpiff);
	server_web.on("/delete_", HTTP_POST, handleDelete);
	server_web.on("/fileInfo_", HTTP_POST, handleFileInfo);
	server_web.on("/formatage_", HTTP_GET, handleFormatage);  // danger mais un POST est diff a faire !!
	server_web.on("/optionsSysteme", handleOptionsSysteme);
	server_web.on("/optionLogProcess", handleOptionLogProcess);
	server_web.on("/optionGPSProcess", handleOptionGPSProcess);
	server_web.on("/optionPointAccesProcess", handleOptionPointAccesProcess);
	#if defined(OPTION_IBUS)
//		server_web.on("/optionIBUS", handleOptionIBUS);
	#endif
	#if defined(OPTION_GSM)
//		server_web.on("/optionSMSCommand", handleOptionSMSCommand);
	#endif
	server_web.on("/droneIDProcess", handleDroneIDProcess);
	server_web.on("/resetUsine", handleResetUsine);
	server_web.on("/reset", handleReset);
	#ifdef OPTION_STATS
//	server_web.on("/stat", handleStat);
//	server_web.on("/readStatistics", handleReadStatistics);
//	server_web.on("/statReset", handleResetStatistics);
	#endif
	#if defined(OPTION_RECEIVER)
//	server_web.on("/recepteur", handleRecepteur);
//	server_web.on("/recepteurRefresh", handleRecepteurRefresh);
//	server_web.on("/recepteurDetail", handleRecepteurDetail);
	#endif
	#if defined(OPTION_OTA)
//	fs_initServerOnOTA(server);  // server.on spécicifiques à OTA
	#endif
}


//*************************************************************************************************
// serverStart()
//*************************************************************************************************
// 
//=================================================================================================
void serverStart() {

//	LittleFS.end();
//	server_dns.stop();
//	server_web.stop();

	Serial.print(F("Start HTTP server : "));
	server_web.begin();
	serverInit();				// Initialize the server_web.on(...)
	Serial.print(F("done\n"));
	Serial.print(F("Start DNS server : "));
	// If DNSServer is started with "*" for domain name, it will reply with provided IP to all DNS request
//	dnsServer.setTTL(300);
	if (server_dns.start(SERVER_DNS_PORT, "*", ip_localaddress)) {
		Serial.print(F("done\n"));
	}
	else {
		Serial.print(F("failed !\n"));
		Serial.print(F("STOPPED => PLEASE REBOOT !!!"));
		#if defined(OPTION_BOARD_PIN_LED)
			digitalWrite(OPTION_BOARD_PIN_LED, HIGH); // turn the LED off
		#endif
		while (true) {		// Don't continue with invalid configuration
			delay(1000);
		}
	}

	Serial.print(F("Start filesystem : "));
	#ifdef ESP32
		if (LittleFS.begin(true)) {	// true = format if fail
			Serial.print(F("done\n"));
		}
		else {
			Serial.print(F("failed !\n"));
			Serial.print(F("STOPPED => PLEASE REBOOT !!!"));
			#if defined(OPTION_BOARD_PIN_LED)
				digitalWrite(OPTION_BOARD_PIN_LED, HIGH); // turn the LED off
			#endif
			while (true) {		// Don't continue with invalid configuration
				delay(1000);
			}
		}
	#else
		if (LittleFS.begin()) {	// ESP8266 format if fails by default
			Serial.print(F("done\n"));
		}
		else {
			Serial.print(F("failed !\n"));
			Serial.print(F("STOPPED => PLEASE REBOOT !!!"));
			#if defined(OPTION_BOARD_PIN_LED)
				digitalWrite(OPTION_BOARD_PIN_LED, HIGH); // turn the LED off
			#endif
			while (true) {		// Don't continue with invalid configuration
				delay(1000);
			}
		}
	#endif

	traceCleanup();
}
