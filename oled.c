// OLED rendering and animation runtime for this keymap.
#include QMK_KEYBOARD_H
#include "oled.h"
#include "layer_names.h"

bool oled_is_enabled = true;

#ifdef OLED_ENABLE
// ---------------------- constants -----------------------------

#define MASTER_FRAME_BYTES 128 // bytes per master animation frame (stored in PROGMEM)
#define SLAVE_FRAME_BYTES 416 // bytes per slave animation frame (stored in PROGMEM)
#define ANIM_FRAME_MS 200 // milliseconds to show each animation frame

static uint32_t ghost_anim_timer;
static uint32_t slave_anim_timer;
static uint8_t anim_frame_idx = 0;

static uint8_t hue;
static uint8_t sat;
static uint8_t val;
static uint8_t rgb_mode;

static char hue_buf[4];
static char sat_buf[4];
static char val_buf[4];
static char mode_buf[4];
// --------------------------------------------------------------

#include "oled_render.h"
#include "ghost.h"

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
    rgb_mode = rgblight_get_mode();
    hue = rgblight_get_hue();
    sat = rgblight_get_sat();
    val = rgblight_get_val();

    if (!oled_is_enabled) {
        if (is_oled_on()) oled_off();
        return false;
    }

    // master OLED: ghost animation + status
    if (is_keyboard_master()) {
        oled_set_cursor(0,1);
        master_render_ghost();
        oled_set_cursor(0,6);
        render_layer();
        oled_set_cursor(0,11);
        render_hsv();
    }
    // slave OLED: animation + mode
    else {
        oled_set_cursor(0,1);
        slave_render_ghost();
        oled_set_cursor(0,13);
        render_mode();
    }

    return false;
}
#endif // OLED_ENABLE
