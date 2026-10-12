# Feuille de route — Balise Identification UAV DGAC

Statut : ⬜ à faire · 🔶 en cours · ✅ fait · ❌ annulé

| # | id | Étape | Contenu | État |
|---|---|---|---|---|
| 1 | E1 | Cœur minimal | GPS + trame balise DGAC, validation sur USB | 🔶 |
| 2 | E5 | Assemblage matériel | Alimentation, signalisation, antenne et boîtier | 🔶 |
| 3 | E6 | Signalisation et énergie | Mesure de batterie et alerte, côté firmware | ⬜ |
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
1.9 ⬜ [P19] Rapatrier le récepteur de banc sous `tools/recepteur/` — projet PlatformIO autonome, en-tête GPL, décodage du type 12 (`CODEINFO`). Fait quand il compile depuis le dépôt et décode une trame reçue

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
2.5 ⬜ [P11] Confirmer l'antenne externe par mesure du niveau reçu, antenne branchée puis débranchée — le cavalier est déjà sur l'externe d'usine ; le récepteur de banc donne le RSSI
2.6 ⬜ [P12] Dessiner et imprimer le boîtier — PETG non chargé en carbone, ouvertures USB, interrupteur, bouton et LED, antenne GPS vers le ciel

## Étape 3 — Signalisation et supervision d'énergie  [E6]

Volet firmware des décisions matérielles de l'étape 2. Règle de conformité qui encadre tout ce qui suit : la balise doit émettre pendant tout le vol, donc une batterie faible se signale mais n'interrompt jamais l'émission.

3.1 ⬜ [P13] Définir `PIN_BATTERY` dans `biadud_platform.h` et écrire `hardware/battery_monitor.{h,cpp}` : lecture sur ADC1, moyennage de plusieurs échantillons, conversion en tension cellule
3.2 ⬜ [P14] Ajouter la seconde instance `LedManager` (`ledWarning`, GPIO6) et définir le mapping des signaux entre LED verte et LED jaune
3.3 ❌ [P15] **Sans objet depuis le 9 septembre 2026.** Le point séparait l'observation de la polarité, prévue à l'étape 1, de sa correction, prévue ici — découpage fondé sur l'idée que les deux se feraient à des moments différents.

En pratique elles étaient inséparables : `P5` demandait de confirmer que le comportement de la LED correspond au mapping documenté, ce qui exigeait de corriger la polarité pour pouvoir l'observer. La carte câble sa LED en actif bas, `main.cpp` appelle désormais `begin(PIN_LED, false)`, et le livrable existe — commit `30a6ca6`. Le travail a donc eu lieu, simplement ailleurs.

Annulé plutôt que supprimé, pour garder la trace du découpage initial et de la raison qui l'a rendu caduc.
3.4 ⬜ [P16] Implémenter l'alerte de batterie faible : seuil autour de 3,4 V, signalement par la LED jaune, sans jamais interrompre l'émission
3.5 ⬜ [P17] Mesurer l'autonomie réelle et la confronter à l'estimation de 5 à 8 heures

## Étape 4 — Portail de configuration (Phase 2)  [E2]

*(non détaillée)*

Réintégrer `network/web_server.{h,cpp}` et `network/dns_server.{h,cpp}` dans `main.cpp` — le code existe déjà mais n'est ni inclus ni appelé. Voir la section « Phase 2 » d'`AGENTS.md` pour ce qui a été retiré du build minimal.

## Étape 5 — Conformité réglementaire WiFi (pays et puissance)  [E3]

*(non détaillée)*

Remplacer `WiFi.setCountry()`, inexistant dans le core Arduino-ESP32 2.0.x, par l'équivalent ESP-IDF `esp_wifi_set_country()` et sa struct `wifi_country_t`.

## Étape 6 — Extension multi-cibles  [E4]

*(non détaillée)*

Compléter les environnements `esp32`, `esp32-s3` et `esp8266` dans `platformio.ini`, aujourd'hui de simples stubs `; TODO`. Seul `esp32-c3` est actif.
