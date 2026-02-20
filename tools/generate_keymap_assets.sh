#!/usr/bin/env bash
set -euo pipefail

# Inputs
LAYER_FILE="./keymap.h"
# Optional background image used as the wallpaper base.
# Override via env var, e.g.:
# BACKGROUND_WALLPAPER_IMAGE=./tools/keymap/wallpaper_base.jpg ./tools/generate_keymap_assets.sh
BACKGROUND_WALLPAPER_IMAGE="${BACKGROUND_WALLPAPER_IMAGE:-}"

# Arguments
PARSE_ARGS=(parse -c 10 -q -)

# Layers to render
LAYERS=(L0 L1 L2 L3)

# base draw args (we'll append the specific layer in the loop)
DRAW_BASE_ARGS=(draw --qmk-keyboard "crkbd/rev1/common" --select-layers)

CONFIG="--config ./tools/config.yaml"

# Create temporary files and ensure they're removed on exit
TMP_C2JSON=$(mktemp)
TMP_PARSE=$(mktemp)
trap 'rm -f "${TMP_C2JSON}" "${TMP_PARSE}"' EXIT

# Step 1: qmk c2json ../keymap.h -> TMP_C2JSON
qmk c2json "${LAYER_FILE}" > "${TMP_C2JSON}"

# Step 2: keymap parse ... reads from TMP_C2JSON and writes to TMP_PARSE
keymap $CONFIG "${PARSE_ARGS[@]}" < "${TMP_C2JSON}" > "${TMP_PARSE}"

if [[ -n "$BACKGROUND_WALLPAPER_IMAGE" && ! -f "$BACKGROUND_WALLPAPER_IMAGE" ]]; then
    echo "[warn] BACKGROUND_WALLPAPER_IMAGE not found: $BACKGROUND_WALLPAPER_IMAGE" >&2
    echo "[warn] Continuing without wallpaper background merge." >&2
    BACKGROUND_WALLPAPER_IMAGE=""
fi

# Step 3: render each requested layer into its own SVG without using pipes
for LAYER in "${LAYERS[@]}"; do
    OUT_FILE="./tools/keymap/keymap_${LAYER}.svg"

    # keymap draw expects '-' to read from stdin; we redirect TMP_PARSE into stdin
    keymap $CONFIG "${DRAW_BASE_ARGS[@]}" "${LAYER}" -- - < "${TMP_PARSE}" > "${OUT_FILE}"

    # Convert SVG to transparent PNG, then optionally composite on wallpaper.
    convert_svg_to_png() {
        local svg="$1" png="$2" base_wallpaper="$3"
        local overlay_png
        overlay_png=$(mktemp --suffix=.png)
        if ! magick -background none -density 200 "$svg" "$overlay_png"; then
            rm -f "$overlay_png"
            return 1
        fi

        if [[ -n "$base_wallpaper" ]]; then
            if ! magick "$base_wallpaper" "$overlay_png" -gravity center -composite "$png"; then
                rm -f "$overlay_png"
                return 1
            fi
            rm -f "$overlay_png"
        elif ! mv "$overlay_png" "$png"; then
            rm -f "$overlay_png"
            return 1
        fi
        return 0
    }

    OUT_PNG="${OUT_FILE%.svg}.png"
    if convert_svg_to_png "$OUT_FILE" "$OUT_PNG" "$BACKGROUND_WALLPAPER_IMAGE"; then
        echo "Wrote ${OUT_FILE} and ${OUT_PNG}"
    else
        echo "Wrote ${OUT_FILE} (PNG conversion failed or skipped)" >&2
    fi
done

# Done
