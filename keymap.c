// kb: crkbd
// km: neoncorneghost

#include QMK_KEYBOARD_H

// Layer indexes for this keymap. Each symbolic name maps to a numeric layer
// index used by QMK. Use these names with layer_on/layer_off and
// layer_state_is to check or change the active layer.
enum corne_layers {
    _BASE,
    _LOWER,
    _RAISE,
    _TUNE
};

// Custom keycodes specific to this keymap. We start at SAFE_RANGE so we
// don't conflict with existing QMK keycodes. These are handled in
// process_record_user() below to implement custom behaviors (layer toggles,
// multi-key macros, etc.).
enum custom_keycodes {
    LOWER = SAFE_RANGE,
    OLED_TG,
    RAISE,
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
//     [SUPER] = ACTION_TAP_DANCE_DOUBLE(KC_APPLICATION, LOWER),
//     [ALT] = ACTION_TAP_DANCE_DOUBLE(KC_RALT, KC_LALT)
// };

// Keymap data moved to external header for easier editing
#include "keymap_layers.h"

// ---------------------- constants -----------------------------

#define ANIM_SIZE_GHOST 128 // bytes per ghost animation frame (stored in PROGMEM)
#define ANIM_FRAME_DURATION 200 // milliseconds to show each animation frame
#define OLED_SLEEP_TIMEOUT_MS 120000

uint32_t anim_ghost_timer;
uint32_t anim_fishing_timer;
uint8_t current_ghost_frame = 0;

uint8_t hue_value;
uint8_t sat_value;
uint8_t val_value;
uint8_t mode_value;

char hue_str[4];
char sat_str[4];
char val_str[4];
char mode_str[4];
char wpm_str[4];

static bool rgb_allowed = false;
static bool oled_enabled = true;
// --------------------------------------------------------------

void keyboard_post_init_user(void) {
    rgblight_disable_noeeprom();
}

// process_record_user handles custom keycodes defined earlier (LOWER,
// RAISE, HUI, HUD) and is called on
// every key event. Return false from a case to indicate that we've handled
// the key and QMK should not perform default processing for it. Typical
// uses here are:
// - Toggle layers (layer_on/layer_off and update_tri_layer for tri-layer)
// - Emit multiple keycodes (register_code/unregister_code) to produce
//   small macros (e.g., quote + space)
// - Adjust runtime variables
bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    switch (keycode) {
        case OLED_TG:
            if (record->event.pressed) {
                oled_enabled = !oled_enabled;
            }
            return true;
        case UG_TOGG:
            // if RGBLIGHT is enabled, we want to allow it to be turned on by the UG_TOGG keycode
            if (record->event.pressed) {
                rgb_allowed = true;
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
        // LOWER: momentary LOWER layer with tri-layer update
        case LOWER:
            if (record->event.pressed) {
                layer_on(_LOWER);
                update_tri_layer(_LOWER, _RAISE, _TUNE);
            }
            else {
                layer_off(_LOWER);
                update_tri_layer(_LOWER, _RAISE, _TUNE);
            }
            return false;
        // RAISE: momentary RAISE layer with tri-layer update
        case RAISE:
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

#ifdef OLED_ENABLE // only compile OLED code if OLED is enabled in rules.mk
#include "ghost.h"
#include "oled_render.h"

// Initialize the OLED rotation for each half. Returning a rotation here
// tells the OLED driver how to orient the display. This board uses the
// same rotation (270 degrees) for both master and slave so the graphics
// align correctly with the physical mounting.
oled_rotation_t oled_init_user(oled_rotation_t rotation) {
    (void)rotation;
    return OLED_ROTATION_270;
}

// OLED characteristics: 128x32 pixels. Internally the driver operates in
// 8-pixel-high pages (so 128/8 = 16 pages across 128 pixels). This task
// is called periodically by QMK and is responsible for drawing content on
// each OLED.
bool oled_task_user(void) {
    mode_value = rgblight_get_mode();
    hue_value = rgblight_get_hue();
    sat_value = rgblight_get_sat();
    val_value = rgblight_get_val();

    #ifdef OLED_ENABLE
    if (!oled_enabled) {
        if (is_oled_on()) oled_off();
        return false;
    }
    #endif // OLED_ENABLE

    // master OLED: ghost animation + status
    if (is_keyboard_master()) {
        oled_set_cursor(0,1);
        master_render_ghost();
        oled_set_cursor(0,6);
        render_layer();
        oled_set_cursor(0,11);
        render_hsv();
    }
    // slave OLED: fishing animation + mode
    else {
        oled_set_cursor(0,1);
        slave_render_ghost();
        oled_set_cursor(0,13);
        render_mode();
    }

    return false;
}
#endif // OLED_ENABLE

// only compile RGBLIGHT code if RGBLIGHT is enabled in rules.mk
#ifdef RGBLIGHT_ENABLE
void matrix_scan_user(void) {
    if (!rgb_allowed) {
        rgblight_disable_noeeprom();
        return;
    }
    if (rgblight_get_mode() != RGBLIGHT_MODE_STATIC_LIGHT) {
        return;
    }
    switch (get_highest_layer(layer_state)) {
        case _TUNE:
            rgblight_sethsv(HSV_RED);
            break;
        case _RAISE:
            rgblight_sethsv(HSV_GREEN);
            break;
        case _LOWER:
            rgblight_sethsv(HSV_BLUE);
            break;
        case _BASE:
        default:
            rgblight_sethsv(HSV_WHITE);
            break;
    }
}
#endif // RGBLIGHT_ENABLE
