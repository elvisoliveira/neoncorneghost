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
- The layer definitions are moved into `layers.h` to make layout edits easier. Edit that file to change layer contents.
- `keymap.c` contains runtime logic and delegates OLED/RGB behavior to `oled.c` and `rgb.c`. Keep big bitmap assets in the animation headers described below.
- After changing `layers.h`, run the formatter tool to tidy the grid alignment so the keyboard layout is easier to read in code:

```console
./tools/tidy_keymap_layers.py ./layers.h
```

OLED animations and assets
- Large PROGMEM bitmaps are stored in:
	- `oled_anims_master.h` — animation frames used by the master OLED
	- `oled_anims_slave.h` — animation frames used by the slave OLED
- Edit these files if you want to change or add animations. They are included inside the OLED-rendering functions so arrays remain function-local.

Troubleshooting
- If the build fails with missing headers, ensure the files above are present in the same directory as `keymap.c`.
- If flashing fails, check bootloader mode for the controller (Pro Micro / Elite C) and use the corresponding `-bl` option shown above.

## Exporting a Keymap

You can export your current layout by generating `keymap.json`:

```console
qmk c2json -kb crkbd/rev1/common -km neoncorneghost ./layers.h  > keymap.json
```

## Layer State Daemon

This watcher changes your desktop wallpaper whenever the active keyboard layer changes.  
The goal is to make layer state visible at a glance, so you always know which layout is active without guessing.
It also watches `layers.h` and automatically runs `./tools/tidy_keymap_layers.py ./layers.h` plus `./tools/generate_keymap_assets.sh` when the file changes.

Use it after generating the keymap wallpapers:

```console
./tools/layer_state_daemon.sh
```

## Generate Wallpaper Assets

This script generates wallpaper images from your current keymap layers (`layers.h`).
Each generated image represents one layer and is used by the watcher script.

Typical workflow:
- Edit `layers.h`
- Reformat with `./tools/tidy_keymap_layers.py ./layers.h`
- Generate updated wallpapers with:

```console
./tools/generate_keymap_assets.sh
```

- Run `./tools/layer_state_daemon.sh` to switch wallpapers automatically as layers change
