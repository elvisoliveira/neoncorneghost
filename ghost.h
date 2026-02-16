/*
 * Master OLED rendering and ghost animations
 * -----------------------------------------
 * The master_render_ghost() function defines several small bitmap animations
 * (stored in PROGMEM to save RAM) and contains the logic to select which
 * animation to draw on the master half's OLED. Animations are stored as
 * arrays of bytes (bitmaps) and written directly to the OLED with
 * oled_write_raw_P().
 *
 * Selection logic (in animate_ghost):
 * - If Caps Lock is active: show the "hide" animation
 * - If LOWER layer is active: show "shyguy" (or "laugh" when TUNE is also active)
 * - If RAISE layer is active: show "eerie" (or "laugh" when TUNE is also active)
 * - If Left or Right Shift is held: show "scare"
 * - If Right Alt is held: show "troll"
 * - Otherwise: show the default "laugh" animation
 *
 * Timing: an anim_ghost_timer controls the frame rate (ANIM_FRAME_DURATION
 * ms per frame). When the user is typing (non-zero WPM) the animation sleep
 * timers are reset so the display remains active.
 */
static void master_render_ghost(void) {
    // Large ghost animation frames moved to separate header to reduce
    // keymap.c size and improve maintainability. This header defines
    // the following function-local PROGMEM arrays:
    //   hide[], shyguy[], eerie[], scare[], troll[], laugh[]
    #define OLED_ANIMS_GHOST
    #include "oled_anims.h"
    #undef OLED_ANIMS_GHOST

    void animate_ghost(void) {
        current_ghost_frame = (current_ghost_frame + 1) % 2; // alternate between frame 0 and 1
        if (led_usb_state.caps_lock) { // Caps Lock active -> hiding animation
            oled_write_raw_P(hide[abs(1 - current_ghost_frame)], ANIM_SIZE_GHOST); // hide
        }
        else if (IS_LAYER_ON(_LOWER)) {
            if (IS_LAYER_OFF(_TUNE)) {
                // LOWER layer active, normal shyguy animation
                oled_write_raw_P(shyguy[abs(1 - current_ghost_frame)], ANIM_SIZE_GHOST);
            }
            else {
                // LOWER + TUNE -> laughing animation
                oled_write_raw_P(laugh[abs(1 - current_ghost_frame)], ANIM_SIZE_GHOST);
            }
        }
        else if (IS_LAYER_ON(_RAISE)) {
            if (IS_LAYER_OFF(_TUNE)) {
                // RAISE layer active, eerie animation
                oled_write_raw_P(eerie[abs(1 - current_ghost_frame)], ANIM_SIZE_GHOST);
            }
            else {
                // RAISE + TUNE -> laughing animation
                oled_write_raw_P(laugh[abs(1 - current_ghost_frame)], ANIM_SIZE_GHOST);
            }
        }
        else if ( (keyboard_report->mods & MOD_BIT (KC_LSFT)) || (keyboard_report->mods & MOD_BIT (KC_RSFT)) ) {
            // Shift held -> scare animation
            oled_write_raw_P(scare[abs(1 - current_ghost_frame)], ANIM_SIZE_GHOST);
        }
        else if (keyboard_report->mods & MOD_BIT (KC_RALT)) {
            // Right Alt held -> troll animation
            oled_write_raw_P(troll[abs(1 - current_ghost_frame)], ANIM_SIZE_GHOST);
        }
        else {
            // Default: laughing animation
            oled_write_raw_P(laugh[abs(1 - current_ghost_frame)], ANIM_SIZE_GHOST);
        }
    }
    // Update the animation frame at the configured frame duration.
    if (timer_elapsed32(anim_ghost_timer) > ANIM_FRAME_DURATION) {
        anim_ghost_timer = timer_read32(); // reset the frame timer
        animate_ghost(); // draw the next frame
        if (current_wpm != 0) {
            // If the user is typing, update the "last active" timestamp
            anim_ghost_sleep = timer_read32();
        }
    }
}

static void slave_render_ghost(void) {
    // The slave (secondary) OLED displays a larger "fishing" animation.
    // This array contains two 416-byte frames (32x100 pixels represented as
    // pages) stored in PROGMEM. The animate_fishing() helper toggles
    // current_ghost_frame and writes the appropriate frame to the slave OLED.
    // Fishing animation frames moved to a separate header to reduce
    // keymap.c size. Included here so `fishing` remains function-local.
    #define OLED_ANIMS_FISHING
    #include "oled_anims.h"
    #undef OLED_ANIMS_FISHING
    // Toggle and draw the fishing animation on the slave OLED at the
    // configured frame rate. The sleep timer is updated when the user is
    // typing to keep the display active.
    void animate_fishing(void) {
        current_ghost_frame = (current_ghost_frame + 1) % 2; // alternate frames
        oled_write_raw_P(fishing[abs(1 - current_ghost_frame)], 416);
    }
    if (timer_elapsed32(anim_fishing_timer) > ANIM_FRAME_DURATION) {
        anim_fishing_timer = timer_read32();
        animate_fishing();
        if (current_wpm != 0) {
            anim_fishing_sleep = timer_read32();
        }
    }
}
