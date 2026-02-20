#!/usr/bin/env bash
# ============================================================
#  layer_state_daemon.sh
#  Listens for physical key press/release events via xinput XI2,
#  updates layer wallpapers, and regenerates keymap assets when
#  watched files change.
#
#  Usage: bash layer_state_daemon.sh XINPUT_DEVICE_ID
#  Stop:  Ctrl+C
# ============================================================

# ---[ CONFIGURATION ]----------------------------------------

XINPUT_DEVICE_ID="${1:-}"

if [[ -z "$XINPUT_DEVICE_ID" ]] || ! [[ "$XINPUT_DEVICE_ID" =~ ^[0-9]+$ ]]; then
    echo "Usage: $0 XINPUT_DEVICE_ID" >&2
    echo "Example: $0 9" >&2
    exit 1
fi

LAYER_ASSETS_DIR="./tools/keymap"
WATCH_FILES=("./keymap.h" "./tools/config.yaml")

BASE_LAYER_WALLPAPER="$LAYER_ASSETS_DIR/keymap_L0.png"
REGEN_RUNNING=0
LAST_HANDLED_STATE=""

declare -A LAYER_KEYCODE_WALLPAPERS=(
    [191]="$LAYER_ASSETS_DIR/keymap_L2.png"
    [192]="$LAYER_ASSETS_DIR/keymap_L1.png"
    [193]="$LAYER_ASSETS_DIR/keymap_L3.png"
)

# ---[ END CONFIGURATION ]------------------------------------


set_layer_wallpaper() {
    local image="$1"
    if [[ ! -f "$image" ]]; then
        echo "[warn] Image not found, skipping: $image" >&2
        return
    fi
    xwallpaper --output eDP1 --center "$image"
    echo "[$(date +%H:%M:%S)] Wallpaper set to: $(basename "$image")"
}

file_hash() {
    local path="$1"
    [[ -f "$path" ]] || return 1
    if command -v sha256sum >/dev/null 2>&1; then
        sha256sum "$path" | awk '{print $1}'
    else
        shasum -a 256 "$path" | awk '{print $1}'
    fi
}

build_watch_state() {
    local state=""
    local path=""
    local hash=""
    for path in "${WATCH_FILES[@]}"; do
        hash=$(file_hash "$path" 2>/dev/null || echo "missing")
        state+="${hash}:"
    done
    printf '%s' "$state"
}

refresh_layer_assets() {
    local force_refresh="${1:-0}"
    if [[ "$REGEN_RUNNING" -eq 1 ]]; then
        return
    fi

    local current_state=""
    current_state=$(build_watch_state)
    if [[ "$force_refresh" -ne 1 && -n "$LAST_HANDLED_STATE" && "$current_state" == "$LAST_HANDLED_STATE" ]]; then
        return
    fi

    REGEN_RUNNING=1
    if [[ "$force_refresh" -eq 1 ]]; then
        echo "[$(date +%H:%M:%S)] Startup refresh: tidying and regenerating assets..."
    else
        echo "[$(date +%H:%M:%S)] Watched file changed; tidying and regenerating assets..."
    fi
    if ./tools/tidy_keymap_layers.py "${WATCH_FILES[0]}" && ./tools/generate_keymap_assets.sh; then
        LAST_HANDLED_STATE="${current_state}"
        echo "[$(date +%H:%M:%S)] Asset regeneration complete."
    else
        echo "[$(date +%H:%M:%S)] [error] Asset regeneration failed." >&2
    fi
    REGEN_RUNNING=0
}

watch_layer_source_changes() {
    local -A last_mtime=()
    local path=""
    for path in "${WATCH_FILES[@]}"; do
        if [[ -f "$path" ]]; then
            last_mtime["$path"]=$(stat -c %Y "$path" 2>/dev/null || echo "")
        else
            last_mtime["$path"]=""
        fi
    done

    while true; do
        sleep 1
        local changed=0
        local current_mtime=""
        for path in "${WATCH_FILES[@]}"; do
            if [[ -f "$path" ]]; then
                current_mtime=$(stat -c %Y "$path" 2>/dev/null || echo "")
            else
                current_mtime=""
            fi
            if [[ "${last_mtime[$path]}" != "$current_mtime" ]]; then
                last_mtime["$path"]="$current_mtime"
                changed=1
            fi
        done
        if [[ "$changed" -eq 1 ]]; then
            refresh_layer_assets
        fi
    done
}

echo "=== Layer State Daemon Started ==="
echo "  Device ID : $XINPUT_DEVICE_ID"
echo "  Default   : $BASE_LAYER_WALLPAPER"
echo "  Watch     : ${WATCH_FILES[*]}"
echo "  Watching keycodes: ${!LAYER_KEYCODE_WALLPAPERS[*]}"
echo "  Press Ctrl+C to stop."
echo ""

# Set the default wallpaper on start
LAST_HANDLED_STATE="$(build_watch_state)"
refresh_layer_assets 1
set_layer_wallpaper "$BASE_LAYER_WALLPAPER"

watch_layer_source_changes &
LAYER_SOURCE_WATCHER_PID=$!
cleanup() {
    if [[ -n "${LAYER_SOURCE_WATCHER_PID:-}" ]]; then
        kill "$LAYER_SOURCE_WATCHER_PID" 2>/dev/null || true
        echo "[$(date +%H:%M:%S)] Layer source watcher stopped."
    fi
}
trap cleanup EXIT INT TERM

handle_key_event() {
    local event="$1"
    local keycode="$2"

    if [[ -z "${LAYER_KEYCODE_WALLPAPERS[$keycode]+_}" ]]; then
        return
    fi

    if [[ "$event" == "press" ]]; then
        set_layer_wallpaper "${LAYER_KEYCODE_WALLPAPERS[$keycode]}"
    elif [[ "$event" == "release" ]]; then
        set_layer_wallpaper "$BASE_LAYER_WALLPAPER"
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
