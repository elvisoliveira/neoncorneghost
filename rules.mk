# rules.mk no esta en lenguaje C

# kb: crkbd
# km: neoncorneghost

OLED_ENABLE = yes
OLED_DRIVER = SSD1306
RGBLIGHT_ENABLE = yes
TAP_DANCE_ENABLE = yes # necesario para ocupar el doble pulso (tap dance) en el keymap
EXTRAKEY_ENABLE = no # EXTRAKEY_ENABLE es necesario para ocupar los keycodes KC_MUTE, KC_VOLD y KC_VOLU
MOUSEKEY_ENABLE = no
WPM_ENABLE = yes # necesario para ocupar get_current_wpm() obtener el valor de words per minute
LTO_ENABLE = yes # Link Time Optimization reduce size al hex
EXTRAFLAGS += -flto # esta linea reduce considerable size al hex
CONSOLE_ENABLE = no