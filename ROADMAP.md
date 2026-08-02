# Feuille de route — Balise Identification UAV DGAC

Statut : ⬜ à faire · 🔶 en cours · ✅ fait · ❌ annulé

| # | id | Étape | Contenu | État |
|---|---|---|---|---|
| 1 | E1 | Cœur minimal | GPS + trame balise DGAC : code écrit et compilable, reste la validation matérielle | 🔶 |
| 2 | E2 | Portail de configuration | Réintégrer le serveur web et le DNS captif dans `main.cpp` (Phase 2) | ⬜ |
| 3 | E3 | Conformité réglementaire WiFi | Pays et puissance d'émission via `esp_wifi_set_country()` | ⬜ |
| 4 | E4 | Extension multi-cibles | Compléter les environnements `esp32`, `esp32-s3`, `esp8266` | ⬜ |

## Étape 1 — Cœur minimal (GPS + trame balise DGAC)  [E1]

Le firmware compile (`pio run -e esp32-c3` → SUCCESS) et le périmètre « GPS + trame balise DGAC » est câblé dans `main.cpp`. Rien n'a encore été validé sur matériel physique.

1.0 ⬜ [P0] **Bloquant — interblocage d'émission.** `main.cpp:219` n'appelle `beaconManager.sendBeaconFrame()` que si `beaconManager.isReady()`, or `BeaconManager::isReady()` (`beacon_manager.cpp:166`) exige `_frame_count > 0`, compteur qui n'est incrémenté que par un envoi réussi. Aucune trame ne partira jamais. `sendBeaconFrame()` filtre déjà lui-même via `shouldSend()` (initialisé + home posée + GPS valide) : soit appeler `sendBeaconFrame()` sans garde, soit faire porter à `isReady()` l'aptitude à émettre (`_initialized && _home_set`) et non l'historique d'émission. À corriger avant tout test matériel, sinon les points P4 et P5 échoueront pour cette seule raison.
1.0b ⬜ [P0b] `main.cpp:197` teste `!gpsManager.isReady() && gpsManager.hasValidFix()` pour armer `_gps_ready` — la lecture attendue est `!_gps_ready && ...` (`isReady()` est le critère DGAC complet : fix + 4 satellites + HDOP + altitude). En l'état le drapeau se lève sur un fix incomplet et cesse de pouvoir se lever une fois le GPS pleinement valide.
1.1 ⬜ [P1] Flasher le firmware sur l'ESP32-C3 physique — `~/.platformio/penv/bin/pio run -e esp32-c3 -t upload`
1.2 ⬜ [P2] Ouvrir le moniteur série (`pio device monitor`, filtres `esp32_exception_decoder` et `time` déjà configurés) et vérifier un boot sans erreur ni crash
1.3 ⬜ [P3] Vérifier que le GPS obtient un fix valide — au moins 4 satellites, HDOP sous 5.0 — dans les logs `DEBUG_INFO`
1.4 ⬜ [P4] Vérifier que `beaconManager.sendBeaconFrame()` s'exécute sans erreur, `esp_wifi_80211_tx` retournant `ESP_OK`
1.5 ⬜ [P5] Confirmer que le comportement de la LED correspond au mapping documenté dans `AGENTS.md` — `BLINK_SLOW` puis `BLINK_FAST` puis `ON` selon l'état GPS et balise

## Étape 2 — Portail de configuration (Phase 2)  [E2]

*(non détaillée)*

Réintégrer `network/web_server.{h,cpp}` et `network/dns_server.{h,cpp}` dans `main.cpp` — le code existe déjà mais n'est ni inclus ni appelé. Voir la section « Phase 2 » d'`AGENTS.md` pour ce qui a été retiré du build minimal.

## Étape 3 — Conformité réglementaire WiFi (pays et puissance)  [E3]

*(non détaillée)*

Remplacer `WiFi.setCountry()`, inexistant dans le core Arduino-ESP32 2.0.x, par l'équivalent ESP-IDF `esp_wifi_set_country()` et sa struct `wifi_country_t`.

## Étape 4 — Extension multi-cibles  [E4]

*(non détaillée)*

Compléter les environnements `esp32`, `esp32-s3` et `esp8266` dans `platformio.ini`, aujourd'hui de simples stubs `; TODO`. Seul `esp32-c3` est actif.
