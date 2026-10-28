# Feuille de route — Balise Identification UAV DGAC

Statut : ⬜ à faire · 🔶 en cours · ✅ fait · ❌ annulé

| # | id | Étape | Contenu | État |
|---|---|---|---|---|
| 1 | E1 | Cœur minimal | GPS + trame balise DGAC, validation sur USB | 🔶 |
| 2 | E5 | Assemblage matériel | Alimentation, signalisation, antenne et boîtier | 🔶 |
| 3 | E6 | Signalisation, énergie et conformité de l'émission | Mesure de batterie et alerte, conformité de l'émission, côté firmware | 🔶 |
| 4 | E2 | Portail de configuration | Serveur web et DNS captif dans `main.cpp` (Phase 2) | ⬜ |
| 5 | E3 | Conformité réglementaire WiFi | Pays et puissance via `esp_wifi_set_country()` | ⬜ |
| 6 | E4 | Extension multi-cibles | Environnements `esp32`, `esp32-s3`, `esp8266` | ⬜ |

## Étape 1 — Cœur minimal (GPS + trame balise DGAC)  [E1]

Le firmware compile (`pio run -e esp32-c3` → SUCCESS) et le périmètre « GPS + trame balise DGAC » est câblé dans `main.cpp`. Rien n'a encore été validé sur matériel physique.

1.1 ✅ [P0] **Interblocage d'émission corrigé.** `isReady()` exigeait `_frame_count > 0` alors que `main.cpp` conditionnait l'envoi à `isReady()` : aucune trame n'aurait jamais pu partir. `isReady()` exprime désormais l'aptitude à émettre (`_initialized && _home_set && checkGpsDataValidity()`), et `main.cpp` appelle `sendBeaconFrame()` sans garde redondante puisque `shouldSend()` filtre déjà.
1.2 ✅ [P6] Armement de `_gps_ready` corrigé dans `main.cpp` : `!_gps_ready && gpsManager.isReady()` au lieu de `!gpsManager.isReady() && gpsManager.hasValidFix()`, qui se déclenchait sur un fix incomplet et devenait impossible à satisfaire une fois le GPS pleinement valide.
1.3 ✅ [P1] **Firmware flashé le 13 août 2026.** `pio run -e esp32-c3 -t upload` → SUCCESS : 833 536 octets écrits à `0x10000`, hash vérifié, reset matériel. La carte enumère en `303a:1001` (Espressif USB JTAG/serial) sur `/dev/ttyACM0`. Deux constats au passage : le maintien du bouton BOOT est inutile sur C3, l'USB-JTAG natif passant seul en mode téléchargement ; et `lsusb` suffit à écarter un câble sans paires data, sans instrument de test.
1.4 ✅ [P2] **Boot validé le 13 août 2026.** `boot:0xf (SPI_FAST_FLASH_BOOT)`, bannière, EEPROM chargée, UART1 GPS ouvert à 9600 sur rxPin(0)/txPin(1), `WiFi beacon mode started: BIADUD_907069C1F484`, `System initialized successfully!` puis `--- Status Update ---` périodique. Aucune exception, aucun redémarrage. **Identifiant DGAC relevé sur la carte : `000MB1000000000000907069C1F484`** — c'est celui à enregistrer sur AlphaTango. Deux pièges d'outillage rencontrés : `pio device monitor` exige un vrai terminal (`termios.error` sinon), et ouvrir le port avec DTR/RTS actifs pilote EN et GPIO9 sur l'USB natif du C3, ce qui renvoie la puce en `boot:0x5 (DOWNLOAD)` — lire avec les lignes de contrôle relâchées. **Anomalie mineure à surveiller** : plusieurs cycles `AP_START`/`AP_STOP` pendant l'init WiFi avant stabilisation.
1.5 ✅ [P3] **Fix valide obtenu le 13 août 2026** : `fix=YES sats=12 hdop=0.65 alt=678.7`, `Home: SET`, `GPS: OK`. Critères DGAC largement remplis.

**Bug de parseur trouvé et corrigé au passage.** `NmeaParser::processSentence()` appelait `splitFields(_buffer + 1, ...)`, ne sautant que le `$` : le jeton de type restait dans le champ 0 et décalait tous les champs d'un cran. Le symptôme trompait — le firmware annonçait `NO FIX` alors que le récepteur avait un fix depuis le début, et la latitude affichée était en réalité l'heure UTC lue comme `ddmm.mmmm`, ce que révélait sa progression de 1/12 toutes les 5 s. Correction portée sur le découpage (saut du jeton de type entier) plutôt que sur les indices, pour que la numérotation du code reste celle de la norme NMEA que `parseGGA`/`parseRMC`/`parseGSA` documentent. Écarté : décaler les indices dans les trois fonctions, qui aurait fait diverger le code de sa propre documentation.

Deuxième défaut corrigé dans la foulée : les gardes exigeaient un nombre de champs exact (`< 15`, `< 13`, `< 17`) et rejetaient donc en silence toute trame dont les champs de queue sont omis. Elles ne réclament plus que les champs réellement lus.

Instrumentation ajoutée à `main.cpp` et conservée : le relevé périodique affiche octets bruts reçus, phrases décodées, checksums invalides, dernier type de phrase, puis fix, satellites, HDOP, altitude et position. C'est elle qui a permis de distinguer une ligne coupée d'un parseur en défaut.
1.6 ✅ [P4] **Émission validée le 13 août 2026** : `txerr=0`, `lasterr=0`, cinq trames de 143 octets en quinze secondes — une toutes les 3 s, cadence réglementaire. `esp_wifi_80211_tx` renvoie `ESP_OK`.

**Deuxième bug bloquant trouvé et corrigé.** `BeaconManager::generateBeaconFrame()` appelait `_drone_id_fr.generateBeaconFrame(buffer, 0)` et n'émettait donc que l'élément vendor-specific, sans en-tête 802.11 : la radio lisait le tag `0xDD` comme un frame control et refusait tout avec `ESP_ERR_INVALID_ARG` (258). Ajout de `buildBeaconHeader()` — trame de gestion, diffusion générale, source et BSSID à la MAC de l'interface, paramètres fixes, DS parameter set, élément SSID — puis greffe de l'IE derrière, ce à quoi le paramètre `start_from` de `DroneIdFr` servait depuis le début. Ajout de `getApMacAddress()` côté plateforme, la radio refusant toute autre adresse source, et passage de `en_sys_seq` à `true` pour que le SDK renseigne le numéro de séquence.

Instrumentation conservée : `getLastBeaconTxError()` garde le dernier code radio, `BeaconManager::getTxStatsString()` compte tentatives, refus de `shouldSend()`, trames vides et échecs. C'est ce triplet qui a distingué une trame jamais construite d'une trame refusée — le compteur `refused` très élevé est normal, `sendBeaconFrame()` étant appelé à chaque tour de boucle.
1.7 ✅ [P5] **Mapping LED vérifié sur matériel le 9 septembre 2026**, et trois défauts corrigés à cette occasion.

**Polarité inversée.** La carte câble sa LED en **actif bas** — anode au 3,3 V, cathode sur GPIO8 — alors que `main.cpp` appelait `ledManager.begin()` sans second argument, donc en actif haut. Tous les états s'affichaient à l'envers, la LED s'éteignant pendant l'émission. Corrigé par `begin(PIN_LED, false)`.

**Cadence plafonnée par l'appelant.** `ledManager.update()` était appelé sous une garde de 1 Hz, or c'est lui qui fait tourner les machines à états de clignotement et de pulsation. Toutes les cadences se trouvaient ramenées à celle de l'appelant : `BLINK_FAST` devenait indiscernable de `BLINK_SLOW` et `LED_PULSE` ne pouvait pas s'afficher. L'appel se fait désormais à chaque tour de boucle, `LedManager` gérant sa propre temporisation — la garde était redondante.

**Deux états de clignotement identiques.** `LED_BLINK_SLOW` et `LED_BLINK_FAST` partageaient la même branche et le même `_blink_interval_ms`, que `setState()` ne renseignait jamais — seule la méthode `blink()`, jamais appelée, le faisait. L'énumération promettait deux vitesses et n'en délivrait qu'une. `setState()` pose maintenant 1000 ms pour lent et 200 ms pour rapide.

**Vérifié sur matériel** : clignotement pendant la recherche, allumage fixe pendant l'émission. `LED_BLINK_FAST` reste transitoire par construction — `main.cpp` pose la position de référence dans la même itération que la détection du fix, donc l'état « fix acquis mais balise pas prête » ne dure qu'une fraction de seconde. Ce n'est pas un défaut.

**Rend `P15` sans objet** : la correction de polarité prévue à l'étape 3 est faite ici.

1.8 ✅ [P18] Émettre latitude et longitude, courantes et de départ, en **1e-5 degré** et non en 1e-7 — **fait le 12 octobre 2026.**

**Défaut** : trouvé par le récepteur de banc, qui lisait 4492,17°. La réécriture avait gardé le format interne en 1e-7 sans la division que le sketch de référence faisait à l'entrée (`set_current_position(int32_t)` × 1e-2). Un récepteur conforme situait la balise hors du globe.

**Source vérifiée** : annexe III de l'arrêté du 27 décembre 2019, sur Légifrance — « précision fixe à 5 décimales […] multipliée par 10⁵ », exemple 48,15278 → `0x00 49 79 AE`. Le sketch de référence, émetteur et récepteur, concorde.

**Retenu** : convertir à l'encodage seulement, par `DroneIdFr::toFrameCoordinate()` — division par 100, arrondi au plus proche avec le signe —, appliquée aux quatre champs dans `generateBeaconFrame()`. **Écarté** : passer tout le format interne en 1e-5, qui toucherait le calcul de distance des 30 m, `ConversionUtils` et les setters pour un gain nul, et perdrait deux décimales avant le calcul.

**Attesté** : neuf cas d'arrondi testés sur le poste (demis positifs et négatifs, zéro, ±180°, valeurs réelles) ; sur matériel, le récepteur lit 44,92168° / 4,70081°, courant et départ, octets `00 44 8B 88` pour la latitude.
1.9 ✅ [P19] Rapatrier le récepteur de banc sous `tools/recepteur/` — projet PlatformIO autonome, en-tête GPL, décodage du type 12 (`CODEINFO`) — **fait le 12 octobre 2026.**

**Rôle** : vérification indépendante de la trame émise. Il écoute le canal 1 en mode promiscuité et décode l'IE `6A:5C:35` d'après l'annexe de l'arrêté, pas d'après le code de la balise — c'est ce qui lui a fait trouver le défaut d'unité de `P18`. Usage : `pio run -d tools/recepteur -t upload --upload-port <port>`, puis lecture série à 115200 ; cartes repérées par le numéro de série USB, qui porte la MAC.

**Retenu** : un projet séparé, que le `pio run` racine ignore — la compilation de la balise reste intacte. **Écarté** : un second environnement dans le `platformio.ini` racine avec filtre de sources, qui mêlait deux programmes dans un même projet. **Écarté aussi** : le balayage `iw` du poste, impasse puisque le noyau ne garde que la dernière balise par BSSID et que celle du point d'accès écrase la trame DGAC.

**Attesté** : `pio run -d tools/recepteur` réussit depuis le dépôt ; flashé sur l'ancienne carte, il décode la balise — identifiant, position 44,92170° / 4,70084°, altitude, hauteur, départ, vitesse, route — et signale le type 12 comme réservé.

**Relevé à cette occasion** : l'annexe réserve les types 12 à 200 « à une utilisation postérieure », et la balise émet un type 12 hérité du sketch de référence. Hors périmètre de ce point, à inscrire au plan.

1.10 ✅ [P20] Retirer le TLV de type 12 (`CODEINFO`) de la trame — **fait le 12 octobre 2026.**

**Motif** : l'annexe de l'arrêté définit les types 1 à 11 et réserve « les types 12 à 200 […] à une utilisation postérieure ». Le type 12 venait du sketch de référence, où un contributeur l'avait ajouté ; la balise l'émettait à 0, sans que rien dans le firmware ne le renseigne.

**Retenu** : retrait complet — émission, `setCodeInfo()`/`getCodeInfo()` que rien n'appelait, membre `_codeinfo`, entrées des deux énumérations (`drone_id_fr.h`, `biadud_core.h`) et de `TLV_LENGTH`. La fin d'énumération porte « Types 12 to 200 are reserved by the order: never emit them », pour qu'il ne revienne pas par une reprise du sketch. **Écarté** : garder l'API en ne retirant que l'émission, qui laissait un champ mort invitant à le réactiver.

**Attesté** : le récepteur de banc décode une trame qui s'arrête au type 0B, sans type réservé ; la balise rapporte `lastsize=140` (143 auparavant), `txerr=0`.
1.11 ✅ [P21] Figer la route émise sous un seuil de vitesse sol — **fait le 12 octobre 2026.**

**Motif** : à l'arrêt, le GPS donne un cap aléatoire, et le u-blox le fige parfois lui-même sur une valeur tirée du bruit — 80° puis 50° relevés balise immobile.

**Retenu** : dans `BeaconManager::updateDroneIdFr()`, la route n'est mise à jour qu'en mouvement, avec hystérésis — suivi repris à **1,0 m/s** (`HEADING_SPEED_ON_MPS`), abandonné sous **0,5 m/s** (`HEADING_SPEED_OFF_MPS`), constantes dans `biadud_core.h`. À l'arrêt la dernière route mesurée est émise, 0 tant qu'aucune ne l'a été. Seuil tiré d'une mesure : 60 relevés sur 5 min, balise au repos à l'intérieur, vitesse apparente au plus 0,22 m/s (moyenne 0,06). Instrumentation conservée : le relevé périodique de `main.cpp` affiche vitesse et cap bruts. **Écarté** : un seuil unique, qui ferait clignoter la route autour de lui ; figer dans `DroneIdFr`, couche d'encodage, qui aurait dépendu de l'ordre d'appel de `setGroundSpeed()` et `setHeading()`.

**Attesté** au récepteur de banc, balise sur batterie : allers-retours le long d'un mur à peu près est-ouest, route ~100° à l'aller et ~285° au retour, demi-tours lus en une ou deux trames ; puis route figée à 293° pendant 40 s de pose dehors, et à **230° sur 106 trames, 5 min 15 de pose continue** à l'intérieur. Un premier essai au repos seul (79 trames à 0°) ne prouvait rien, le cap brut étant lui aussi resté à 0.

**Limite connue** : un saut de position du GPS assez fort pour dépasser 1 m/s — 3 m/s relevés en rentrant à l'intérieur — met la route à jour. Sans dérive à l'arrêt pour autant. Traitée par `P23`, filtre du récepteur.
1.12 ✅ [P23] Configurer le filtre du u-blox par `UBX-CFG-NAV5` dans `gps_ublox` : modèle dynamique « airborne <1g » au lieu de « portable » d'usine, et *static hold* autour de 0,5 m/s, pour que le récepteur rejette lui-même les sauts de position et de vitesse (3 m/s relevés le 12 octobre à l'intérieur) et fige la position à l'arrêt. Aucun lissage de position côté firmware : il retarderait la position émise en vol. Un médian sur 3 mesures de vitesse et de route ne viendrait qu'en complément, si le récepteur ne suffit pas. Fait quand le récepteur accuse la configuration (`UBX-ACK-ACK`) et que, à marche identique, le récepteur de banc montre moins de sauts et une position stable à l'arrêt. **Étendu le 23 octobre 2026 (décision de l'utilisateur) :** la position de départ n'est plus posée dès que `isReady()` passe, mais après un fix stabilisé — HDOP sous 2 pendant 10 s consécutives —, pour éviter l'altitude de départ prise sur un début de fix (hauteur de −37 m relevée à l'arrêt). Fait aussi quand, à l'arrêt, la hauteur émise reste proche de 0 — **fait le 25 octobre 2026.**

**Attesté sur matériel.** Le récepteur accuse la trame `UBX-CFG-NAV5` (`UBX-ACK-ACK`, décodé par un vrai lecteur d'accusé et non plus supposé). À l'arrêt, comparaison stricte du 23 octobre, même endroit, 3 minutes, 36 relevés — sans filtre (portable, seuil nul) : vitesse moyenne 0,068 m/s, dérive 2,6 m, altitude sur 8 m ; avec filtre : 0,003 m/s, 0,4 m, 0,1 m. Position de départ posée après HDOP < 2 pendant 10 s : la hauteur de −37 m a disparu (0 m à l'arrêt, vérifiée 90 s au récepteur). En marche, au pas, le 25 octobre : 82 trames en 250 s, aucun saut de position, route à moins de 30° du cap réel sur 24 fenêtres de 12 s sur 29 (les 5 autres chevauchent un demi-tour), trajet jugé cohérent sur la carte par l'utilisateur.

**Retenu.** Modèle airborne <1g et *static hold* à 50 cm/s avec sortie à 10 m (valeur choisie sans l'avoir lue dans le manuel u-blox ; seule source la définition de message de l'interface ROS, structure, offsets et unités). Position de départ attendue jusqu'à un fix stabilisé, avec repli à 60 s mesuré depuis le premier fix pour que la balise émette toujours. Seuils de la route abaissés à 0,6 m/s (enclenchement) et 0,4 m/s (arrêt), au lieu de 1,0 et 0,5, parce qu'un drone peut avancer à l'allure d'un pas : le *static hold* ramène la vitesse au repos sous 0,10 m/s, ce qui libère la marge.

**Écarté.** Tout lissage côté firmware, par médiane de 3 à 7 trames : l'écart-type de la hauteur passe seulement de 4,1 à 3,8 m, et la hauteur serait retardée en vol. Sauvegarde de la configuration en flash du récepteur : elle est renvoyée à chaque démarrage.

**À savoir.** Le critère « moins de sauts de vitesse » n'a pas pu être démontré : les 3 m/s relevés à l'intérieur le 12 octobre ne se sont reproduits ni avec ni sans filtre. La **hauteur en mouvement reste imprécise** (dérive de 0 à −9 m en marche, puis saut de +11 m à l'arrêt), erreur verticale du GPS que le filtre ne traite pas ; non corrigée, la balise étant un moyen de contrôle et non une mesure précise. Pistes laissées ouvertes : baromètre sur `I²C` (GPIO7 et GPIO10 libres), masque d'élévation et système satellite supplémentaire à mesurer par comparaison avec/sans, essai à découvert loin d'un mur. Les deux essais de marche ont eu lieu près d'un mur de la maison.

## Étape 2 — Assemblage matériel et boîtier  [E5]

Périmètre matériel arrêté le 12 août 2026, détaillé dans la section « Matériel embarqué » d'`AGENTS.md` : nomenclature, brochage, câblage, pièges d'assemblage.

**Alimentation et charge validées** (5 septembre 2026). Le buck-boost est monté et réglé, le chargeur MCP73871 a mené un cycle complet, la thermistance est en place. Le TP4056 initialement prévu a été abandonné, ses deux exemplaires ayant leur chemin de décharge hors service — détail dans `AGENTS.md`. Reste à connecter l'ESP32-C3 au régulateur, puis à câbler la signalisation et à dessiner le boîtier.

2.1 ✅ [P7] **Sortie du buck-boost réglée le 30 août 2026** : 3,28 V à vide, 3,29 V sous 100 Ω, soit 0,6 % du nominal.

Le pad 3,3 V d'usine donnait **3,45 V**. La référence interne du clone XL63020 est à ~0,526 V au lieu des 0,500 V du TPS63020 authentique, et les ponts sérigraphiés ont été calculés pour la puce d'origine — valeur utilisable, l'ESP32-C3 admettant 3,0 à 3,6 V, mais avec seulement 150 mV de marge. Multimètre et oscilloscope concordants, l'instrument est hors de cause.

Correction portée sur le **quatrième chemin de contre-réaction**, que la carte prévoit pour cet usage : un pont de 0 Ω et un emplacement libre où poser sa propre valeur, la résistance basse de ~180 kΩ étant commune aux quatre branches. Le pont de 0 Ω ressemble à un fusible et se mesure comme lui, mais la branche ne transporte que ~2,7 µA — rien ne peut s'y ouvrir.

**Écarté** : reprendre les résistances du chemin 3,3 V d'usine, qui aurait supprimé le repli en cas de soudure ratée sur un composant de cette taille. **À ne jamais faire** : laisser deux pads pontés, les branches se mettant alors en parallèle et la sortie montant.
2.2 ✅ [P8] **Chargeur MCP73871 en service le 5 septembre 2026** : cycle complet mené à son terme, courant décroissant en fin de charge puis **4,20 V** en tension finale — donc la version standard du circuit, parmi les quatre existantes (4,10 · 4,20 · 4,35 · 4,40 V).

Réglages relevés : `PROG1` porte 2 kΩ d'usine, soit exactement les 500 mA visés (`I(mA) = 1000 / R(kΩ)`) ; `SEL`, `PROG2`, `CE` et `TE` sont tous à l'état haut, donc mode adaptateur secteur et minuteur désactivé ; le 10 kΩ fixe de `THERM` a été remplacé par la NTC 10 kΩ B3950, ce qui donne une fenêtre de charge d'environ +6 à +42 °C. Condensateur de 10 µF polarisé posé sur `BATT`, le datasheet exigeant 4,7 µF minimum.

**Le TP4056 initialement prévu a été abandonné.** Ses deux exemplaires ont leur chemin de décharge hors service : `OUT-` flotte au lieu d'être collé à `B-`, et la tension s'effondre sous 100 Ω. Diagnostic mené en remontant de la cellule — celle-ci débite, ses soudures aussi, `IN+`↔`IN-` à 5 V — ce qui isole la section protection du module. Le MCP73871 apporte en prime le chemin d'alimentation, qui supprime la règle « interrupteur sur OFF pendant la charge ».

**Un module détruit le 2 septembre 2026** en lançant le mode « test de charge rapide » du wattmètre avec la carte branchée : le testeur négocie jusqu'à 20 V, le maximum absolu du circuit est de 7,0 V. Schottky d'entrée en court-circuit, trois LED d'état grillées. Le reste du montage a été épargné parce que l'interrupteur était sur OFF.

**Deux mesures inutiles à retenter** : l'ohmmètre sur les broches de configuration donne une valeur d'artefact identique pour toutes — c'est la tension, carte alimentée, qui dit l'état logique. Et le condensateur de 4700 µF fourni avec la carte se monte sur l'entrée, pas sur `BATT` : ne pas le monter du tout, l'appel de courant à la connexion est excessif.
2.3 ✅ [P9] Câbler la chaîne d'alimentation : cellule → interrupteur sur le `+` → buck-boost → `3V3` de la carte et `VCC` du GPS, avec le condensateur réservoir de 220 µF polymère et son 100 nF au plus près — **fait le 12 octobre 2026.**

**Attesté sur matériel, batterie seule** (USB débranché, interrupteur sur ON) : 3,260 V sur `3V3` pendant l'émission contre 3,261 V au repos, 2,038 V sur GPIO3 (cellule ~4,08 V) ; trames DGAC reçues par le récepteur de banc toutes les ~3 s à −52/−55 dBm, identifiant `000MB1000000000000907069C29644`. Buck-boost : 3,28 V à vide, 3,29 V sous 100 Ω. Découplage 220 µF polymère 10 V + 100 nF céramique au plus près.

**Retenu** : alimentation de la carte par sa pin `3V3`, jamais `5V` — une première soudure sur `5V` aurait renvoyé le VBUS sur tout le rail, relevée le 9 octobre avant mise sous tension. Interrupteur mécanique sur le `+`, `EN` du XL63020 relié à demeure à `VIN` : coupure franche, consommation nulle à l'arrêt.

**Incident** : première ESP32-C3 abandonnée après deux défauts de reprise au fer — `EN` court-circuité par un résidu de flux (résolu au nettoyage), puis pastille `3V3` déconnectée du réseau interne. Montage refait sur une carte neuve ; l'identifiant DGAC a changé avec la MAC. Détail et leçons de diagnostic dans `AGENTS.md`.
2.4 ✅ [P10] Câbler signalisation et entrées : LED verte déportée sur GPIO8, LED jaune sur GPIO6, LED rouge fixe sur le rail 3,3 V, bouton sur GPIO2, pont de 2 × 220 kΩ sur GPIO3 — **fait le 12 octobre 2026**, sur la carte neuve.

**Attesté sur matériel** : verte et bleue de la carte clignotent ensemble sur GPIO8 sans fix, verte fixe pendant l'émission ; rouge du rail allumée ; jaune sur GPIO6 éteinte, attendu puisqu'aucun code ne la pilote encore (`P14`) ; pont mesuré à 2,023 V sur GPIO3 pour une cellule à ~4,05 V ; bouton GPIO2 tenu au redémarrage → `Factory reset requested via pin!`, démarrage normal.

**Retenu** : les trois LED en **actif bas**, anode au 3V3 et 1 kΩ — comme la LED bleue de la carte, restée soudée sur GPIO8, sans quoi verte et bleue s'allumeraient à contretemps et `begin(PIN_LED, false)` serait faux pour l'une des deux. Pont sur **`BATT` du MCP73871, avant l'interrupteur** : sur `LOAD` il lirait l'USB dès qu'il est branché, et rien interrupteur ouvert ; son courant permanent, ~9,5 µA, est négligeable. **Écarté** : la LED RGB, trois GPIO pour rien que les motifs ne disent déjà.

**À savoir** : GPIO2 est une broche de configuration du démarrage, mais le tenir bas au reset ne bloque pas le démarrage, vérifié. Le bouton n'est lu qu'une fois, ~1 s après le reset : il faut le tenir pendant le redémarrage.
2.5 ✅ [P11] Confirmer l'antenne externe par mesure du niveau reçu — **fait le 12 octobre 2026.**

**Attesté par mesure**, récepteur de banc et boîte métallique, sur 10 trames DGAC à chaque fois : référence à l'air libre −42 dBm (médiane) ; carte de la balise enfermée, antenne externe dehors −41,5 dBm, soit aucun écart au-delà de ±3 dB ; contrôle, récepteur entier enfermé −52 dBm, soit ~10 dB d'atténuation de la boîte. Si la balise rayonnait par son antenne intégrée, l'enfermer aurait coûté ces 10 dB : elle rayonne par l'externe. Concorde avec l'examen de la carte (8 octobre) et l'annonce du vendeur pour ce modèle (sélecteur sur l'externe, ne pas l'utiliser sans antenne).

**Retenu** : enfermer la carte, antenne dehors — l'émetteur garde sa charge normale. **Écarté** : débrancher l'antenne de la balise, interdit puisqu'émettre à 20 dBm sans charge use l'étage de puissance ; enfermer l'antenne seule, qui la désaccorde et renvoie de la puissance vers l'émetteur ; l'ohmmètre sur la piste RF, que condensateurs série et selfs à la masse rendent illisible. **Réserves** : marge de 10 dB seulement, la boîte fuyant par sa fente ; le déplacement du récepteur pour le contrôle rend ces 10 dB approximatifs. Ni l'une ni l'autre ne renverse le résultat.
2.6 🔶 [P12] Dessiner et imprimer le boîtier — PETG non chargé en carbone, ouvertures USB, interrupteur, bouton et LED, antenne GPS vers le ciel

## Étape 3 — Signalisation, énergie et conformité de l'émission  [E6]

Volet firmware des décisions matérielles de l'étape 2. Règle de conformité qui encadre tout ce qui suit : la balise doit émettre pendant tout le vol, donc une batterie faible se signale mais n'interrompt jamais l'émission.

3.1 ✅ [P13] Définir `PIN_BATTERY` dans `biadud_platform.h` et écrire `hardware/battery_monitor.{h,cpp}` : lecture sur ADC1, moyennage de plusieurs échantillons, conversion en tension cellule — **fait le 23 octobre 2026.**

**Attesté sur matériel** : le firmware lit 4 094 à 4 098 mV sur la cellule (broche 2 047 à 2 049 mV) contre 4,088 V au multimètre sur `BATT`, soit +8 mV (0,2 %), pour un critère de ±50 mV. Dispersion de 2 à 4 mV en émission. Code au commit `1f72a2c` : `PIN_BATTERY 3` sur C3 et `-1` ailleurs, ADC1 atténuation 11 dB (0 à 2 500 mV sur C3, doc Espressif), 16 échantillons par `analogReadMilliVolts`, une lecture par seconde, ligne `Battery:` du relevé périodique.

**Retenu** : un **100 nF céramique entre GPIO3 et `GND`**, au plus près de la broche. Sans lui, l'ADC lisait 40 mV trop bas sur la broche (−80 mV sur la cellule, le 12 octobre) : les ~110 kΩ du point milieu ne rechargent pas assez vite la capacité d'échantillonnage. Le multimètre lisait le pont juste (2,031 V, cohérent avec sa charge de 10 MΩ), ce qui mettait l'ADC en cause et non le pont. Le 10 µF existant sur `BATT`, en haut du pont, n'y faisait rien.

**Écarté** : la constante d'étalonnage logicielle, qui aurait masqué la cause et dérivé avec la tension ; un condensateur électrolytique, dont une fuite de l'ordre du µA fausserait la lecture d'une centaine de mV à travers 110 kΩ.
3.2 ✅ [P14] Ajouter la seconde instance `LedManager` (`ledWarning`, GPIO6) et poser le mapping des LED : verte = avancement (clignote en attendant le premier fix ou après sa perte, fixe quand la balise émet), jaune = pannes (éteinte, clignotante si GPS muet ou WiFi KO, fixe si les deux). « GPS muet » : aucune phrase NMEA valide depuis plus de 5 s. « WiFi KO » : radio non initialisée, ou 3 échecs d'émission consécutifs. Test de démarrage : verte et jaune allumées 1 s. Fait quand chaque état est provoqué et observé sur matériel (GPS débranché, WiFi non initialisé), et que l'émission continue avec la dernière position quand le GPS se tait — **fait le 27 octobre 2026.**

**Attesté sur matériel** : au démarrage, verte et jaune allumées 1 s (le test a révélé un défaut de câblage sur chacune, réparés depuis). GPS branché : verte clignotante jusqu'au fix, fixe ensuite, jaune éteinte. GPS débranché en marche : jaune clignotante en moins de 5 s, verte clignotante. GPS rebranché : retour à verte fixe. Perte de fix provoquée par un blindage fermé (aluminium et boîte métallique, antenne vers le bas) : verte clignotante pendant la perte, fixe au retour, jaune restée éteinte puisque le lien GPS parlait toujours ; compteur de trames figé à 408 puis repris (424, 426). Firmware compilé et flashé le 26 octobre.

**Retenu** : `ledWarning` sur `PIN_LED_WARNING` (6 sur C3, -1 ailleurs), actif bas. `updateIndicators()` évalue les deux LED à chaque tour de boucle et n'appelle `setState()` qu'au changement. Verte = avancement : fixe si WiFi bon, GPS parlant et `isReady()`, clignotement lent sinon, éteinte si le WiFi est perdu. Jaune = pannes : éteinte, clignotement rapide si GPS muet ou WiFi perdu, fixe si les deux. GPS muet : aucune phrase décodée depuis 5 s (`GPS_SILENCE_MS`), mesuré depuis l'initialisation. WiFi perdu : radio non initialisée ou 3 refus d'émission consécutifs (`WIFI_TX_FAIL_LIMIT`, `BeaconManager::getConsecutiveTxErrors()`). Test de démarrage de 1 s (`LED_TEST_MS`). L'émission n'est pas touchée par ces signaux.

**Écarté** : une jaune sur perte de fix, qui aurait clignoté à chaque acquisition normale du premier fix ; faire expirer les données GPS pour que `isReady()` retombe, ce qui aurait coupé l'émission dès que le GPS se tait ; l'état de pulsation de la verte, supprimé.

**Non éprouvé** : le WiFi perdu, jamais provoqué sur la carte et appuyé par la seule lecture du code ; l'émission pendant GPS muet, non confirmée au récepteur de banc.

**Constaté hors périmètre** : à la perte de fix la balise cesse d'émettre (`shouldSend()` exige 4 satellites et HDOP < 5), ce qui contredit la règle de ne jamais interrompre l'émission en vol. Ce que l'arrêté exige dans ce cas n'a pas été lu ; inscrit au plan comme `P25`.
3.3 ❌ [P15] **Sans objet depuis le 9 septembre 2026.** Le point séparait l'observation de la polarité, prévue à l'étape 1, de sa correction, prévue ici — découpage fondé sur l'idée que les deux se feraient à des moments différents.

En pratique elles étaient inséparables : `P5` demandait de confirmer que le comportement de la LED correspond au mapping documenté, ce qui exigeait de corriger la polarité pour pouvoir l'observer. La carte câble sa LED en actif bas, `main.cpp` appelle désormais `begin(PIN_LED, false)`, et le livrable existe — commit `30a6ca6`. Le travail a donc eu lieu, simplement ailleurs.

Annulé plutôt que supprimé, pour garder la trace du découpage initial et de la raison qui l'a rendu caduc.
3.4 ✅ [P25] Émission sur perte de fix : lire dans l'arrêté du 27 décembre 2019 (annexes) et la notice du ministère ce qui est exigé quand la position n'est plus disponible en vol, puis décider entre continuer à la cadence de 3 s avec la dernière position connue, une autre valeur prévue par le texte, ou le silence. Aujourd'hui `shouldSend()` exige 4 satellites et HDOP < 5, et la balise se tait (constaté le 27 octobre, compteur figé à 408 sous blindage). Préciser au passage le cas d'avant le premier fix, où rien n'est émis. Fait quand la règle est citée depuis le texte, la décision archivée, et que sous blindage le récepteur de banc reçoit ce que la décision prévoit — **fait le 28 octobre 2026.**

**Règle citée** — arrêté du 27 décembre 2019, article 2, I, 9° (Légifrance, lu le 27 octobre) : « L'émission du message de signalement s'effectue dès le décollage de l'aéronef circulant sans personne à bord jusqu'à l'atterrissage de celui-ci et de manière périodique », et pour une balise non native « soit sur le canal Wifi numéro 6 au premier des termes temporels ou spatiaux échus suivants : 2 envois sont séparés d'au plus 3 secondes ; 2 envois sont séparés d'au plus 30 mètres. » Notice du ministère (4 juin 2020) : signalement « diffusé à tout instant du vol » ; un dispositif qui « n'émet pas toutes les informations attendues » rend l'aéronef « non coopératif ». **Aucun des deux textes ne traite la perte de position, ni ne fixe de seuil de satellites ou de HDOP**, ni de valeur « position indisponible ».

**Retenu** (décision de l'utilisateur, 28 octobre) : l'émission commence à la position de départ stabilisée (règle de `P23` inchangée) — au sol avant le décollage rien n'oblige à émettre, et le champ du point de décollage, obligatoire, ne peut pas être rempli avant ; ensuite elle ne s'arrête plus, sans limite de durée. Sans fix vivant, la dernière position vivante est renvoyée, vitesse 0, route figée. `shouldSend()` et `isReady()` n'exigent plus qu'une position connue (`_position_known`) ; `hasLivePosition()` (fix et coordonnées non nulles) décide entre position vivante et figée (`isPositionFrozen()`). Un GPS muet garde son dernier `has_fix` : `main.cpp` le force à faux quand `isGpsMute()`. Les seuils 4 satellites / HDOP ≤ 5 restent des critères de qualité pour poser la position de départ ; les commentaires qui les disaient « DGAC » sont corrigés. Mapping des LED arrêté avec l'utilisateur, deux LED indépendantes : **verte = ce qui est émis** (éteinte rien, rapide stabilisation, lente position figée, fixe position vivante) ; **jaune = ce qui ne va pas** (éteinte rien, lente initialisation jusqu'à la première phrase GPS, rapide recherche du fix, fixe GPS muet ou WiFi perdu, double éclat batterie faible réservé à `P16`). Il remplace celui de `P14`. Rouge laissée câblée en dur sur le 3V3.

**Écarté** : le silence sur perte de fix (contraire au texte) ; une limite de durée à la répétition de la position (rien dans le texte, et la notice préfère une information à aucune) ; un point de décollage provisoire posé au premier fix (hauteur fausse jusqu'à la stabilisation) ; piloter la LED rouge sur GPIO7 (elle perdrait le témoin indépendant du firmware, et GPIO7/10 sont les deux dernières broches libres) ; la verte en clignotement pour la perte de fix, qui aurait dit « pas prête » alors que la balise émet.

**Attesté sur matériel** le 28 octobre (balise sur USB, interrupteur sur OFF, récepteur de banc sur le canal 1) : démarrage — verte et jaune 1 s, jaune lente, jaune rapide, verte rapide en stabilisation, verte fixe et émission, confirmé par l'utilisateur. GPS sous blindage : position `FROZEN` dès 16:45:20, **2 785 trames identiques en 2 h 20** jusqu'au retour du fix, vitesse 0, route figée à 151°, verte lente et jaune rapide. GPS débranché : passage en `FROZEN` sous 5 s, trames continues, jaune fixe et verte lente. GPS rebranché : jaune rapide puis éteinte, verte fixe, position `LIVE`. Sur l'ensemble, 2 892 trames sans erreur radio, intervalles de 3 s au récepteur (écarts de 4 s toujours compensés par un de 2 s : arrondi de l'horodatage à la seconde).

**Constaté** : sous blindage, le fix affaibli dérive d'environ 17 m dans les 12 s avant de lâcher ; c'est cette dernière position, déjà fausse, qui est figée. Le compteur `refused` de `getTxStatsString()` compte aussi chaque tour où les 3 s ne sont pas écoulées : il ne mesure pas un refus d'émission.

**Hors périmètre, relevé ici** : la balise émet sur le canal 1 alors que l'arrêté impose le canal 6 — inscrit comme `P26` ; et la cadence attend 3 000 ms plus un tour de boucle, donc dépasse légèrement les « au plus 3 secondes » (3,1 s mesuré le 25 octobre) — le sketch de référence anticipe d'une période GPS ; point à inscrire, non tranché.
3.5 🔶 [P26] Émettre sur le canal WiFi 6, imposé par l'arrêté du 27 décembre 2019 (article 2, I, 9°) à une balise non native de l'aéronef ; aujourd'hui `WIFI_AP_CHANNEL` vaut 1 dans `main.cpp`. Passer le récepteur de banc (`tools/recepteur/`) sur le canal 6 en même temps. Fait quand le récepteur de banc, réglé sur le canal 6, reçoit les trames à la cadence de 3 s, et qu'il ne reçoit plus rien sur le canal 1
3.6 ⬜ [P27] Tenir la cadence à au plus 3 s entre deux envois, comme l'exige l'arrêté (article 2, I, 9°) : aujourd'hui l'envoi part à 3 000 ms plus un tour de boucle, soit jusqu'à 3,1 s mesurées le 25 octobre ; anticiper d'une marge, à la manière du sketch de référence qui avance d'une période GPS (viser ~2,8 s), dans `DroneIdFr`. Fait quand, sur au moins 100 trames, aucun intervalle ne dépasse 3,0 s, mesuré par les horodatages de la balise elle-même (le récepteur arrondit à la seconde)
3.7 ⬜ [P16] Implémenter l'alerte de batterie faible : seuil autour de 3,4 V, signalement par la LED jaune en double éclat (choisi le 28 octobre) — motif à ajouter à `LedManager`, aucun `LedState` ne le fait aujourd'hui —, et décider ce qui l'emporte entre batterie faible et jaune fixe (GPS muet ou WiFi perdu). Sans jamais interrompre l'émission
3.8 ⬜ [P17] Mesurer l'autonomie réelle et la confronter à l'estimation de 5 à 8 heures
3.9 ⬜ [P24] Journal d'événements en NVS : tampon circulaire de 64 entrées de 8 octets (temps depuis le démarrage, numéro de démarrage, type, valeur), écrit aux transitions seulement et coalescé pour limiter l'usure. Événements : fix acquis et perdu (satellites), GPS muet et revenu, WiFi KO et revenu (dernier code radio), batterie faible (tension, avec P16), et cause de chaque redémarrage (`esp_reset_reason()`). Affichage sur le port série au démarrage. Fait quand une coupure d'alimentation, un GPS débranché et un fix perdu y sont retrouvés après redémarrage, et que le tampon ne dépasse jamais sa taille après plus de 64 événements

**Écarté** : l'`EEPROM` du projet, qui réécrit une page entière à chaque écriture ; un journal à historique long, seule la cause récente compte ; l'écriture à chaque trame.
3.10 ⬜ [P28] Écrire la notice d'utilisation dans le `README` : table des LED (verte = ce qui est émis, jaune = ce qui ne va pas, rouge = alimentation), règle « ne pas décoller tant que la verte n'est pas fixe » (conséquence de `P25`), précautions USB, interrupteur et charge, lecture de l'identifiant au démarrage sur le port série. Fait quand la notice est commitée et que chaque état de LED qu'elle décrit a été observé sur matériel

## Étape 4 — Portail de configuration (Phase 2)  [E2]

*(non détaillée)*

Réintégrer `network/web_server.{h,cpp}` et `network/dns_server.{h,cpp}` dans `main.cpp` — le code existe déjà mais n'est ni inclus ni appelé. Voir la section « Phase 2 » d'`AGENTS.md` pour ce qui a été retiré du build minimal.

## Étape 5 — Conformité réglementaire WiFi (pays et puissance)  [E3]

Remplacer `WiFi.setCountry()`, inexistant dans le core Arduino-ESP32 2.0.x, par l'équivalent ESP-IDF `esp_wifi_set_country()` et sa struct `wifi_country_t`. Détaillée partiellement le 12 octobre 2026 : le reste se découpera au besoin.

5.1 ⬜ [P22] Déclarer le pays `FR` par `esp_wifi_set_country()` — canaux 1 à 13, puissance plafonnée à 20 dBm —, la balise du point d'accès annonçant aujourd'hui `CN`. Ajouter au récepteur de banc le décodage de l'élément Country. Fait quand le récepteur lit `FR`

## Étape 6 — Extension multi-cibles  [E4]

*(non détaillée)*

Compléter les environnements `esp32`, `esp32-s3` et `esp8266` dans `platformio.ini`, aujourd'hui de simples stubs `; TODO`. Seul `esp32-c3` est actif.
