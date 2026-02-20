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

XINPUT_DEVICE_ID="${1:-}"

if ! [[ "$XINPUT_DEVICE_ID" =~ ^[0-9]+$ ]]; then
    echo "Usage: $0 XINPUT_DEVICE_ID" >&2
    echo "Example: $0 9" >&2
    exit 1
fi

LAYER_ASSETS_DIR="./tools/keymap"
WATCH_FILES=("./keymap.h" "./tools/config.yaml")

BASE_LAYER_WALLPAPER="$LAYER_ASSETS_DIR/keymap_L0.png"
PRESSED_LAYER_KEYS=()

set_layer_wallpaper() {
    local image="$1"
    [[ -f "$image" ]] || {
        echo "[warn] Image not found, skipping: $image" >&2
        return
    }
    xwallpaper --output eDP1 --center "$image"
    echo "[$(date +%H:%M:%S)] Wallpaper set to: $(basename "$image")"
}

build_watch_state() {
    local state=""
    local path=""
    local mtime=""
    for path in "${WATCH_FILES[@]}"; do
        if [[ -f "$path" ]]; then
            mtime=$(stat -c %Y "$path" 2>/dev/null || echo "missing")
        else
            mtime="missing"
        fi
        state+="${mtime}:"
    done
    printf '%s' "$state"
}

refresh_layer_assets() {
    local reason="${1:-Watched file changed}"
    echo "[$(date +%H:%M:%S)] ${reason}; tidying and regenerating assets..."
    if ./tools/tidy_keymap_layers.py "${WATCH_FILES[0]}" && ./tools/generate_keymap_assets.sh; then
        echo "[$(date +%H:%M:%S)] Asset regeneration complete."
    else
        echo "[$(date +%H:%M:%S)] [error] Asset regeneration failed." >&2
    fi
}

watch_layer_source_changes() {
    local last_state=""
    local current_state=""
    last_state=$(build_watch_state)
    while sleep 1; do
        current_state=$(build_watch_state)
        [[ "$current_state" == "$last_state" ]] && continue
        last_state="$current_state"
        refresh_layer_assets
    done
}

array_contains() {
    local needle="$1"
    local value
    shift
    for value in "$@"; do
        [[ "$value" == "$needle" ]] && return 0
    done
    return 1
}

add_pressed_layer_key() {
    local key="$1"
    array_contains "$key" "${PRESSED_LAYER_KEYS[@]}" || PRESSED_LAYER_KEYS+=("$key")
}

remove_pressed_layer_key() {
    local key="$1"
    local value
    local remaining=()
    for value in "${PRESSED_LAYER_KEYS[@]}"; do
        [[ "$value" == "$key" ]] || remaining+=("$value")
    done
    PRESSED_LAYER_KEYS=("${remaining[@]}")
}

resolve_layer_wallpaper() {
    local has_191=0
    local has_192=0
    array_contains 191 "${PRESSED_LAYER_KEYS[@]}" && has_191=1
    array_contains 192 "${PRESSED_LAYER_KEYS[@]}" && has_192=1
    case "${has_191}${has_192}" in
        11) printf '%s' "$LAYER_ASSETS_DIR/keymap_L3.png" ;;
        10) printf '%s' "$LAYER_ASSETS_DIR/keymap_L2.png" ;;
        01) printf '%s' "$LAYER_ASSETS_DIR/keymap_L1.png" ;;
        *) printf '%s' "$BASE_LAYER_WALLPAPER" ;;
    esac
}

handle_key_event() {
    local event="$1"
    local keycode="$2"

    case "$keycode" in
        191|192) ;;
        *) return ;;
    esac

    case "$event" in
        press) add_pressed_layer_key "$keycode" ;;
        release) remove_pressed_layer_key "$keycode" ;;
        *) return ;;
    esac

    set_layer_wallpaper "$(resolve_layer_wallpaper)"
}

echo "=== Layer State Daemon Started ==="
echo "  Device ID : $XINPUT_DEVICE_ID"
echo "  Default   : $BASE_LAYER_WALLPAPER"
echo "  Watch     : ${WATCH_FILES[*]}"
echo "  Press Ctrl+C to stop."
echo ""

refresh_layer_assets "Startup refresh"
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

# Use XI2 raw events to avoid X autorepeat press/release spam.
# Example block:
#   EVENT type 14 (RawKeyPress)
#       device: 14 (14)
#       detail: 191
current_event=""
current_device=""
current_keycode=""

xinput test-xi2 --root "$XINPUT_DEVICE_ID" | while read -r line; do
    case "$line" in
        *"(RawKeyPress)"*)
            current_event="press"
            current_device=""
            current_keycode=""
            continue
            ;;
        *"(RawKeyRelease)"*)
            current_event="release"
            current_device=""
            current_keycode=""
            continue
            ;;
    esac

    [[ -n "$current_event" ]] || continue

    if [[ "$line" =~ ^[[:space:]]*device:\ ([0-9]+) ]]; then
        current_device="${BASH_REMATCH[1]}"
    elif [[ "$line" =~ ^[[:space:]]*detail:\ ([0-9]+) ]]; then
        current_keycode="${BASH_REMATCH[1]}"
    fi

    if [[ -n "$current_device" && -n "$current_keycode" ]]; then
        [[ "$current_device" == "$XINPUT_DEVICE_ID" ]] && handle_key_event "$current_event" "$current_keycode"
        current_event=""
        current_device=""
        current_keycode=""
    fi
done
