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
# Logs the bench telemetry printed by the bench receiver ("TLM" lines) to a CSV file, one row per
# frame, stamped with the PC clock. Each row is flushed at once: the run usually ends with the
# beacon cut off by its battery protection, and nothing received must be lost.
#
# Standard library only. The port is opened without HUPCL, so closing it does not reset the
# receiver.
#
#   python3 tools/recepteur/consigne.py /dev/ttyACM1 decharge.csv [--duree 3600]
# ================================================================================================

import argparse
import csv
import logging
import os
import sys
import termios
import time
from pathlib import Path

FIELDS = ["seq", "boot", "up", "cell", "pin", "frames", "maxgap", "over",
          "fix", "home", "frozen", "rssi"]


def open_port(port: str) -> int:
    fd = os.open(port, os.O_RDONLY | os.O_NOCTTY)
    attrs = termios.tcgetattr(fd)
    attrs[0] = 0                                                # iflag: raw
    attrs[1] = 0                                                # oflag
    attrs[2] = termios.CS8 | termios.CREAD | termios.CLOCAL     # cflag, HUPCL cleared
    attrs[3] = 0                                                # lflag: no echo, no canonical
    attrs[4] = attrs[5] = termios.B115200
    attrs[6][termios.VMIN] = 0
    attrs[6][termios.VTIME] = 10                                # read returns after 1 s at most
    termios.tcsetattr(fd, termios.TCSANOW, attrs)
    return fd


def parse(line: str) -> dict[str, int] | None:
    if not line.startswith("TLM seq="):
        return None
    values = dict(item.split("=", 1) for item in line[4:].split())
    try:
        return {key: int(values[key]) for key in FIELDS}
    except (KeyError, ValueError):
        return None


def main() -> int:
    parser = argparse.ArgumentParser(description="Log the bench telemetry to CSV")
    parser.add_argument("port", help="serial port of the bench receiver, e.g. /dev/ttyACM1")
    parser.add_argument("output", type=Path, help="CSV file, appended to if it exists")
    parser.add_argument("--duree", type=float, default=0,
                        help="stop after this many seconds (default: until Ctrl-C)")
    args = parser.parse_args()

    logging.basicConfig(level=logging.INFO, format="%(asctime)s %(message)s", datefmt="%H:%M:%S")

    new_file = not args.output.exists() or args.output.stat().st_size == 0
    fd = open_port(args.port)
    start = time.monotonic()
    rows = 0
    last: dict[str, int] | None = None
    pending = b""

    with args.output.open("a", encoding="utf-8", newline="") as out:
        writer = csv.writer(out)
        if new_file:
            writer.writerow(["pc_time", "pc_epoch"] + FIELDS)
            out.flush()
        try:
            while not args.duree or time.monotonic() - start < args.duree:
                chunk = os.read(fd, 512)
                if not chunk:
                    continue
                pending += chunk
                *lines, pending = pending.split(b"\n")
                for raw in lines:
                    sample = parse(raw.decode("ascii", "replace").strip())
                    if sample is None:
                        continue
                    now = time.time()
                    writer.writerow([time.strftime("%Y-%m-%dT%H:%M:%S", time.localtime(now)),
                                     f"{now:.1f}"] + [sample[key] for key in FIELDS])
                    out.flush()
                    rows += 1
                    if last is not None and sample["boot"] != last["boot"]:
                        logging.warning("new boot %d (was %d)", sample["boot"], last["boot"])
                    elif last is not None and sample["seq"] != last["seq"] + 1:
                        logging.warning("%d frame(s) lost", sample["seq"] - last["seq"] - 1)
                    logging.info("cell %d mV  frames %d  maxgap %d ms  over %d  fix %d  rssi %d",
                                 sample["cell"], sample["frames"], sample["maxgap"],
                                 sample["over"], sample["fix"], sample["rssi"])
                    last = sample
        except KeyboardInterrupt:
            pass
        finally:
            os.close(fd)

    logging.info("%d rows written to %s", rows, args.output)
    return 0


if __name__ == "__main__":
    sys.exit(main())
