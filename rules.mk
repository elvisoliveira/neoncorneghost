# rules.mk is not C code

# kb: crkbd
# km: neoncorneghost

OLED_ENABLE = yes
OLED_DRIVER = ssd1306
RGBLIGHT_ENABLE = no
TAP_DANCE_ENABLE = yes # required to use tap dance in the keymap
EXTRAKEY_ENABLE = no # required if you want KC_MUTE / KC_VOLD / KC_VOLU
MOUSEKEY_ENABLE = no
WPM_ENABLE = yes # required to use get_current_wpm()
LTO_ENABLE = yes # Link Time Optimization reduces hex size
EXTRAFLAGS += -flto # further reduces hex size
CONSOLE_ENABLE = no

# Build additional keymap modules
SRC += oled.c rgb.c
