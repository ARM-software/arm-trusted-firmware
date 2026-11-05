# Copyright (c) 2026, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause

import logging
import re
from pathlib import Path

from src.files.file import FileUpdater
from src.models.cpuname import CpuName
from src.models.erratum import Erratum

from .helpers import insert_text_before, replace_text_range, update_copyright_year


class MkFile(FileUpdater):
    def __init__(self, file_path: Path) -> None:
        super().__init__(
            file_path,
            logging.getLogger(self.__class__.__name__)
        )

    def _update_file(self, cpu_name: CpuName, erratum: Erratum):
        """Insert or update errata build flag in makefile."""
        stack = MkFile._parse_mk_file(cpu_name, self.path)

        errata_macro = f"ERRATA_{cpu_name.flag_name}_{erratum.id}"
        flag = "CPU_FLAG_LIST += " + errata_macro

        insert_anchor = len(stack) - 1
        for i, item in enumerate(stack):
            erratum_number = item["erratum_number"]
            if erratum_number and erratum_number == int(erratum.id):
                start_line = item["start_line"]
                end_line = item["end_line"] + 1
                replace_text_range(self.path, start_line, end_line, flag)

                return True
            elif erratum_number and erratum_number > int(erratum.id):
                insert_anchor = i
                break

        if len(stack) == 1 and stack[0]["erratum_number"] is None:
            line = stack[0]["start_line"]
        elif insert_anchor == len(stack) - 1:
            line = stack[-2]["end_line"] + 1
        else:
            line = stack[insert_anchor]["start_line"]

        insert_text_before(self.path, line, flag)
        update_copyright_year(self.path)

        return True

    @staticmethod
    def _parse_mk_file(cpu_name: CpuName, file_path: Path) -> list[dict]:
        """
        Parse makefile to find existing errata build flags.

        Returns list of dicts with line numbers and errata IDs.
        """
        errata_macro_name = f"ERRATA_{cpu_name.flag_name}"
        macro_pattern = re.compile(rf"{re.escape(errata_macro_name)}_(\d+)")

        with open(file_path, "r") as f:
            lines = f.readlines()

        stack: list[dict] = []

        for i, line in enumerate(lines, start=1):
            stripped = line.strip()

            # This is for finding the end of the errata block in
            # the lib/cpus/cpu-ops.mk
            if "DSU" in line:
                stack.append(
                    {
                        "start_line": i,
                        "end_line": i,
                        "erratum_number": None
                    }
                )
                break

            if match:= macro_pattern.search(stripped):
                stack.append(
                    {
                        "start_line": i,
                        "end_line": i,
                        "erratum_number": int(match.group(1))
                    }
                )

        return stack
