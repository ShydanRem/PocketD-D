#pragma once
#include "lvgl.h"
#include <stdbool.h>

/* Device view, independent of game state and storage.
 * Only one view may be open. The receiver owns game actions and persistence.
 * Call close BEFORE deleting the root. show replaces the contents of its root.
 */
typedef enum { COMPANION_HOME, COMPANION_MENU, COMPANION_VITALS } companion_page;
typedef enum {
    COMPANION_OPEN_MENU, COMPANION_OPEN_HOME, COMPANION_OPEN_VITALS,
    COMPANION_ATTACK, COMPANION_DICE, COMPANION_MAGIC, COMPANION_PACK,
    COMPANION_REST, COMPANION_SHEET, COMPANION_APPEARANCE,
    COMPANION_MINUS, COMPANION_PLUS, COMPANION_UNDO, COMPANION_RESOURCES,
    COMPANION_HEAL, COMPANION_TEMPORARY, COMPANION_DAMAGE, COMPANION_STATUS
} companion_action;
typedef struct {
    companion_page page;
    const char *name; /* copied by LVGL during show; no borrowed text in timer */
    unsigned level, hp, maximum, temporary, armor;
    bool motion, paused, storage_error, can_undo, can_heal, can_hurt;
    int applied_hp, applied_temporary; /* actual committed deltas, not requested amounts */
    uint32_t feedback_age; /* elapsed since commit; redraw never restarts a receipt */
    const char *notice;
    const lv_image_dsc_t *avatar; /* caller owns live descriptor; NULL: demonstration sprite */
} companion_view;
typedef void (*companion_handler)(companion_action action, void *context);

void companion_show(lv_obj_t *root, const companion_view *view,
                    companion_handler handler, void *context);
void companion_close(void);
void companion_pause(bool paused);
void companion_focus_step(int direction);
void companion_activate(void);
unsigned companion_focus_index(void);
lv_obj_t *companion_avatar_object(void);
bool companion_feedback_visible(void);
void companion_clear_feedback(void);
