// RGB behavior runtime for this keymap.
#include QMK_KEYBOARD_H
#include "rgb.h"
#include "layer_names.h"

bool rgb_is_allowed = false;

#ifdef RGBLIGHT_ENABLE
void matrix_scan_user(void) {
    if (!rgb_is_allowed) {
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
