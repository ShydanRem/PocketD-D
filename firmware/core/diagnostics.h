#pragma once
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define PD_DIAGNOSTICS_MAX 2048
#define PD_BOARD_TARGET "ESP32-S3-Touch-AMOLED-1.75"

/* Runtime observations only. No character data or device identifiers. */
typedef struct {
    const char *firmware, *idf;
    uint64_t uptime_ms;
    uint32_t reset_reason;
    uint32_t internal_free, internal_min, internal_largest;
    uint32_t psram_total, psram_free, psram_min, psram_largest;
    uint32_t ui_stack_min, usb_stack_min;
    uint32_t lvgl_ticks, lvgl_max_us, flushes_submitted;
    uint32_t touch_samples, touch_errors, touch_contacts;
    uint32_t pwr_read_errors, pwr_presses, boot_presses;
    bool pwr_available, state_writable, history_available;
    bool history_pending, history_matches, state_valid;
    bool dimmed, screen_failed, busy;
} pd_diagnostics;

/* JSON only, excluding newline/prefix. Zero on invalid input or small buffer. */
size_t pd_diagnostics_encode(const pd_diagnostics *d, char *out, size_t capacity);
