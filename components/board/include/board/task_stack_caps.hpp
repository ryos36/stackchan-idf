// SPDX-FileCopyrightText: 2026 Kenta IDA <fuga@fugafuga.org>
// SPDX-License-Identifier: BSL-1.0
#pragma once

#include <esp_heap_caps.h>
#include <freertos/FreeRTOS.h>

namespace stackchan::board {

// Memory capability flags for a large (multi-KiB) task stack.
//
// ESP32 (plain, e.g. Core2) can't safely place a task stack in PSRAM:
// CONFIG_FREERTOS_TASK_CREATE_ALLOW_EXT_MEM defaults off there, so
// FreeRTOS's xPortcheckValidStackMem() (components/freertos/heap_idf.c)
// rejects any stack pointer that isn't internal RAM — a hard assert at
// task-creation time, not a soft failure. Confirmed on Core2 hardware via
// the jtts test-speak worker (2026-08-13): the crash was immediate and
// deterministic, not a rare cache-disabled-window hit.
//
// MALLOC_CAP_8BIT is bundled in on both targets (not left to call sites):
// ESP32's internal-RAM heap includes spare IRAM registered as 32-bit-only
// memory, so MALLOC_CAP_INTERNAL alone can hand back a region that fails
// xPortcheckValidStackMem's esp_ptr_byte_accessible() check just the same.
// PSRAM never had this ambiguity (it's byte-accessible by construction),
// which is why some existing PSRAM call sites omitted MALLOC_CAP_8BIT
// without incident — that omission would silently break moving to
// MALLOC_CAP_INTERNAL.
//
// ESP32-S3 (CoreS3 etc.) allows PSRAM stacks safely and several tasks rely
// on keeping internal RAM free for Wi-Fi/BLE/mbedtls, so PSRAM stays the
// default there.
#if CONFIG_IDF_TARGET_ESP32
inline constexpr UBaseType_t kLargeTaskStackCaps = MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT;
#else
inline constexpr UBaseType_t kLargeTaskStackCaps = MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT;
#endif

} // namespace stackchan::board
