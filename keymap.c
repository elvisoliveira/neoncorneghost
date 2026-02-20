// kb: crkbd
// km: neoncorneghost

#include QMK_KEYBOARD_H
#include "keymap_brazilian_abnt2.h"
#include "oled.h"
#include "rgb.h"
#include "layer_names.h"

// Custom keycodes specific to this keymap. We start at SAFE_RANGE so we
// don't conflict with existing QMK keycodes. These are handled in
// process_record_user() below to implement custom behaviors (layer toggles,
// multi-key macros, etc.).
enum custom_keycodes {
    KC_LOWER = SAFE_RANGE,
    KC_RAISE,
    TG_OLED
};

enum td_keycodes {
    TD_C_CED,
    TD_A_ACC,
    TD_E_ACC,
    TD_I_ACC,
    TD_O_ACC,
    TD_U_ACC,
};

#define ACCENT_FN(LETTER) \
    void LETTER##_accents(tap_dance_state_t *state, void *user_data) { \
        if (state->count == 2) tap_code(KC_LBRC); \
        else if (state->count == 3) tap_code(KC_QUOT); \
        tap_code(KC_##LETTER); \
    }

ACCENT_FN(A)
ACCENT_FN(E)
ACCENT_FN(I)
ACCENT_FN(O)
ACCENT_FN(U)

// Tap Dance actions. Tap Dance lets a single physical key perform different
// actions depending on how many times it is tapped in quick succession.
tap_dance_action_t tap_dance_actions[] = {
    [TD_C_CED] = ACTION_TAP_DANCE_DOUBLE(BR_C, BR_CCED),
    [TD_A_ACC] = ACTION_TAP_DANCE_FN(A_accents),
    [TD_E_ACC] = ACTION_TAP_DANCE_FN(E_accents),
    [TD_I_ACC] = ACTION_TAP_DANCE_FN(I_accents),
    [TD_O_ACC] = ACTION_TAP_DANCE_FN(O_accents),
    [TD_U_ACC] = ACTION_TAP_DANCE_FN(U_accents),
};

// Keymap data moved to external header for easier editing
#include "keymap.h"

void keyboard_post_init_user(void) {
    rgblight_disable_noeeprom();
}

// process_record_user handles custom keycodes defined earlier (KC_LOWER,
// KC_RAISE, HUI, HUD) and is called on
// every key event. Return false from a case to indicate that we've handled
// the key and QMK should not perform default processing for it. Typical
// uses here are:
// - Toggle layers (layer_on/layer_off and update_tri_layer for tri-layer)
// - Emit multiple keycodes (register_code/unregister_code) to produce
//   small macros (e.g., quote + space)
// - Adjust runtime variables
bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    switch (keycode) {
        case TG_OLED:
            if (record->event.pressed) {
                oled_is_enabled = !oled_is_enabled;
            }
            return true;
        case UG_TOGG:
            // if RGBLIGHT is enabled, we want to allow it to be turned on by the UG_TOGG keycode
            if (record->event.pressed) {
                rgb_is_allowed = true;
            }
            return true;
        // KC_LOWER: momentary LOWER layer with tri-layer update
        case KC_LOWER:
            if (record->event.pressed) {
                layer_on(_LOWER);
                update_tri_layer(_LOWER, _RAISE, _TUNE);
                register_code(KC_F14);
            }
            else {
                layer_off(_LOWER);
                update_tri_layer(_LOWER, _RAISE, _TUNE);
                unregister_code(KC_F14);
            }
            return false;
        // KC_RAISE: momentary RAISE layer with tri-layer update
        case KC_RAISE:
            if (record->event.pressed) {
                layer_on(_RAISE);
                update_tri_layer(_LOWER, _RAISE, _TUNE);
                register_code(KC_F13);
            }
            else {
                layer_off(_RAISE);
                update_tri_layer(_LOWER, _RAISE, _TUNE);
                unregister_code(KC_F13);
            }
            return false;
    }
    return true;
}
