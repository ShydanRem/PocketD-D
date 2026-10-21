#pragma once
#include "lvgl.h"
#include <stdbool.h>

/* Optional native prototype, not connected to production state or storage.
 * Only one view may be open. The receiver owns game actions and persistence.
 * Call close BEFORE deleting the root. show replaces the contents of its root.
 */
typedef enum { COMPANION_HOME, COMPANION_MENU, COMPANION_VITALS } companion_page;
typedef enum {
    COMPANION_OPEN_MENU, COMPANION_OPEN_HOME, COMPANION_OPEN_VITALS,
    COMPANION_ATTACK, COMPANION_DICE, COMPANION_MAGIC, COMPANION_PACK,
    COMPANION_REST, COMPANION_SHEET, COMPANION_APPEARANCE,
    COMPANION_MINUS, COMPANION_PLUS, COMPANION_UNDO
} companion_action;
typedef struct {
    companion_page page;
    const char *name; /* copied by LVGL during show; no borrowed text in timer */
    unsigned level, hp, maximum, temporary, armor;
    bool motion, paused, storage_error, can_undo, can_heal, can_hurt;
    int applied_hp, applied_temporary; /* actual committed deltas, not requested amounts */
} companion_view;
typedef void (*companion_handler)(companion_action action, void *context);

void companion_show(lv_obj_t *root, const companion_view *view,
                    companion_handler handler, void *context);
void companion_close(void);
void companion_pause(bool paused);
void companion_focus_step(int direction);
void companion_activate(void);
unsigned companion_focus_index(void);
