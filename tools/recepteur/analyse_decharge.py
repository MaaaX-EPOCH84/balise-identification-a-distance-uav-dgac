#!/usr/bin/env python3
# ************************************************************************************************
# * BALISE D'IDENTIFICATION A DISTANCE POUR UAV (DGAC)                                           *
# ************************************************************************************************
# * Author:     Maxime MARCONATO (maxime@maaax.com, aka MaaaX^^, aka EPOCH84)
# * Repository: https://github.com/EPOCH84/Balise_Identification_A_Distance_UAV_DGAC
# ************************************************************************************************
# GPL V3:
# This program is free software: you can redistribute it and/or modify it under the terms of the
# GNU General Public License as published by the Free Software Foundation, either version 3 of the
# License, or (at your option) any later version.
# This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
# without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.
# ================================================================================================
# Reads a discharge log written by consigne.py and reports what the reference cycle must give:
# duration from the start of the log to the last frame received (the battery protection cut-off),
# reboots and lost frames on the way, the cell voltage a given number of minutes before the end,
# and a voltage -> remaining time table. Optionally draws the curve as an SVG file.
#
# The end of the run is the last frame before the first silence longer than --coupure minutes,
# so it is known to one telemetry period (~10 s). What follows that silence is reported apart: a
# drained cell recovers a few hundred mV once unloaded, and the beacon then restarts for a minute
# or so before cutting off again. Those relapses are not part of the usable discharge.
# Voltages are medians over a short window, the ADC reading moving by a few mV from one frame to
# the next under radio load.
#
# Standard library only.
#
#   python3 tools/recepteur/analyse_decharge.py decharge.csv [--reserve 10] [--pas 5]
#                                               [--coupure 5] [--table table.csv] [--svg courbe.svg]
# ================================================================================================

import argparse
import csv
import statistics
import sys
from dataclasses import dataclass
from pathlib import Path

WINDOW_S = 60          # median window around a point of the curve
GAP_WARN_S = 30        # reception gap worth reporting (three telemetry periods)
CUTOFF_MIN = 5         # default silence that marks the cut-off, in minutes


@dataclass
class Sample:
    t: float           # PC time, seconds since the first row
    seq: int
    boot: int
    up: int
    cell: int          # mV
    fix: int
    maxgap: int
    over: int


def load(path: Path) -> list[Sample]:
    with path.open(encoding="utf-8", newline="") as src:
        rows = list(csv.DictReader(src))
    if not rows:
        raise SystemExit(f"{path}: no data")
    t0 = float(rows[0]["pc_epoch"])
    return [Sample(float(r["pc_epoch"]) - t0, int(r["seq"]), int(r["boot"]), int(r["up"]),
                   int(r["cell"]), int(r["fix"]), int(r["maxgap"]), int(r["over"]))
            for r in rows]


def median_cell(samples: list[Sample], centre: float) -> int | None:
    near = [s.cell for s in samples if abs(s.t - centre) <= WINDOW_S / 2]
    return round(statistics.median(near)) if near else None


def hms(seconds: float) -> str:
    seconds = round(seconds)
    return f"{seconds // 3600}h{seconds % 3600 // 60:02d}m{seconds % 60:02d}s"


def split_at_cutoff(samples: list[Sample], cutoff_min: float) -> tuple[list[Sample], list[Sample]]:
    for i, (prev, cur) in enumerate(zip(samples, samples[1:])):
        if cur.t - prev.t > cutoff_min * 60:
            return samples[:i + 1], samples[i + 1:]
    return samples, []


def report_relapses(relapses: list[Sample], cut: float) -> None:
    if not relapses:
        print("relapses   none")
        return
    print("relapses   (after the cut-off, not counted)")
    start = relapses[0]
    for prev, cur in zip(relapses, relapses[1:] + [None]):
        if cur is None or cur.boot != prev.boot or cur.t - prev.t > GAP_WARN_S:
            print(f"  +{hms(start.t - cut)} boot {start.boot}: {prev.t - start.t + 10:.0f} s, "
                  f"cell {start.cell} -> {prev.cell} mV")
            start = cur


def report(samples: list[Sample], reserve_min: float, step_min: float) -> list[tuple[float, int]]:
    end = samples[-1].t
    print(f"frames     {len(samples)}, from {samples[0].cell} mV to {samples[-1].cell} mV")
    print(f"duration   {hms(end)} (end = last frame, +/- one telemetry period)")

    boots = sorted({s.boot for s in samples})
    print(f"boots      {boots}" + ("" if len(boots) == 1 else "  <- reboot(s) during the run"))
    for prev, cur in zip(samples, samples[1:]):
        if cur.boot != prev.boot:
            print(f"  reboot at {hms(cur.t)}: boot {prev.boot} -> {cur.boot}, "
                  f"cell {prev.cell} -> {cur.cell} mV, up {cur.up} s")
        elif cur.t - prev.t > GAP_WARN_S:
            print(f"  gap of {cur.t - prev.t:.0f} s at {hms(prev.t)} "
                  f"({cur.seq - prev.seq - 1} frame(s) lost)")

    lost = sum(cur.seq - prev.seq - 1 for prev, cur in zip(samples, samples[1:])
               if cur.boot == prev.boot and cur.seq > prev.seq)
    print(f"lost       {lost} frame(s) in sequence")
    print(f"fix        {100 * sum(s.fix for s in samples) / len(samples):.0f} % of the frames")
    print(f"DGAC       maxgap {max(s.maxgap for s in samples)} ms, "
          f"over 3 s {max(s.over for s in samples)}")

    threshold = median_cell(samples, end - reserve_min * 60)
    print(f"reserve    {threshold} mV at end - {reserve_min:g} min")

    table = []
    remaining = 0.0
    while remaining * 60 <= end:
        value = median_cell(samples, end - remaining * 60)
        if value is not None:
            table.append((remaining, value))
        remaining += step_min
    print("remaining  cell")
    for minutes, value in table:
        print(f"{minutes:7.0f} min  {value} mV")
    return table


def write_table(path: Path, table: list[tuple[float, int]]) -> None:
    with path.open("w", encoding="utf-8", newline="") as out:
        writer = csv.writer(out)
        writer.writerow(["remaining_min", "cell_mv"])
        writer.writerows(table)


def write_svg(path: Path, samples: list[Sample], reserve_min: float) -> None:
    width, height, margin = 900, 500, 60
    end = samples[-1].t or 1.0
    low = min(s.cell for s in samples) // 100 * 100
    high = -(-max(s.cell for s in samples) // 100) * 100
    span = (high - low) or 100

    def x(t: float) -> float:
        return margin + (width - 2 * margin) * t / end

    def y(mv: float) -> float:
        return height - margin - (height - 2 * margin) * (mv - low) / span

    parts = [f'<svg xmlns="http://www.w3.org/2000/svg" width="{width}" height="{height}" '
             f'font-family="sans-serif" font-size="12">',
             f'<rect width="{width}" height="{height}" fill="white"/>']
    for mv in range(low, high + 1, 100):
        parts.append(f'<line x1="{margin}" x2="{width - margin}" y1="{y(mv):.1f}" '
                     f'y2="{y(mv):.1f}" stroke="#ddd"/>'
                     f'<text x="{margin - 6}" y="{y(mv) + 4:.1f}" text-anchor="end">{mv}</text>')
    for hour in range(int(end // 3600) + 1):
        parts.append(f'<line x1="{x(hour * 3600):.1f}" x2="{x(hour * 3600):.1f}" '
                     f'y1="{margin}" y2="{height - margin}" stroke="#ddd"/>'
                     f'<text x="{x(hour * 3600):.1f}" y="{height - margin + 16}" '
                     f'text-anchor="middle">{hour} h</text>')
    mark = x(end - reserve_min * 60)
    parts.append(f'<line x1="{mark:.1f}" x2="{mark:.1f}" y1="{margin}" y2="{height - margin}" '
                 f'stroke="#c60" stroke-dasharray="4 4"/>'
                 f'<text x="{mark - 4:.1f}" y="{margin - 6}" text-anchor="end" fill="#c60">'
                 f'end - {reserve_min:g} min</text>')
    points = " ".join(f"{x(s.t):.1f},{y(s.cell):.1f}" for s in samples)
    parts.append(f'<polyline points="{points}" fill="none" stroke="#06c" stroke-width="1.5"/>')
    parts.append(f'<text x="{width / 2}" y="{margin / 2}" text-anchor="middle">'
                 f'Cell voltage (mV) - {hms(end)}</text></svg>')
    path.write_text("\n".join(parts), encoding="utf-8")


def main() -> int:
    parser = argparse.ArgumentParser(description="Analyse a reference discharge log")
    parser.add_argument("log", type=Path, help="CSV written by consigne.py")
    parser.add_argument("--reserve", type=float, default=10,
                        help="reserve in minutes before the cut-off (default: 10)")
    parser.add_argument("--pas", type=float, default=5,
                        help="step of the remaining-time table, in minutes (default: 5)")
    parser.add_argument("--coupure", type=float, default=CUTOFF_MIN,
                        help=f"silence marking the cut-off, in minutes (default: {CUTOFF_MIN})")
    parser.add_argument("--table", type=Path, help="write the remaining-time table as CSV")
    parser.add_argument("--svg", type=Path, help="draw the curve as SVG")
    args = parser.parse_args()

    samples, relapses = split_at_cutoff(load(args.log), args.coupure)
    table = report(samples, args.reserve, args.pas)
    report_relapses(relapses, samples[-1].t)
    if args.table:
        write_table(args.table, table)
    if args.svg:
        write_svg(args.svg, samples, args.reserve)
    return 0


if __name__ == "__main__":
    sys.exit(main())
