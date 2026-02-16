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
    RAISE,
    QUOTE,
    CIRC,
    WAVE,
    VERTICALBAR,
    GRAVE,
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

// ---------------------- variables -----------------------------

#define ANIM_SIZE_GHOST 128 // number of bytes per ghost animation frame (stored in PROGMEM)
#define ANIM_FRAME_DURATION 200 // milliseconds to show each animation frame

led_t led_usb_state;

uint32_t anim_ghost_timer;
uint32_t anim_ghost_sleep;
uint32_t anim_fishing_timer;
uint32_t anim_fishing_sleep;

uint8_t current_wpm = 0;
uint8_t current_ghost_frame = 0;
uint8_t hue_value;
uint8_t sat_value;
uint8_t val_value;
uint8_t mode_value;
uint8_t current_hue;
uint8_t current_val;

char hue_str[4];
char sat_str[4];
char val_str[4];
char mode_str[4];
char wpm_str[4];

// --------------------------------------------------------------

// Runs once right after the keyboard finishes initializing. Use this to
// read initial hardware state and initialize runtime variables that depend
// on that state. Here we capture the current RGB hue and set the working
// brightness limit.
void keyboard_post_init_user(void) {
    current_hue = rgblight_get_hue();
    // Use the configured brightness limit so we don't exceed it later.
    current_val = RGBLIGHT_LIMIT_VAL;
}

// layer_state_set_user is a QMK hook called whenever the layer state
// changes. The "highest" active layer (the one with highest index that is
// currently enabled) is used to decide the RGB color. We avoid changing
// RGB when Caps Lock is on because led_set_user() forces a Caps Lock
// indicator color separately.
layer_state_t layer_state_set_user(layer_state_t state) {
    switch (get_highest_layer(state)) {
        case _TUNE:
            // TUNE layer uses a white RGB for clarity.
            if (!host_keyboard_led_state().caps_lock) {
                rgblight_sethsv(HSV_WHITE);
            }
            break;
        case _RAISE:
            // RAISE layer uses a bluish tint.
            if (!host_keyboard_led_state().caps_lock) {
                rgblight_sethsv(245, 255, current_val);
            }
            break;
        case _LOWER:
            // LOWER layer uses teal.
            if (!host_keyboard_led_state().caps_lock) {
                rgblight_sethsv(HSV_TEAL);
            }
            break;
        case _BASE:
            // BASE layer uses the user's selected hue.
            if (!host_keyboard_led_state().caps_lock) {
                rgblight_sethsv(current_hue, 255, current_val);
            }
            break;
    }
    return state;
}

// void led_set_user(uint8_t usb_led) {
//     if (usb_led & (1<<USB_LED_CAPS_LOCK)) {
//         rgblight_sethsv(22, 255, current_val); // yellow for Caps Lock
//     } else { 
//         rgblight_sethsv(current_hue, 255, current_val);
//     }
// }

#ifdef OLED_ENABLE // only compile OLED code if OLED is enabled in rules.mk
#include <stdio.h>
#include "ghost.h"

// Render the textual layer indicator on the OLED. Each line writes the layer
// name and highlights (inverted text) the currently active layer. The
// function uses layer_state_is() and layer_state flags to decide which name
// to highlight. Keep the output compact so it fits on the small OLED.
static void render_layer(void) {
    oled_write_P(PSTR("RAISE"), layer_state_is(_RAISE) && !layer_state_is(_TUNE));
    oled_write_P(PSTR("BASE\n"), layer_state_is(_BASE));
    oled_write_P(PSTR("LOWER"), layer_state_is(_LOWER) && !layer_state_is(_TUNE));
    oled_write_P(PSTR("TUNE\n"), layer_state_is(_TUNE));
}

static void render_mode(void) {
    mode_str[3] = '\0';
    mode_str[2] = '0' + mode_value % 10;
    mode_str[1] = '0' + ( mode_value /= 10) % 10;
    mode_str[0] = '0' + mode_value / 10;
    // Format the RGB mode number as a 3-digit ASCII string and print it.
    // mode_value comes from rgblight_get_mode(). Note: the code below uses
    // the "/=" operator which mutates mode_value. That works here because
    // mode_value is a copy of the global retrieved earlier in oled_task_user(),
    // but be careful if you refactor this code.
    oled_write("MODE ", false);
    oled_write(" ", false);
    oled_write(mode_str, false);
}

// Print current Hue, Saturation and Value (HSV) to the OLED.
// The function converts numeric HSV values into 3-char ASCII strings for
// compact display. Note: the implementation uses in-place division (x /= 10)
// which mutates the source variable; if you need the original values later
// copy them into a temporary variable first.
static void render_hsv(void) {
    oled_write("H ", false);
    hue_str[3] = '\0';
    hue_str[2] = '0' + hue_value % 10;
    hue_str[1] = '0' + ( hue_value /= 10) % 10;
    hue_str[0] = '0' + hue_value / 10;
    oled_write(hue_str, false);

    oled_write("S ", false);
    sat_str[3] = '\0';
    sat_str[2] = '0' + sat_value % 10;
    sat_str[1] = '0' + ( sat_value /= 10) % 10;
    sat_str[0] = '0' + sat_value / 10;
    oled_write(sat_str, false);

    oled_write("V ", false);
    val_str[3] = '\0';
    val_str[2] = '0' + val_value % 10;
    val_str[1] = '0' + ( val_value /= 10) % 10;
    val_str[0] = '0' + val_value / 10;
    oled_write(val_str, false);    
}


/*
static void render_wpm(void) {
    oled_write("WPM\n", false);
    sprintf(wpm_str, "%03d", current_wpm);
    oled_write(wpm_str, false); // print WPM value
}
*/

// Initialize the OLED rotation for each half. Returning a rotation here
// tells the OLED driver how to orient the display. This board uses the
// same rotation (270 degrees) for both master and slave so the graphics
// align correctly with the physical mounting.
oled_rotation_t oled_init_user(oled_rotation_t rotation) {
    if (is_keyboard_master()) {
        return OLED_ROTATION_270;
    }
    else {
        return OLED_ROTATION_270;
    }
    return rotation;
}

// OLED characteristics: 128x32 pixels. Internally the driver operates in
// 8-pixel-high pages (so 128/8 = 16 pages across 128 pixels). This task
// is called periodically by QMK and is responsible for drawing content on
// each OLED.
bool oled_task_user(void) {
    current_wpm = get_current_wpm();
    mode_value = rgblight_get_mode();
    hue_value = rgblight_get_hue();
    sat_value = rgblight_get_sat();
    val_value = rgblight_get_val();
    if ((timer_elapsed32(anim_ghost_sleep) > 120000) && (timer_elapsed32(anim_fishing_sleep) > 120000) && (current_wpm == 0)) {
        if (is_oled_on()) {
            oled_off();
        }
        timer_init();
        return false;
    }
    if (current_wpm != 0 && !is_oled_on()) {
        oled_on();
    }
    led_usb_state = host_keyboard_led_state();
    if (is_keyboard_master()) { // master OLED: ghost animation + status
        oled_set_cursor(0,1);
        master_render_ghost(); // ghost animation
        oled_set_cursor(0,6);
        render_layer(); // layer indicators
        oled_set_cursor(0,11);
        render_hsv(); // HSV values
    }
    else { // slave OLED: fishing animation + mode
        oled_set_cursor(0,1);
        slave_render_ghost();
        oled_set_cursor(0,13);
        render_mode();
    }
    return false;
}

#endif // OLED_ENABLE

// process_record_user handles custom keycodes defined earlier (LOWER,
// RAISE, QUOTE, CIRC, WAVE, VERTICALBAR, GRAVE, HUI, HUD) and is called on
// every key event. Return false from a case to indicate that we've handled
// the key and QMK should not perform default processing for it. Typical
// uses here are:
// - Toggle layers (layer_on/layer_off and update_tri_layer for tri-layer)
// - Emit multiple keycodes (register_code/unregister_code) to produce
//   small macros (e.g., quote + space)
// - Adjust runtime variables (e.g., current_hue for RGB
//   control)
bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    switch (keycode) {
        case HUI:
            if (record->event.pressed) {
                current_hue = current_hue + 5;
                if (current_hue > 255) {
                    current_hue = current_hue - 256;
                }
            }
            else {
                ;
            }
            return false;
        case HUD:
            if (record->event.pressed) {
                current_hue = current_hue - 5;
                if (current_hue < 0) {
                    current_hue = 256 + current_hue;
                }
            }
            else {
                ;
            }
            return false;
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
        case QUOTE:
            if (record->event.pressed) {
                register_code(KC_QUOTE);
                register_code(KC_SPACE);
            }
            else { 
                unregister_code(KC_QUOTE);
                unregister_code(KC_SPACE);
            }
            return false;
        case CIRC:
            if (record->event.pressed) {
                register_code(KC_LSFT);
                register_code(KC_6);
                register_code(KC_SPACE);
            }
            else {
                unregister_code(KC_LSFT);
                unregister_code(KC_6);
                unregister_code(KC_SPACE);
            }
            return false;
        case WAVE:
            if (record->event.pressed) {
                register_code(KC_LSFT);
                register_code(KC_GRAVE);
                register_code(KC_SPACE);
            }
            else {
                unregister_code(KC_LSFT);
                unregister_code(KC_GRAVE);
                unregister_code(KC_SPACE);
            }
            return false;
        case VERTICALBAR:
            if (record->event.pressed) {
                register_code(KC_LSFT);
                register_code(KC_SLASH);
            }
            else {
                unregister_code(KC_LSFT);
                unregister_code(KC_SLASH);
            }
            return false; 
        case GRAVE:
            if (record->event.pressed) {
                register_code(KC_GRAVE);
                register_code(KC_SPACE);
            }
            else {
                unregister_code(KC_GRAVE);
                unregister_code(KC_SPACE);
            }
            return false;
    }
    return true;
}