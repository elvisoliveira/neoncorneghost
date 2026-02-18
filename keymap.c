// kb: crkbd
// km: neoncorneghost

#include QMK_KEYBOARD_H
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
    TG_OLED,
    HUI,
    HUD
};

enum td_keycodes {
    CAPLOCK,
    SUPER,
    ALT
};

// Tap Dance actions. Tap Dance lets a single physical key perform different
// actions depending on how many times it is tapped in quick succession.
// The mapping below defines three tap-dance behaviors used in the keymap:
// - CAPLOCK:  single tap -> Left Shift, double tap -> Caps Lock
// - SUPER:    single tap -> Application/Menu key, double tap -> activate LOWER layer
// - ALT:      single tap -> Right Alt, double tap -> Left Alt
// qk_tap_dance_action_t tap_dance_actions[] = {
//     [CAPLOCK] = ACTION_TAP_DANCE_DOUBLE(KC_LSFT, KC_CAPS),
//     [SUPER] = ACTION_TAP_DANCE_DOUBLE(KC_APPLICATION, KC_LOWER),
//     [ALT] = ACTION_TAP_DANCE_DOUBLE(KC_RALT, KC_LALT)
// };

// Keymap data moved to external header for easier editing
#include "layers.h"

void keyboard_post_init_user(void) {
    rgblight_disable_noeeprom();
}

static uint16_t layer_hold_keycode(uint8_t layer) {
    switch (layer) {
        case _RAISE:
            return KC_F13;
        case _LOWER:
            return KC_F14;
        case _TUNE:
            return KC_F15;
        default:
            return KC_NO;
    }
}

layer_state_t layer_state_set_user(layer_state_t state) {
    static uint8_t previous_layer = 0xFF;
    uint8_t layer = get_highest_layer(state);

    uint16_t previous_keycode = layer_hold_keycode(previous_layer);
    uint16_t current_keycode  = layer_hold_keycode(layer);

    if (previous_layer != layer) {
        if (previous_keycode != KC_NO) {
            unregister_code(previous_keycode);
        }
        if (current_keycode != KC_NO) {
            register_code(current_keycode);
        }
    }

    previous_layer = layer;
    return state;
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
        // HUI: increase RGB hue (no EEPROM write)
        case HUI:
            if (record->event.pressed) {
                rgblight_increase_hue_noeeprom();
            }
            return false;
        // HUD: decrease RGB hue (no EEPROM write)
        case HUD:
            if (record->event.pressed) {
                rgblight_decrease_hue_noeeprom();
            }
            return false;
        // KC_LOWER: momentary LOWER layer with tri-layer update
        case KC_LOWER:
            if (record->event.pressed) {
                layer_on(_LOWER);
                update_tri_layer(_LOWER, _RAISE, _TUNE);
            }
            else {
                layer_off(_LOWER);
                update_tri_layer(_LOWER, _RAISE, _TUNE);
            }
            return false;
        // KC_RAISE: momentary RAISE layer with tri-layer update
        case KC_RAISE:
            if (record->event.pressed) {
                layer_on(_RAISE);
                update_tri_layer(_LOWER, _RAISE, _TUNE);
            }
            else {
                layer_off(_RAISE);
                update_tri_layer(_LOWER, _RAISE, _TUNE);
            }
            return false;
    }
    return true;
}
