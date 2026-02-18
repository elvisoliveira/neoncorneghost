#!/usr/bin/env bash
# ============================================================
#  wallpaper_watcher.sh
#  Listens for physical key press/release events via xinput XI2
#  and sets wallpapers accordingly.
#  wallpapers accordingly using xwallpaper.
#
#  Usage: bash wallpaper_watcher.sh
#  Stop:  Ctrl+C
# ============================================================

# ---[ CONFIGURATION ]----------------------------------------

XINPUT_DEVICE_ID=14

KEYMAP_DIR="./tools/keymap"

DEFAULT_WALLPAPER="$KEYMAP_DIR/keymap_L0.png"

declare -A PRESS_WALLPAPERS=(
    [191]="$KEYMAP_DIR/keymap_L2.png"
    [192]="$KEYMAP_DIR/keymap_L1.png"
    [193]="$KEYMAP_DIR/keymap_L3.png"
)

# ---[ END CONFIGURATION ]------------------------------------


set_wallpaper() {
    local image="$1"
    if [[ ! -f "$image" ]]; then
        echo "[warn] Image not found, skipping: $image" >&2
        return
    fi
    xwallpaper --output eDP1 --center "$image"
    echo "[$(date +%H:%M:%S)] Wallpaper set to: $(basename "$image")"
}

echo "=== Wallpaper Watcher Started ==="
echo "  Device ID : $XINPUT_DEVICE_ID"
echo "  Default   : $DEFAULT_WALLPAPER"
echo "  Watching keycodes: ${!PRESS_WALLPAPERS[*]}"
echo "  Press Ctrl+C to stop."
echo ""

# Set the default wallpaper on start
set_wallpaper "$DEFAULT_WALLPAPER"

handle_key_event() {
    local event="$1"
    local keycode="$2"

    if [[ -z "${PRESS_WALLPAPERS[$keycode]+_}" ]]; then
        return
    fi

    if [[ "$event" == "press" ]]; then
        set_wallpaper "${PRESS_WALLPAPERS[$keycode]}"
    elif [[ "$event" == "release" ]]; then
        set_wallpaper "$DEFAULT_WALLPAPER"
    fi
}

# Use XI2 raw events to avoid X autorepeat press/release spam.
# Example block:
#   EVENT type 14 (RawKeyPress)
#       device: 14 (14)
#       detail: 191
current_event=""
current_device=""
current_keycode=""

xinput test-xi2 --root "$XINPUT_DEVICE_ID" | while read -r line; do
    if [[ "$line" == *"(RawKeyPress)"* ]]; then
        current_event="press"
        current_device=""
        current_keycode=""
        continue
    fi

    if [[ "$line" == *"(RawKeyRelease)"* ]]; then
        current_event="release"
        current_device=""
        current_keycode=""
        continue
    fi

    if [[ -z "$current_event" ]]; then
        continue
    fi

    if [[ "$line" =~ ^[[:space:]]*device:\ ([0-9]+) ]]; then
        current_device="${BASH_REMATCH[1]}"
    elif [[ "$line" =~ ^[[:space:]]*detail:\ ([0-9]+) ]]; then
        current_keycode="${BASH_REMATCH[1]}"
    fi

    if [[ -n "$current_device" && -n "$current_keycode" ]]; then
        if [[ "$current_device" == "$XINPUT_DEVICE_ID" ]]; then
            handle_key_event "$current_event" "$current_keycode"
        fi
        current_event=""
        current_device=""
        current_keycode=""
    fi
done
