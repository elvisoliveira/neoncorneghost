// OLED rendering helpers kept separate to reduce keymap.c size.
#pragma once

static void format_3digits(uint8_t value, char out[4]) {
    out[3] = '\0';
    out[2] = '0' + value % 10;
    value /= 10;
    out[1] = '0' + value % 10;
    out[0] = '0' + value / 10;
}

static void render_layer(void) {
    oled_write_P(PSTR("RAISE"), layer_state_is(_RAISE) && !layer_state_is(_TUNE));
    oled_write_P(PSTR("BASE\n"), layer_state_is(_BASE));
    oled_write_P(PSTR("LOWER"), layer_state_is(_LOWER) && !layer_state_is(_TUNE));
    oled_write_P(PSTR("TUNE\n"), layer_state_is(_TUNE));
}

static void render_mode(void) {
    // Format the RGB mode number as a 3-digit ASCII string and print it.
    format_3digits(rgb_mode, mode_buf);
    oled_write("MODE  ", false);
    oled_write(mode_buf, false);
}

// Print current Hue, Saturation and Value (HSV) to the OLED.
static void render_hsv(void) {
    oled_write("H ", false);
    format_3digits(hue, hue_buf);
    oled_write(hue_buf, false);

    oled_write("S ", false);
    format_3digits(sat, sat_buf);
    oled_write(sat_buf, false);

    oled_write("V ", false);
    format_3digits(val, val_buf);
    oled_write(val_buf, false);
}
