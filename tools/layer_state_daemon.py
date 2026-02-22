#!/usr/bin/env python3
"""
layer_state_daemon.py
Listens for physical key press/release events via xinput XI2, updates layer
wallpapers, and regenerates keymap assets when keymap.h changes.
"""

from __future__ import annotations

import argparse
import signal
import subprocess
import sys
import threading
import time
from datetime import datetime
from pathlib import Path


# ---[ CONFIGURATION ]---------------------------------------------------------
XINPUT_DEVICE_ID = 9

BASE_DIR = Path(__file__).resolve().parent.parent
LAYER_ASSETS_DIR = BASE_DIR / "tools" / "keymap"
LAYER_SOURCE_FILE = BASE_DIR / "keymap.h"

BASE_LAYER_WALLPAPER = LAYER_ASSETS_DIR / "keymap_L0.png"
LAYER_KEYCODE_WALLPAPERS = {
    191: LAYER_ASSETS_DIR / "keymap_L2.png",
    192: LAYER_ASSETS_DIR / "keymap_L1.png",
    193: LAYER_ASSETS_DIR / "keymap_L3.png",
}
# ---[ END CONFIGURATION ]-----------------------------------------------------


stop_event = threading.Event()
xinput_process: subprocess.Popen[str] | None = None
current_wallpaper: Path | None = None
cleanup_done = False


def ts() -> str:
    return datetime.now().strftime("%H:%M:%S")


def set_layer_wallpaper(image: Path) -> None:
    global current_wallpaper
    if not image.is_file():
        print(f"[warn] Image not found, skipping: {image}", file=sys.stderr)
        return

    if current_wallpaper == image:
        return

    result = subprocess.run(
        ["xwallpaper", "--output", "eDP1", "--center", str(image)],
        check=False,
        stdout=subprocess.DEVNULL,
        stderr=subprocess.DEVNULL,
    )
    if result.returncode == 0:
        current_wallpaper = image
        print(f"[{ts()}] Wallpaper set to: {image.name}")
    else:
        print(f"[{ts()}] [error] Failed to set wallpaper: {image}", file=sys.stderr)


def refresh_layer_assets(startup: bool = False) -> None:
    if startup:
        print(f"[{ts()}] Startup refresh: tidying and regenerating assets...")
    else:
        print(f"[{ts()}] keymap.h changed; tidying and regenerating assets...")

    tidy_rc = subprocess.run(
        [sys.executable, str(BASE_DIR / "tools" / "tidy_keymap_layers.py"), str(LAYER_SOURCE_FILE)],
        check=False,
        cwd=BASE_DIR,
    ).returncode
    gen_rc = subprocess.run(
        ["bash", str(BASE_DIR / "tools" / "generate_keymap_assets.sh")],
        check=False,
        cwd=BASE_DIR,
    ).returncode

    if tidy_rc == 0 and gen_rc == 0:
        print(f"[{ts()}] Asset regeneration complete.")
    else:
        print(f"[{ts()}] [error] Asset regeneration failed.", file=sys.stderr)


def watch_layer_source_changes() -> None:
    last_mtime = int(LAYER_SOURCE_FILE.stat().st_mtime) if LAYER_SOURCE_FILE.is_file() else None

    while not stop_event.is_set():
        time.sleep(1)
        if not LAYER_SOURCE_FILE.is_file():
            continue

        current_mtime = int(LAYER_SOURCE_FILE.stat().st_mtime)
        if last_mtime is None:
            last_mtime = current_mtime
            continue

        if current_mtime != last_mtime:
            last_mtime = current_mtime
            refresh_layer_assets(startup=False)


def handle_key_event(event: str, keycode: int) -> None:
    if keycode not in LAYER_KEYCODE_WALLPAPERS:
        print(f"[{ts()}] Ignored {event} for unmapped keycode: {keycode}")
        return

    print(f"[{ts()}] Key event: {event} keycode={keycode}")
    if event == "release":
        print(f"[{ts()}] Active layer keycode: none (base wallpaper)")
        set_layer_wallpaper(BASE_LAYER_WALLPAPER)
        return

    print(f"[{ts()}] Active layer keycode: {keycode}")
    set_layer_wallpaper(LAYER_KEYCODE_WALLPAPERS[keycode])


def cleanup() -> None:
    global cleanup_done
    if cleanup_done:
        return
    cleanup_done = True

    stop_event.set()
    global xinput_process
    if xinput_process and xinput_process.poll() is None:
        xinput_process.terminate()
        try:
            xinput_process.wait(timeout=2)
        except subprocess.TimeoutExpired:
            xinput_process.kill()
    print(f"[{ts()}] Layer source watcher stopped.")


def install_signal_handlers() -> None:
    def _handler(_signum: int, _frame: object) -> None:
        cleanup()
        raise SystemExit(0)

    signal.signal(signal.SIGINT, _handler)
    signal.signal(signal.SIGTERM, _handler)


def main() -> int:
    global xinput_process, XINPUT_DEVICE_ID

    parser = argparse.ArgumentParser(description="Layer state daemon for QMK wallpaper + asset refresh.")
    parser.add_argument(
        "--device-id",
        type=int,
        default=XINPUT_DEVICE_ID,
        help="xinput device id to listen to (default: %(default)s)",
    )
    args = parser.parse_args()
    XINPUT_DEVICE_ID = args.device_id

    print("=== Layer State Daemon Started ===")
    print(f"  Device ID : {XINPUT_DEVICE_ID}")
    print(f"  Default   : {BASE_LAYER_WALLPAPER}")
    print(f"  Layers    : {LAYER_SOURCE_FILE}")
    print(f"  Watching keycodes: {' '.join(str(k) for k in LAYER_KEYCODE_WALLPAPERS.keys())}")
    print("  Press Ctrl+C to stop.")
    print("")

    refresh_layer_assets(startup=True)
    set_layer_wallpaper(BASE_LAYER_WALLPAPER)

    watcher = threading.Thread(target=watch_layer_source_changes, daemon=True)
    watcher.start()

    install_signal_handlers()

    current_event = ""
    current_device: int | None = None
    current_keycode: int | None = None

    try:
        xinput_process = subprocess.Popen(
            ["xinput", "test-xi2", "--root", str(XINPUT_DEVICE_ID)],
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            text=True,
            bufsize=1,
        )
    except FileNotFoundError:
        print("[error] xinput not found in PATH.", file=sys.stderr)
        cleanup()
        return 1

    assert xinput_process.stdout is not None
    for raw_line in xinput_process.stdout:
        if stop_event.is_set():
            break

        line = raw_line.rstrip("\n")
        if "(RawKeyPress)" in line:
            current_event = "press"
            current_device = None
            current_keycode = None
            continue

        if "(RawKeyRelease)" in line:
            current_event = "release"
            current_device = None
            current_keycode = None
            continue

        if not current_event:
            continue

        stripped = line.strip()
        if stripped.startswith("device:"):
            value = stripped.split(":", 1)[1].strip().split(" ", 1)[0]
            if value.isdigit():
                current_device = int(value)
        elif stripped.startswith("detail:"):
            value = stripped.split(":", 1)[1].strip().split(" ", 1)[0]
            if value.isdigit():
                current_keycode = int(value)

        if current_device is not None and current_keycode is not None:
            if current_device == XINPUT_DEVICE_ID:
                handle_key_event(current_event, current_keycode)
            current_event = ""
            current_device = None
            current_keycode = None

    cleanup()
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except KeyboardInterrupt:
        cleanup()
        raise SystemExit(0)
