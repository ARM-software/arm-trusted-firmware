# Copyright (c) 2026, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause

import logging
import re
from pathlib import Path

from src.files.file import FileUpdater
from src.models.cpuname import CpuName
from src.models.erratum import Erratum

from .helpers import (
    insert_text_before,
    replace_text_range,
    update_copyright_year,
    wrap_line_at_words,
)


class RstFile(FileUpdater):
    def __init__(self, file_path: Path) -> None:
        super().__init__(
            file_path,
            logging.getLogger(self.__class__.__name__)
        )

    def _update_file(self, cpu_name: CpuName, erratum: Erratum) -> bool:
        """Insert or update errata entry in RST documentation."""

        stack = RstFile._parse_rst_file(self.path, cpu_name)

        errata_macro = f"ERRATA_{cpu_name.flag_name}_{erratum.id}"
        comment = RstFile._create_rst_comment(cpu_name, errata_macro, erratum)

        idx = len(stack)
        for i, item in enumerate(stack):
            erratum_number = item['erratum_number']
            if erratum_number and erratum_number == int(erratum.id):
                start_line = item['start_line']
                end_line = item['end_line']
                replace_text_range(self.path, start_line, end_line, comment)

                return True
            elif erratum_number and erratum_number > int(erratum.id):
                idx = i
                break

        if len(stack) == 1 and stack[0]['erratum_number'] == float("inf"):
            line = stack[0]['start_line']
        elif idx == len(stack) - 1:
            line = stack[-2]['end_line'] + 1
        else:
            line = stack[idx]['start_line']

        if comment is not None:
            if len(stack) == 1 and stack[0]['erratum_number'] == float("inf"):
                comment = (
                    f"For {cpu_name.human_readable_name}, "
                    f"the following errata build flags are defined:\n\n"
                    f"{comment}"
                )

            insert_text_before(self.path, line, comment)
            update_copyright_year(self.path)

            return True

        return False

    @staticmethod
    def _create_rst_comment(cpu_names: CpuName, errata_macro, erratum: Erratum):
        revision_label = "revisions" if len(erratum.present_revisions) > 1 else "revision"
        fix_status = "still open" if erratum.is_open() else f"fixed in {erratum.fixed_revision}"

        comment = (
            f"-  ``{errata_macro}``: This applies erratum {erratum.id} workaround "
            f"to the {cpu_names.human_readable_name} CPU. "
            f"This needs to be enabled for {revision_label} "
            f"{', '.join(erratum.present_revisions)} of the CPU. "
            f"It is {fix_status}."
        )
        comment = wrap_line_at_words(comment, "   ", 80)
        comment += "\n\n"

        return comment

    @staticmethod
    def _parse_rst_file(file_path: Path, cpu_names: CpuName):
        errata_macro_name = f"ERRATA_{cpu_names.flag_name}"
        macro_pattern = re.compile(rf"{re.escape(errata_macro_name)}_(\d+)")

        with file_path.open("r", encoding="utf-8") as file:
            lines = file.readlines()

        stack = []
        last_comment = None

        for i, line in enumerate(lines, start=1):
            stripped = line.strip()

            if stripped == "":
                if last_comment is not None and stack:
                    stack[-1]["end_line"] = i
                last_comment = None
                continue

            # This is for finding the end of the errata block in
            # the docs/design/cpu-specific-build-macros.rst
            if "DSU Errata Workarounds" in line:
                stack.append({
                    "start_line": i,
                    "end_line": i,
                    "erratum_number": float("inf"),
                })
                break

            if m:= macro_pattern.search(stripped):
                last_comment = stripped

                stack.append({
                        "start_line": i,
                        "erratum_number": int(m.group(1)),
                    }
                )

        return stack
