# Copyright (c) 2026, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause

import re
from datetime import datetime
from pathlib import Path


def insert_text_before(path: Path, line_no: int, text: str):
    """
    Insert `text` BEFORE line number `line_no` (1-based).
    """
    lines = path.read_text(encoding="utf-8").splitlines(keepends=True)

    idx = max(0, min(line_no - 1, len(lines)))
    lines.insert(idx, text if text.endswith("\n") else text + "\n")

    path.write_text("".join(lines), encoding="utf-8")

def replace_text_at_line(path: Path, line_no: int, new_line: str):
    """Replace the specified line with new text."""
    lines = path.read_text(encoding="utf-8").splitlines(keepends=True)

    if 1 <= line_no <= len(lines):
        lines[line_no - 1] = new_line if new_line.endswith("\n") else new_line + "\n"

    path.write_text("".join(lines), encoding="utf-8")

def replace_text_range(path: Path, start_line: int, end_line: int, new_text: str):
    """
    Replace lines [start_line, end_line] (1-based, inclusive) with new_text.
    """
    lines = path.read_text(encoding="utf-8").splitlines(keepends=True)

    if start_line < 1 or end_line > len(lines) or start_line > end_line:
        raise ValueError("Invalid line range")

    replacement = new_text if new_text.endswith("\n") else new_text + "\n"

    # Replace the slice
    lines[start_line - 1 : end_line] = [replacement]

    path.write_text("".join(lines), encoding="utf-8")

def delete_text_at_line(path: Path, line_no: int):
    """Delete the specified line from file."""
    lines = path.read_text(encoding="utf-8").splitlines(keepends=True)

    if 1 <= line_no <= len(lines):
        del lines[line_no - 1]

    path.write_text("".join(lines), encoding="utf-8")

def wrap_line_at_words(text: str, prefix = '', width: int = 70) -> str:
    """
    Insert '\n' so each line is as close to `width` chars as possible,
    breaking only after words (whitespace). If a single word is longer
    than `width`, it is placed on its own line.
    """
    words = text.split()
    if not words:
        return ""

    lines = []
    cur = words[0]

    for w in words[1:]:
        # +1 for the space that would be inserted
        if len(cur) + 1 + len(w) <= width:
            cur += " " + w
        else:
            lines.append(cur)
            cur = w
    lines.append(cur)

    return f"\n{prefix}".join(lines)

def update_copyright_year(file_path: Path) -> bool:
    """
    Updates the ending year in:
    'Copyright (c) YYYY-YYYY, Arm Limited. All rights reserved.'

    Returns True if a change was made, False otherwise.
    """
    current_year = datetime.now().year

    with open(file_path, "r", encoding="utf-8") as f:
        content = f.read()

    pattern = re.compile(
        r"""
        ^(.*?)
        Copyright\s+\(c\)\s+
        (\d{4})-(\d{4})                            # start year - end year
        ,\s+Arm\s+Limited
        (?:\s+and\s+Contributors)?                 # optional "and Contributors"
        \.\s+All\s+rights\s+reserved\.
        (\s*\*?)                                   # optional trailing *
        """,
        re.MULTILINE | re.VERBOSE,
    )

    def replace(match):
        prefix = match.group(1)
        start_year = match.group(2)
        end_year = int(match.group(3))
        suffix = match.group(4)

        if end_year < current_year:
            return (
                f"{prefix}Copyright (c) {start_year}-{current_year}, "
                f"Arm Limited"
                f"{' and Contributors' if 'Contributors' in match.group(0) else ''}"
                f". All rights reserved.{suffix}"
            )
        return match.group(0)

    updated_content, count = re.subn(pattern, replace, content)

    if count > 0 and updated_content != content:
        with open(file_path, "w", encoding="utf-8") as f:
            f.write(updated_content)
        return True

    return False

__all__ = [
    "delete_text_at_line",
    "insert_text_before",
    "replace_text_at_line",
    "replace_text_range",
    "wrap_line_at_words"
]
