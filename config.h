// kb: crkbd
// km: neoncorneghost
#pragma once

// #define USE_MATRIX_I2C
// #define MASTER_LEFT
// #define MASTER_RIGHT
// #define SPLIT_USB_DETECT 
#define EE_HANDS
// #define USE_SERIAL_PD2
// #define TAPPING_FORCE_HOLD
#define TAPPING_TERM 400 // tap-dance double-tap must happen within 400 ms
#define OLED_BRIGHTNESS 120
#define OLED_DISABLE_TIMEOUT
#define SPLIT_OLED_ENABLE
#define SPLIT_WPM_ENABLE

#ifdef RGBLIGHT_ENABLE // only if RGBLIGHT_ENABLE = yes in rules.mk
    // #undef RGBLED_NUM
    // #define RGBLED_NUM 54 // number of LEDs in the keyboard
    #undef RGBLED_SPLIT
    #define RGBLED_SPLIT \
        {27,27}
    #define RGBLIGHT_EFFECT_BREATHING // enable breathing effect
    #define RGBLIGHT_EFFECT_RAINBOW_MOOD // enable rainbow mood effect
    #define RGBLIGHT_EFFECT_RAINBOW_SWIRL
    // #undef RGBLIGHT_LIMIT_VAL
    // #define RGBLIGHT_LIMIT_VAL 110 // max Value in HSV
    #define RGBLIGHT_HUE_STEP 5 // step size for Hue changes
    #define RGBLIGHT_SAT_STEP 17 // step size for Saturation changes
    #define RGBLIGHT_VAL_STEP 17 // step size for Value changes
#endif

#define OLED_FONT_H "keyboards/crkbd/lib/glcdfont.c"
#define NO_ACTION_MACRO // enabled because EXTRAFLAGS += -flto in rules.mk
#define NO_ACTION_FUNCTION // enabled because EXTRAFLAGS += -flto in rules.mk
