// Keymap layers separated into their own file to make configuration easier.
// This header defines the `keymaps` PROGMEM array used by QMK. Keeping the
// large bitmap and layout data in a separate file makes `keymap.c` easier
// to read and edit.
#pragma once

// Named aliases for right-alt (AltGr) combos used in the keymap.
// Using macros makes it easier to change the combos in one place.
#define RALT_1    RALT(KC_1)
#define RALT_SLSH RALT(KC_SLSH)
#define META      MT(MOD_LALT, KC_ESC)

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    [_BASE] = LAYOUT_split_3x6_3(
    //|----------+----------+----------+----------+----------+----------|                        |----------+----------+----------+----------+----------+----------|
           KC_TAB,      KC_Q,      KC_W,      KC_E,      KC_R,      KC_T,                               KC_Y,      KC_U,      KC_I,      KC_O,      KC_P,   KC_BSPC,
    //|----------+----------+----------+----------+----------+----------|                        |----------+----------+----------+----------+----------+----------|
          KC_LSFT,      KC_A,      KC_S,      KC_D,      KC_F,      KC_G,                               KC_H,      KC_J,      KC_K,      KC_L,   KC_SCLN,   XXXXXXX,
    //|----------+----------+----------+----------+----------+----------|                        |----------+----------+----------+----------+----------+----------|
          KC_LCTL,      KC_Z,      KC_X,      KC_C,      KC_V,      KC_B,                               KC_N,      KC_M,   KC_COMM,    KC_DOT,   KC_SLSH,    KC_ESC,
    //|----------+----------+----------+----------+----------+----------+----------`  `----------+----------+----------+----------+----------+----------+----------|
                                                         META,     LOWER,    KC_SPC,       KC_ENT,     RAISE,   TD(ALT)
    //                                            |----------+----------+----------|  |----------+----------+----------|
    ),

    [_LOWER] = LAYOUT_split_3x6_3(
    //|----------+----------+----------+----------+----------+----------|                        |----------+----------+----------+----------+----------+----------|
           KC_TAB,      KC_1,      KC_2,      KC_3,      KC_4,      KC_5,                               KC_6,      KC_7,      KC_8,      KC_9,      KC_0,   KC_BSPC,
    //|----------+----------+----------+----------+----------+----------|                        |----------+----------+----------+----------+----------+----------|
          KC_LSFT,   KC_PGDN,   KC_PGUP,   KC_PSCR,   KC_LGUI,   XXXXXXX,                            KC_HOME,   KC_LEFT,   KC_DOWN,     KC_UP,  KC_RIGHT,    KC_END,
    //|----------+----------+----------+----------+----------+----------|                        |----------+----------+----------+----------+----------+----------|
          KC_LCTL,     KC_F1,     KC_F2,     KC_F3,     KC_F4,     KC_F5,                              KC_F6,     KC_F7,     KC_F8,     KC_F9,    KC_F10,    KC_F11,
    //|----------+----------+----------+----------+----------+----------+----------`  `----------+----------+----------+----------+----------+----------+----------|
                                                       KC_DEL,   _______,    KC_SPC,       KC_ENT,     RAISE,   TD(ALT)
    //                                            |----------+----------+----------|  |----------+----------+----------|
    ),

    [_RAISE] = LAYOUT_split_3x6_3(
    //|----------+----------+----------+----------+----------+----------|                        |----------+----------+----------+----------+----------+----------|
           KC_TAB,   KC_EXLM,     KC_AT,   KC_HASH,    KC_DLR,   KC_PERC,                            XXXXXXX,   KC_AMPR,   KC_ASTR,   KC_LPRN,   KC_RPRN,   KC_BSPC,
    //|----------+----------+----------+----------+----------+----------|                        |----------+----------+----------+----------+----------+----------|
          KC_LSFT,   KC_LBRC,   KC_RBRC,   KC_LCBR,   KC_RCBR,   KC_BSLS,                           KC_EQUAL,   KC_PLUS,   KC_MINS,   KC_ASTR,   KC_PSLS,   KC_UNDS,
    //|----------+----------+----------+----------+----------+----------|                        |----------+----------+----------+----------+----------+----------|
          KC_LCTL,    RALT_1, RALT_SLSH,   XXXXXXX,   XXXXXXX,   XXXXXXX,                            XXXXXXX,   XXXXXXX,   XXXXXXX,   XXXXXXX,   XXXXXXX,   XXXXXXX,
    //|----------+----------+----------+----------+----------+----------+----------`  `----------+----------+----------+----------+----------+----------+----------|
                                                       KC_DEL,     SUPER,    KC_SPC,       KC_ENT,   _______,   TD(ALT)
    //                                            |----------+----------+----------|  |----------+----------+----------|
    ),

    [_TUNE] = LAYOUT_split_3x6_3(
    //|----------+----------+----------+----------+----------+----------|                        |----------+----------+----------+----------+----------+----------|
          UG_TOGG,   RGB_M_P,   RGB_M_B,   RGB_M_R,  RGB_M_SW,   XXXXXXX,                            XXXXXXX,   XXXXXXX,   XXXXXXX,   XXXXXXX,   XXXXXXX,   OLED_TG,
    //|----------+----------+----------+----------+----------+----------|                        |----------+----------+----------+----------+----------+----------|
          RGB_M_G,       HUI,   XXXXXXX,   XXXXXXX,   XXXXXXX,   XXXXXXX,                            XXXXXXX,   XXXXXXX,   XXXXXXX,   XXXXXXX,   XXXXXXX,   XXXXXXX,
    //|----------+----------+----------+----------+----------+----------|                        |----------+----------+----------+----------+----------+----------|
          XXXXXXX,       HUD,   XXXXXXX,   XXXXXXX,   XXXXXXX,   XXXXXXX,                            XXXXXXX,   XXXXXXX,   XXXXXXX,   XXXXXXX,   XXXXXXX,   XXXXXXX,
    //|----------+----------+----------+----------+----------+----------+----------`  `----------+----------+----------+----------+----------+----------+----------|
                                                       KC_DEL,   _______,    KC_SPC,       KC_ENT,   _______,   TD(ALT)
    //                                            |----------+----------+----------|  |----------+----------+----------|
    )
};
