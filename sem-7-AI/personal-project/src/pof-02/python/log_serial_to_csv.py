#!/usr/bin/env python3
"""Capture CSV rows streamed over Serial from pof-02 and persist to a local CSV file."""

from __future__ import annotations

import argparse
from datetime import datetime, timezone
from pathlib import Path

import serial


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--port", required=True, help="Serial port, e.g. /dev/ttyUSB0 or COM4")
    parser.add_argument("--baud", type=int, default=115200, help="Baud rate (default: 115200)")
    parser.add_argument(
        "--output",
        default=f"pof02-log-{datetime.now(timezone.utc).strftime('%Y%m%d-%H%M%S')}.csv",
        help="CSV output path on this laptop",
    )
    return parser.parse_args()


def main() -> None:
    args = parse_args()
    output_path = Path(args.output).expanduser().resolve()
    output_path.parent.mkdir(parents=True, exist_ok=True)

    with serial.Serial(args.port, args.baud, timeout=1) as ser, output_path.open("w", encoding="utf-8") as out_file:
        print(f"Listening on {args.port} @ {args.baud} and writing to {output_path}")
        for raw_line in ser:
            line = raw_line.decode("utf-8", errors="replace").strip()
            if not line:
                continue

            if line.startswith("timestamp_utc,") or line.startswith("unix_time,") or line[:1].isdigit():
                out_file.write(line + "\n")
                out_file.flush()
                print(line)
            else:
                print(f"[device] {line}")


if __name__ == "__main__":
    main()
