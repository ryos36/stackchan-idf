#!/usr/bin/env python3
# SPDX-FileCopyrightText: 2026 Kenta IDA <fuga@fugafuga.org>
# SPDX-License-Identifier: BSL-1.0
"""Non-interactive serial log capture.

Resets the target via RTS (matching esptool's HardReset sequence, i.e. a
normal app boot rather than bootloader entry), then reads stdout from the
device for a fixed duration. Useful from a non-TTY harness where idf.py
monitor refuses to run.

Usage:
    python tools/monitor_log.py [--port /dev/ttyACM0] [--seconds 8]
                                [--baud 115200]
"""
from __future__ import annotations

import argparse
import sys
import time

import serial


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--port", default="/dev/ttyACM0")
    ap.add_argument("--baud", type=int, default=115200)
    ap.add_argument("--seconds", type=float, default=8.0)
    args = ap.parse_args()

    with serial.Serial(args.port, args.baud, timeout=0.2) as ser:
        # esptool's HardReset (not ClassicReset): only toggle RTS (EN).
        # DTR (IO0) stays False/high throughout so the chip boots the app
        # normally. The previous DTR+RTS dance here was esptool's
        # ClassicReset sequence, which is for *entering* the ROM
        # downloader before a flash write, not for a normal app boot —
        # it left the device stuck at "waiting for download".
        ser.setDTR(False)
        ser.setRTS(True);  time.sleep(0.1)
        ser.setRTS(False)

        end = time.time() + args.seconds
        while time.time() < end:
            chunk = ser.read(4096)
            if chunk:
                sys.stdout.buffer.write(chunk)
                sys.stdout.buffer.flush()
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
