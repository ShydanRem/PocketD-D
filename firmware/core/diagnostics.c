#include "diagnostics.h"
#include <inttypes.h>
#include <stdio.h>

static bool version_string(const char *s) {
    if (!s || !*s) return false;
    for (unsigned i=0;s[i];i++) {
        unsigned char c=s[i];
        if (i>=48 || !((c>='a'&&c<='z')||(c>='A'&&c<='Z')||
                (c>='0'&&c<='9')||c=='.'||c=='-'||c=='_'||c=='+')) return false;
    }
    return true;
}
static const char *boolean(bool value) { return value?"true":"false"; }

size_t pd_diagnostics_encode(const pd_diagnostics *d, char *out, size_t capacity) {
    if (out && capacity) out[0]=0;
    if (!d || !out || !capacity || !version_string(d->firmware) || !version_string(d->idf)) return 0;
    int n=snprintf(out,capacity,
        "{\"format\":\"pocketdd-diagnostics\",\"version\":1,\"board_target\":\"" PD_BOARD_TARGET "\","
        "\"firmware\":\"%s\",\"idf\":\"%s\",\"uptime_ms\":%" PRIu64 ",\"reset_reason\":%" PRIu32 ","
        "\"formats\":{\"usb\":16,\"state\":35,\"history\":2,\"backup\":1},"
        "\"memory\":{\"internal\":{\"free\":%" PRIu32 ",\"minimum_free\":%" PRIu32 ",\"largest_block\":%" PRIu32 "},"
        "\"psram\":{\"total\":%" PRIu32 ",\"free\":%" PRIu32 ",\"minimum_free\":%" PRIu32 ",\"largest_block\":%" PRIu32 "}},"
        "\"stack\":{\"ui_min_bytes\":%" PRIu32 ",\"usb_min_bytes\":%" PRIu32 "},"
        "\"io\":{\"lvgl_ticks\":%" PRIu32 ",\"lvgl_max_us\":%" PRIu32 ",\"flushes_submitted\":%" PRIu32 ","
        "\"touch_samples\":%" PRIu32 ",\"touch_errors\":%" PRIu32 ",\"touch_contacts\":%" PRIu32 ","
        "\"pwr_read_errors\":%" PRIu32 ",\"pwr_presses\":%" PRIu32 ",\"boot_presses\":%" PRIu32 ",\"pwr_available\":%s},"
        "\"storage\":{\"state_writable\":%s,\"history_available\":%s,\"history_pending\":%s,\"history_matches\":%s,\"state_valid\":%s},"
        "\"screen\":{\"dimmed\":%s,\"failed\":%s,\"busy\":%s}}",
        d->firmware,d->idf,d->uptime_ms,d->reset_reason,
        d->internal_free,d->internal_min,d->internal_largest,
        d->psram_total,d->psram_free,d->psram_min,d->psram_largest,
        d->ui_stack_min,d->usb_stack_min,d->lvgl_ticks,d->lvgl_max_us,d->flushes_submitted,
        d->touch_samples,d->touch_errors,d->touch_contacts,d->pwr_read_errors,d->pwr_presses,d->boot_presses,
        boolean(d->pwr_available),boolean(d->state_writable),boolean(d->history_available),
        boolean(d->history_pending),boolean(d->history_matches),boolean(d->state_valid),
        boolean(d->dimmed),boolean(d->screen_failed),boolean(d->busy));
    if (n<0 || (size_t)n>=capacity || n>PD_DIAGNOSTICS_MAX) { out[0]=0;return 0; }
    return (size_t)n;
}
