// ************************************************************************************************
// * BALISE D'IDENTIFICATION A DISTANCE POUR UAV (DGAC)                                           *
// ************************************************************************************************
// * Author:     Maxime MARCONATO (maxime@maaax.com, aka MaaaX^^, aka EPOCH84)
// * Repository: https://github.com/EPOCH84/Balise_Identification_A_Distance_UAV_DGAC
// * Based on:
// * - https://github.com/fanfanlatulipe26/BaliseDGAC_GPS_Logger
// * - https://github.com/dev-fred/GPS_Tracker_ESP8266
// * - https://github.com/khancyr/droneID_FR
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

#ifndef STATUS_PAGE_H
#define STATUS_PAGE_H

#include <Arduino.h>

// ================================================================================================
// STATUS PAGE
// Static page served once; its script polls /status.json about every 3 s, in step with the
// emission, so that each answer stays a few hundred bytes. The labels are in French, for the
// pilot and for an inspection in France. Values are written with textContent, never as HTML.
// ================================================================================================

static const char STATUS_PAGE_HTML[] PROGMEM = R"rawliteral(<!doctype html>
<html lang="fr">
<head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>Balise BIADUD</title>
  <style>
    *, *::before, *::after { box-sizing: border-box; }
    :root { --bg: #f4f6f8; --card: #ffffff; --text: #1b1f24; --muted: #5b6672; --ok: #1e7d32; --warn: #b26a00; --bad: #b3261e; --line: #d9dee3; }
    @media (prefers-color-scheme: dark) {
      :root { --bg: #15181c; --card: #1f2329; --text: #e8eaed; --muted: #9aa4ae; --ok: #66bb6a; --warn: #ffb74d; --bad: #ef9a9a; --line: #343a42; }
    }
    body { margin: 0; padding: 0.75rem; background: var(--bg); color: var(--text); font: 1rem/1.4 system-ui, sans-serif; }
    h1 { margin: 0 0 0.25rem; font-size: 1.3rem; }
    h2 { margin: 0 0 0.5rem; font-size: 1rem; color: var(--muted); }
    section { margin: 0 0 0.75rem; padding: 0.75rem; background: var(--card); border: 1px solid var(--line); border-radius: 0.5rem; }
    table { width: 100%; border-collapse: collapse; }
    th, td { padding: 0.2rem 0; text-align: left; vertical-align: top; }
    th { width: 45%; color: var(--muted); font-weight: normal; }
    td { font-variant-numeric: tabular-nums; word-break: break-all; }
    .state { font-size: 1.1rem; font-weight: bold; }
    .ok { color: var(--ok); }
    .warn { color: var(--warn); }
    .bad { color: var(--bad); }
    .note { margin: 0.4rem 0 0; color: var(--muted); font-size: 0.85rem; }
  </style>
</head>
<body>
  <header>
    <h1>Balise BIADUD</h1>
    <p id="state" class="state">Connexion&hellip;</p>
  </header>

  <section>
    <h2>Dernière trame émise</h2>
    <table>
      <tr><th scope="row">Identifiant</th><td id="f-id">&ndash;</td></tr>
      <tr><th scope="row">Latitude</th><td id="f-lat">&ndash;</td></tr>
      <tr><th scope="row">Longitude</th><td id="f-lon">&ndash;</td></tr>
      <tr><th scope="row">Altitude</th><td id="f-alt">&ndash;</td></tr>
      <tr><th scope="row">Hauteur</th><td id="f-h">&ndash;</td></tr>
      <tr><th scope="row">Départ</th><td id="f-home">&ndash;</td></tr>
      <tr><th scope="row">Vitesse</th><td id="f-spd">&ndash;</td></tr>
      <tr><th scope="row">Route</th><td id="f-hdg">&ndash;</td></tr>
      <tr><th scope="row">Émise il y a</th><td id="f-age">&ndash;</td></tr>
    </table>
    <p class="note">Ce que la balise a émis. La conformité se vérifie avec un récepteur indépendant.</p>
  </section>

  <section>
    <h2>Émission</h2>
    <table>
      <tr><th scope="row">Trames émises</th><td id="t-frames">&ndash;</td></tr>
      <tr><th scope="row">Écart maximal</th><td id="t-gap">&ndash;</td></tr>
      <tr><th scope="row">Écarts au-delà de 3 s</th><td id="t-over">&ndash;</td></tr>
    </table>
  </section>

  <section>
    <h2>GPS</h2>
    <table>
      <tr><th scope="row">Liaison</th><td id="g-link">&ndash;</td></tr>
      <tr><th scope="row">Fix</th><td id="g-fix">&ndash;</td></tr>
      <tr><th scope="row">Satellites</th><td id="g-sats">&ndash;</td></tr>
      <tr><th scope="row">HDOP</th><td id="g-hdop">&ndash;</td></tr>
    </table>
  </section>

  <section>
    <h2>Batterie</h2>
    <table>
      <tr><th scope="row">Cellule</th><td id="b-mv">&ndash;</td></tr>
      <tr><th scope="row">Alerte</th><td id="b-low">&ndash;</td></tr>
    </table>
  </section>

  <section>
    <h2>Maxima depuis le départ</h2>
    <table>
      <tr><th scope="row">Vitesse</th><td id="m-spd">&ndash;</td></tr>
      <tr><th scope="row">Hauteur</th><td id="m-h">&ndash;</td></tr>
    </table>
    <p class="note">Valeurs brutes : un saut du GPS peut les gonfler. Remises à zéro au redémarrage.</p>
  </section>

  <p id="updated" class="note">&ndash;</p>

  <script>
    'use strict';
    const POLL_MS = 3027;      // Out of step with the 2.8 s emission, as in the reference sketch
    const TIMEOUT_MS = 2500;
    let lastOk = 0;

    function put(id, text, cls) {
      const el = document.getElementById(id);
      el.textContent = text;
      el.className = cls || '';
    }

    function deg(e5) { return (e5 / 1e5).toFixed(5) + '°'; }

    function show(s) {
      const f = s.frame;
      if (s.state === 'live') { put('state', 'Émet une position vivante', 'state ok'); }
      else if (s.state === 'frozen') { put('state', 'Émet la dernière position connue (fix perdu)', 'state warn'); }
      else { put('state', "N'émet pas encore : attente de la position de départ", 'state bad'); }

      if (f) {
        put('f-id', f.id);
        put('f-lat', deg(f.lat));
        put('f-lon', deg(f.lon));
        put('f-alt', f.alt + ' m');
        put('f-h', f.h + ' m');
        put('f-home', deg(f.hlat) + ' / ' + deg(f.hlon));
        put('f-spd', f.spd + ' m/s (' + Math.round(f.spd * 3.6) + ' km/h)');
        put('f-hdg', f.hdg + '°');
        put('f-age', (s.age / 1000).toFixed(1) + ' s', s.age > 3000 ? 'bad' : '');
      }
      put('t-frames', String(s.frames));
      put('t-gap', s.maxgap + ' ms', s.maxgap > 3000 ? 'bad' : 'ok');
      put('t-over', String(s.over), s.over > 0 ? 'bad' : 'ok');

      const g = s.gps;
      put('g-link', g.mute ? 'Muet' : 'Reçoit', g.mute ? 'bad' : 'ok');
      put('g-fix', g.fix ? 'Oui' : 'Non', g.fix ? 'ok' : 'warn');
      put('g-sats', String(g.sats));
      put('g-hdop', g.hdop.toFixed(2));

      if (s.bat) {
        put('b-mv', (s.bat.mv / 1000).toFixed(2) + ' V');
        put('b-low', s.bat.low ? 'Batterie faible : se poser' : 'Non', s.bat.low ? 'bad' : 'ok');
      } else {
        put('b-mv', 'Non mesurée');
        put('b-low', '–');
      }

      if (s.max) {
        put('m-spd', s.max.spd.toFixed(1) + ' m/s (' + Math.round(s.max.spd * 3.6) + ' km/h)');
        put('m-h', s.max.h + ' m');
      }
    }

    async function poll() {
      const ctrl = new AbortController();
      const timer = setTimeout(() => ctrl.abort(), TIMEOUT_MS);
      try {
        const r = await fetch('/status.json', { cache: 'no-store', signal: ctrl.signal });
        if (!r.ok) { throw new Error('HTTP ' + r.status); }
        show(await r.json());
        lastOk = Date.now();
        put('updated', 'Mis à jour à ' + new Date(lastOk).toLocaleTimeString('fr-FR'));
      } catch (e) {
        const since = lastOk ? ' depuis ' + Math.round((Date.now() - lastOk) / 1000) + ' s' : '';
        put('updated', 'Balise injoignable' + since, 'note bad');
      } finally {
        clearTimeout(timer);
        setTimeout(poll, POLL_MS);
      }
    }

    poll();
  </script>
</body>
</html>
)rawliteral";

#endif // #ifndef STATUS_PAGE_H
