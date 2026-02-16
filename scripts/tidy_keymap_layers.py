#!/usr/bin/env python3
"""
QMK Keymap Aligner
Aligns QMK keymap keycodes to their visual comment guide columns.
Handles both guide-before-keys and keys-before-guide patterns.
"""

import re
import sys
from typing import List, Tuple


def find_split_sections(guide_line: str):
    """
    Detect if this is a split keyboard guide with left and right sections.
    Returns (left_columns, right_columns) or (all_columns, None) for non-split.
    """
    columns = []
    i = 0

    while i < len(guide_line):
        if guide_line[i] in "|+":
            start = i
            i += 1
            while i < len(guide_line) and guide_line[i] not in "|+":
                i += 1
            if i < len(guide_line):
                columns.append((start, i))
        else:
            i += 1

    if not columns:
        return columns, None

    # Detect split by finding gap columns
    widths = [end - start for start, end in columns]
    avg_width = sum(widths) / len(widths)

    # Look for either:
    # 1. Very wide columns (>1.5x average) - large gap with lots of spaces
    # 2. Very narrow columns (<0.5x average) - small gap like "|  |"
    for i, (start, end) in enumerate(columns):
        width = end - start
        if width > avg_width * 1.5 or width < avg_width * 0.5:
            # This is likely the gap column - split here
            left_cols = columns[:i]
            right_cols = columns[i + 1 :]  # Skip the gap column itself
            if left_cols and right_cols:
                return left_cols, right_cols

    # No split detected
    return columns, None


def extract_keycodes(line: str):
    """Extract keycodes and indentation from a line."""
    indent = re.match(r"^(\s*)", line).group(1)
    code = re.sub(r"//.*$", "", line).strip()
    code = code.rstrip(",")
    keycodes = [k.strip() for k in code.split(",") if k.strip()]
    return indent, keycodes


def is_keycode_line(line):
    """Check if line contains keycodes."""
    # Exclude layer declarations like "[_LOWER] = LAYOUT_split_3x6_3("
    if re.search(r"\[_\w+\]\s*=\s*LAYOUT", line):
        return False

    return bool(
        re.search(r"KC_|RAISE|LOWER|SUPER|QUOTE|TD\(|LT\(|MT\(|XXXXXXX|_______", line)
    ) and not line.strip().startswith("//")


def align_split_layout(keycodes, left_cols, right_cols, indent, is_last_row=False):
    """Align keycodes for split keyboard layout."""
    # Split keycodes in half, not based on column count
    # (since layouts may have extra columns for optional keys)
    num_keys = len(keycodes)
    num_left_keys = num_keys // 2
    left_keys = keycodes[:num_left_keys]
    right_keys = keycodes[num_left_keys:]

    max_pos = max(end for _, end in right_cols) + 50
    chars = [" "] * max_pos

    # Set indent
    for j, c in enumerate(indent):
        chars[j] = c

    # Align left section
    last_written_pos = len(indent) - 1

    for j, key in enumerate(left_keys):
        if j < len(left_cols):
            col_start, col_end = left_cols[j]
            comma_pos = col_end
            keycode_start = comma_pos - len(key)

            # Only adjust if keycode would actually overwrite the last comma
            # (keycode ends after last comma position)
            if keycode_start + len(key) > last_written_pos:
                # No overlap - use ideal position
                pass
            else:
                # Would overlap - move keycode right
                keycode_start = last_written_pos + 1
                if keycode_start + len(key) > comma_pos:
                    comma_pos = keycode_start + len(key)

            for k, c in enumerate(key):
                pos = keycode_start + k
                if pos >= 0 and pos < len(chars):
                    chars[pos] = c

            if comma_pos < len(chars):
                chars[comma_pos] = ","
                last_written_pos = comma_pos

    # Align right section
    if right_cols:
        last_written_pos = right_cols[0][0] - 1

    for j, key in enumerate(right_keys):
        if j < len(right_cols):
            col_start, col_end = right_cols[j]
            comma_pos = col_end
            keycode_start = comma_pos - len(key)

            # Only adjust if keycode would actually overwrite
            if keycode_start + len(key) > last_written_pos:
                # No overlap - use ideal position
                pass
            else:
                # Would overlap - move keycode right
                keycode_start = last_written_pos + 1
                if keycode_start + len(key) > comma_pos:
                    comma_pos = keycode_start + len(key)

            for k, c in enumerate(key):
                pos = keycode_start + k
                if pos >= 0 and pos < len(chars):
                    chars[pos] = c

            # Only add comma if this is not the last keycode OR not the last row
            if (j < len(right_keys) - 1 or not is_last_row) and comma_pos < len(chars):
                chars[comma_pos] = ","
                last_written_pos = comma_pos

    result = "".join(chars).rstrip()
    return result


def align_non_split_layout(keycodes, cols, indent, is_last_row=False):
    """Align keycodes for non-split layout."""
    max_pos = max(end for _, end in cols) + 50
    chars = [" "] * max_pos

    for j, c in enumerate(indent):
        chars[j] = c

    last_written_pos = len(indent) - 1

    for j, key in enumerate(keycodes):
        if j < len(cols):
            col_start, col_end = cols[j]
            comma_pos = col_end
            keycode_start = comma_pos - len(key)

            # Only adjust if keycode would actually overwrite
            if keycode_start + len(key) > last_written_pos:
                # No overlap - use ideal position
                pass
            else:
                # Would overlap - move keycode right
                keycode_start = last_written_pos + 1
                if keycode_start + len(key) > comma_pos:
                    comma_pos = keycode_start + len(key)

            for k, c in enumerate(key):
                pos = keycode_start + k
                if pos >= 0 and pos < len(chars):
                    chars[pos] = c

            # Only add comma if this is not the last keycode OR not the last row
            if (j < len(keycodes) - 1 or not is_last_row) and comma_pos < len(chars):
                chars[comma_pos] = ","
                last_written_pos = comma_pos

    result = "".join(chars).rstrip()
    return result


def process_file(content):
    """Process the keymap file, handling both guide-before and guide-after patterns."""
    lines = content.split("\n")
    output = []
    i = 0

    while i < len(lines):
        line = lines[i]

        # Case 1: Guide line followed by keycodes (normal case)
        if re.match(r"\s*//\s*\|[-+|]+", line):
            output.append(line)

            left_cols, right_cols = find_split_sections(line)

            if i + 1 < len(lines) and is_keycode_line(lines[i + 1]):
                # Check if this is a thumb row pattern:
                # The previous line should NOT be a guide (thumb rows have space/closing paren before them)
                # AND the next line after keycodes IS a guide
                if i > 0 and not re.match(r"\s*//\s*\|[-+|]+", lines[i - 1]):
                    # Previous line is not a guide, so this might be start of a layer
                    # Check if keycodes have their own guide after
                    if i + 2 < len(lines) and re.match(
                        r"\s*//\s*\|[-+|]+", lines[i + 2]
                    ):
                        # This is thumb row pattern: no guide before, keycodes, guide after
                        # Skip this and let Case 2 handle it
                        i += 1
                        continue

                indent, keycodes = extract_keycodes(lines[i + 1])

                # Check if this is the last row before closing paren
                is_last_row = False
                # Look ahead to see if we hit a closing paren soon
                for j in range(i + 2, min(i + 5, len(lines))):
                    stripped = lines[j].strip()
                    if stripped.startswith("),") or stripped.startswith(")"):
                        is_last_row = True
                        break
                    elif is_keycode_line(lines[j]):
                        # Found another keycode line, so this isn't the last
                        break

                if left_cols and right_cols:
                    aligned = align_split_layout(
                        keycodes, left_cols, right_cols, indent, is_last_row
                    )
                elif left_cols:
                    aligned = align_non_split_layout(
                        keycodes, left_cols, indent, is_last_row
                    )
                else:
                    aligned = lines[i + 1]

                output.append(aligned)
                i += 2
                continue
            else:
                # Guide line with no keycodes after - just keep it
                i += 1
                continue

        # Case 2: Keycode line followed by guide (thumb row case)
        elif is_keycode_line(line):
            if i + 1 < len(lines) and re.match(r"\s*//\s*\|[-+|]+", lines[i + 1]):
                guide_line = lines[i + 1]
                left_cols, right_cols = find_split_sections(guide_line)
                indent, keycodes = extract_keycodes(line)

                # Check if this is the last row before closing paren
                is_last_row = False
                for j in range(i + 2, min(i + 5, len(lines))):
                    stripped = lines[j].strip()
                    if stripped.startswith("),") or stripped.startswith(")"):
                        is_last_row = True
                        break
                    elif is_keycode_line(lines[j]):
                        break

                if left_cols and right_cols:
                    aligned = align_split_layout(
                        keycodes, left_cols, right_cols, indent, is_last_row
                    )
                elif left_cols:
                    aligned = align_non_split_layout(
                        keycodes, left_cols, indent, is_last_row
                    )
                else:
                    aligned = line

                output.append(aligned)
                output.append(guide_line)
                i += 2
                continue

        output.append(line)
        i += 1

    return "\n".join(output)


def main():
    """CLI entry point."""
    if len(sys.argv) < 2:
        content = sys.stdin.read()
        output = process_file(content)
        print(output)
        return

    input_path = sys.argv[1]

    try:
        with open(input_path, "r", encoding="utf-8") as f:
            content = f.read()
    except FileNotFoundError:
        print(f"Error: File '{input_path}' not found", file=sys.stderr)
        sys.exit(1)

    output = process_file(content)

    output_path = sys.argv[1]
    with open(output_path, "w", encoding="utf-8") as f:
        f.write(output)
    print(f"✓ Aligned keymap saved to '{output_path}'")


if __name__ == "__main__":
    main()
