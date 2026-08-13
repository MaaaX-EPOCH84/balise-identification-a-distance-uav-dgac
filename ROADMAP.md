# Feuille de route — Balise Identification UAV DGAC

Statut : ⬜ à faire · 🔶 en cours · ✅ fait · ❌ annulé

| # | id | Étape | Contenu | État |
|---|---|---|---|---|
| 1 | E1 | Cœur minimal | GPS + trame balise DGAC, validation sur USB | 🔶 |
| 2 | E5 | Assemblage matériel | Alimentation, signalisation, antenne et boîtier | ⬜ |
| 3 | E6 | Signalisation et énergie | Mesure de batterie et alerte, côté firmware | ⬜ |
| 4 | E2 | Portail de configuration | Serveur web et DNS captif dans `main.cpp` (Phase 2) | ⬜ |
| 5 | E3 | Conformité réglementaire WiFi | Pays et puissance via `esp_wifi_set_country()` | ⬜ |
| 6 | E4 | Extension multi-cibles | Environnements `esp32`, `esp32-s3`, `esp8266` | ⬜ |

## Étape 1 — Cœur minimal (GPS + trame balise DGAC)  [E1]

Le firmware compile (`pio run -e esp32-c3` → SUCCESS) et le périmètre « GPS + trame balise DGAC » est câblé dans `main.cpp`. Rien n'a encore été validé sur matériel physique.

1.1 ✅ [P0] **Interblocage d'émission corrigé.** `isReady()` exigeait `_frame_count > 0` alors que `main.cpp` conditionnait l'envoi à `isReady()` : aucune trame n'aurait jamais pu partir. `isReady()` exprime désormais l'aptitude à émettre (`_initialized && _home_set && checkGpsDataValidity()`), et `main.cpp` appelle `sendBeaconFrame()` sans garde redondante puisque `shouldSend()` filtre déjà.
1.2 ✅ [P6] Armement de `_gps_ready` corrigé dans `main.cpp` : `!_gps_ready && gpsManager.isReady()` au lieu de `!gpsManager.isReady() && gpsManager.hasValidFix()`, qui se déclenchait sur un fix incomplet et devenait impossible à satisfaire une fois le GPS pleinement valide.
1.3 ⬜ [P1] Flasher le firmware sur l'ESP32-C3 physique — `~/.platformio/penv/bin/pio run -e esp32-c3 -t upload`
1.4 ⬜ [P2] Ouvrir le moniteur série (`pio device monitor`, filtres `esp32_exception_decoder` et `time` déjà configurés) et vérifier un boot sans erreur ni crash
1.5 ⬜ [P3] Vérifier que le GPS obtient un fix valide — au moins 4 satellites, HDOP sous 5.0 — dans les logs `DEBUG_INFO`
1.6 ⬜ [P4] Vérifier que `beaconManager.sendBeaconFrame()` s'exécute sans erreur, `esp_wifi_80211_tx` retournant `ESP_OK`
1.7 ⬜ [P5] Confirmer que le comportement de la LED correspond au mapping documenté dans `AGENTS.md` — `BLINK_SLOW` puis `BLINK_FAST` puis `ON` selon l'état GPS et balise

## Étape 2 — Assemblage matériel et boîtier  [E5]

Périmètre matériel arrêté le 12 août 2026, détaillé dans la section « Matériel embarqué » d'`AGENTS.md` : nomenclature, brochage, câblage, pièges d'assemblage.

**En attente de livraison du buck-boost XL63020** (commandé le 12 août 2026). Cette attente bloque `P7`, `P9`, `P12` — et `P17` à l'étape 3, qui suppose l'assemblage complet. Elle ne bloque rien d'autre : `P8`, `P10` et `P11` se font sur une carte alimentée en USB, et **la chaîne de mesure de batterie fonctionne sans le régulateur** (cellule sur le TP4056, pont diviseur sur `OUT+`, carte en USB), donc l'étape 3 peut être menée en parallèle plutôt qu'après. L'ordre des étapes n'est plus contraignant tant que le module n'est pas là.

2.1 ⬜ [P7] Souder le pad 3,3 V du XL63020 et vérifier la sortie à vide au multimètre avant tout raccordement à la carte
2.2 ⬜ [P8] Ramener le courant de charge du TP4056 à 500 mA (résistance de 2,4 kΩ), et vérifier la continuité `5V` ↔ VBUS de la carte avant de câbler `IN+`/`IN-`
2.3 ⬜ [P9] Câbler la chaîne d'alimentation : cellule → TP4056 → interrupteur → régulateur → 3V3 de la carte et VCC du GPS, avec le condensateur réservoir au plus près
2.4 ⬜ [P10] Câbler signalisation et entrées : LED verte déportée sur GPIO8, LED jaune sur GPIO6, LED rouge fixe sur le rail 3,3 V, bouton sur GPIO2, pont de 2 × 220 kΩ sur GPIO3
2.5 ⬜ [P11] Basculer le cavalier RF vers le connecteur d'antenne externe et vérifier le gain de portée réel
2.6 ⬜ [P12] Dessiner et imprimer le boîtier — PETG non chargé en carbone, ouvertures USB, interrupteur, bouton et LED, antenne GPS vers le ciel

## Étape 3 — Signalisation et supervision d'énergie  [E6]

Volet firmware des décisions matérielles de l'étape 2. Règle de conformité qui encadre tout ce qui suit : la balise doit émettre pendant tout le vol, donc une batterie faible se signale mais n'interrompt jamais l'émission.

3.1 ⬜ [P13] Définir `PIN_BATTERY` dans `biadud_platform.h` et écrire `hardware/battery_monitor.{h,cpp}` : lecture sur ADC1, moyennage de plusieurs échantillons, conversion en tension cellule
3.2 ⬜ [P14] Ajouter la seconde instance `LedManager` (`ledWarning`, GPIO6) et définir le mapping des signaux entre LED verte et LED jaune
3.3 ⬜ [P15] Corriger la polarité de la LED d'état si la carte s'avère câblée en actif bas — second argument de `LedManager::begin()`
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
