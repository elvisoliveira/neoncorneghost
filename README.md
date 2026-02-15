# neoncorneghost

Flash for pro micro-based builds

```console
qmk flash -kb crkbd/rev1/common -km neoncorneghost -bl avrdude-split-left
qmk flash -kb crkbd/rev1/common -km neoncorneghost -bl avrdude-split-right
```

Flash for Elite C or dfu bootloader builds

```console
qmk flash -kb crkbd/rev1/common -km neoncorneghost -bl dfu-split-left
qmk flash -kb crkbd/rev1/common -km neoncorneghost -bl dfu-split-right
```

These commands can be mixed if, for example, you have an Elite C on the left and a pro micro on the right.

Note: its not recommended to try to use QMK Toolbox

## Getting started

A few quick notes to build, edit, and flash this keymap.

Prerequisites
- Install QMK (qmk_firmware) and the toolchain for your platform. Make sure `qmk` is on your PATH.

Build (compile only)
- Compile the firmware without flashing to check for errors:

```console
qmk compile -kb crkbd/rev1/common -km neoncorneghost
```

Flash (example)
- The repo already includes examples for Pro Micro and Elite C/DFU flashing. Example using `qmk flash`:

```console
qmk flash -kb crkbd/rev1/common -km neoncorneghost -bl avrdude-split-left
qmk flash -kb crkbd/rev1/common -km neoncorneghost -bl avrdude-split-right
```

Editing the keymap
- The layer definitions are moved into `keymap_layers.h` to make layout edits easier. Edit that file to change layer contents.
- `keymap.c` contains the runtime logic (layers, OLED rendering, RGB handling). Keep big bitmap assets in the animation headers described below.

OLED animations and assets
- Large PROGMEM bitmaps are stored in the headers:
	- `oled_ghost_anims.h` — ghost animation frames used by the master OLED
	- `oled_fishing_anims.h` — fishing animation frames used by the slave OLED
- Edit these files if you want to change or add animations. They are included inside the OLED-rendering functions so they remain function-local.

Troubleshooting
- If the build fails with missing headers, ensure the files above are present in the same directory as `keymap.c`.
- If flashing fails, check bootloader mode for the controller (Pro Micro / Elite C) and use the corresponding `-bl` option shown above.

If you'd like, I can also add a short development guide for modifying the OLED frames or add a Makefile/CI step to build automatically.
