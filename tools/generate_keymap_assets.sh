#!/usr/bin/env bash
set -euo pipefail

# Inputs
LAYER_FILE="./layers.h"

# Arguments
PARSE_ARGS=(parse -c 10 -q -)

# Layers to render
LAYERS=(L0 L1 L2 L3)

# base draw args (we'll append the specific layer in the loop)
DRAW_BASE_ARGS=(draw --qmk-keyboard "crkbd/rev1/common" --select-layers)

# Create temporary files and ensure they're removed on exit
TMP_C2JSON=$(mktemp)
TMP_PARSE=$(mktemp)
trap 'rm -f "${TMP_C2JSON}" "${TMP_PARSE}"' EXIT

# Step 1: qmk c2json ../layers.h -> TMP_C2JSON
qmk c2json "${LAYER_FILE}" > "${TMP_C2JSON}"

# Step 2: keymap parse ... reads from TMP_C2JSON and writes to TMP_PARSE
keymap "${PARSE_ARGS[@]}" < "${TMP_C2JSON}" > "${TMP_PARSE}"

# Step 3: render each requested layer into its own SVG without using pipes
for LAYER in "${LAYERS[@]}"; do
    OUT_FILE="./tools/keymap/keymap_${LAYER}.svg"

    # keymap draw expects '-' to read from stdin; we redirect TMP_PARSE into stdin
    keymap "${DRAW_BASE_ARGS[@]}" "${LAYER}" -- - < "${TMP_PARSE}" > "${OUT_FILE}"

    # Try to convert the generated SVG to PNG using a available tool.
    # Prefer rsvg-convert, then inkscape (new/old CLI), then ImageMagick 'convert'.
    convert_svg_to_png() {
        local svg="$1" png="$2"
        magick -background none -density 200 "$svg" "$png"
        return $?
    }

    OUT_PNG="${OUT_FILE%.svg}.png"
    if convert_svg_to_png "$OUT_FILE" "$OUT_PNG"; then
        echo "Wrote ${OUT_FILE} and ${OUT_PNG}"
    else
        echo "Wrote ${OUT_FILE} (PNG conversion failed or skipped)" >&2
    fi
done

# Done