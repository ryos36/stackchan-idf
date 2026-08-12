// SPDX-FileCopyrightText: 2026 Kenta IDA <fuga@fugafuga.org>
// SPDX-License-Identifier: BSL-1.0

#pragma once

namespace stackchan::board {

// Which Stack-chan base board the SoC is mounted on. Detected at begin() —
// CoreS3 variants discriminate on the M5 base's PY32 IO expander (0x6F);
// AtomS3R variants ("Atom-nyan") are picked up from M5Unified's chip ID;
// Core2 is picked up from M5Unified's board ID directly (no Stack-chan
// extension base — see board.cpp's early-return branch).
enum class BoardKind {
    M5Base,    // M5Stack Stack-chan base: PY32 servo-power EN, INA226 battery, servo on G6/G7.
    TakaoBase, // Takao Base (CoreS3 SE port A): half-duplex servos on port A, no power/battery control.
    AtomNyan,  // AtomS3R + Atomic ECHO BASE: 128x128 LCD, ES8311 codec, no servo/battery/LED/touch.
    AtomS3,    // Plain AtomS3 (no PSRAM) + Atomic ECHO BASE: avatar / jtts / LED only, no conv/audio-stream.
    StopWatch, // M5 StopWatch (C152): 466×466 AMOLED 円形 + CST820B touch + ES8311 + M5PM1 PMIC + M5IOE1.
               // No PY32 / no INA226 / no Si12T / no nekomimi LED 配線. Servo は背面 UART0 経由の Phase 3 オプション.
    Core2,     // M5Stack Core2 (ESP32-D0WDQ6-V3 / Xtensa LX6): 320×240 ILI9342C
               // + FT6336 touch, AXP192 PMIC, NS4168 speaker + PDM mic (I2S_NUM_0
               // 共有), MPU6886, BM8563. PY32 / Si12T / ネコミミ配線 / サーボバス
               // は無い。ESP32 では G6/G7 が内蔵フラッシュ専用なので、サーボ付き
               // Core2 を将来足すなら Port A (G32/G33) を使う別 kind として append。
};

// WIRE-FORMAT CONTRACT: the numeric values above are externally visible —
// BLE chr "BoardKind" serves static_cast<uint8_t>(kind) to paired web UIs,
// and wifi_config_service's release-OTA maps the same byte to a firmware
// slug (cores3/atoms3r/atoms3/stopwatch/core2). Never reorder or renumber;
// append new boards at the end only.
static_assert(static_cast<int>(BoardKind::M5Base) == 0 &&
              static_cast<int>(BoardKind::TakaoBase) == 1 &&
              static_cast<int>(BoardKind::AtomNyan) == 2 &&
              static_cast<int>(BoardKind::AtomS3) == 3 &&
              static_cast<int>(BoardKind::StopWatch) == 4 &&
              static_cast<int>(BoardKind::Core2) == 5,
              "BoardKind numbering is a BLE/OTA wire contract — append only");

} // namespace stackchan::board
