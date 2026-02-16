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
 * Timing: a ghost_anim_timer controls the frame rate (ANIM_FRAME_MS
 * ms per frame). When the user is typing (non-zero WPM) the animation sleep
 * timers are reset so the display remains active.
 */

// Large ghost animation frames moved to separate header to reduce
// keymap.c size and improve maintainability. This header defines
// the following function-local PROGMEM arrays:
//   hide[], shyguy[], eerie[], scare[], troll[], laugh[]
#include "oled_anims_master.h"

// The slave (secondary) OLED displays a larger animation.
// This array contains two SLAVE_FRAME_BYTES frames (32x100 pixels represented as
// pages) stored in PROGMEM. The animate_slave() helper toggles
// anim_frame_idx and writes the appropriate frame to the slave OLED.
// Slave animation frames moved to a separate header to reduce
// keymap.c size. Included here so `slave_anim` remains function-local.
#include "oled_anims_slave.h"

 static void animate_ghost(void) {
    anim_frame_idx = (anim_frame_idx + 1) % 2; // alternate between frame 0 and 1
    // hide animation
    // oled_write_raw_P(hide[abs(1 - anim_frame_idx)], MASTER_FRAME_BYTES); // hide
    if (IS_LAYER_ON(_LOWER)) {
        if (IS_LAYER_OFF(_TUNE)) {
            // LOWER layer active, normal shyguy animation
            oled_write_raw_P(shyguy[abs(1 - anim_frame_idx)], MASTER_FRAME_BYTES);
        }
        else {
            // LOWER + TUNE -> laughing animation
            oled_write_raw_P(laugh[abs(1 - anim_frame_idx)], MASTER_FRAME_BYTES);
        }
    }
    else if (IS_LAYER_ON(_RAISE)) {
        if (IS_LAYER_OFF(_TUNE)) {
            // RAISE layer active, eerie animation
            oled_write_raw_P(eerie[abs(1 - anim_frame_idx)], MASTER_FRAME_BYTES);
        }
        else {
            // RAISE + TUNE -> laughing animation
            oled_write_raw_P(laugh[abs(1 - anim_frame_idx)], MASTER_FRAME_BYTES);
        }
    }
    else if ((keyboard_report->mods & MOD_BIT(KC_LSFT)) || (keyboard_report->mods & MOD_BIT(KC_RSFT))) {
        // Shift held -> scare animation
        oled_write_raw_P(scare[abs(1 - anim_frame_idx)], MASTER_FRAME_BYTES);
    }
    else if (keyboard_report->mods & MOD_BIT(KC_RALT)) {
        // Right Alt held -> troll animation
        oled_write_raw_P(troll[abs(1 - anim_frame_idx)], MASTER_FRAME_BYTES);
    }
    else {
        // Default: laughing animation
        oled_write_raw_P(laugh[abs(1 - anim_frame_idx)], MASTER_FRAME_BYTES);
    }
}

static void animate_slave(void) {
    anim_frame_idx = (anim_frame_idx + 1) % 2; // alternate frames
    oled_write_raw_P(slave_anim[abs(1 - anim_frame_idx)], SLAVE_FRAME_BYTES);
}

static void master_render_ghost(void) {
    // Update the animation frame at the configured frame duration.
    if (timer_elapsed32(ghost_anim_timer) > ANIM_FRAME_MS) {
        ghost_anim_timer = timer_read32(); // reset the frame timer
        animate_ghost(); // draw the next frame
    }
}

static void slave_render_ghost(void) {
    // Toggle and draw the animation on the slave OLED at the
    // configured frame rate. The sleep timer is updated when the user is
    // typing to keep the display active.
    if (timer_elapsed32(slave_anim_timer) > ANIM_FRAME_MS) {
        slave_anim_timer = timer_read32();
        animate_slave();
    }
}
