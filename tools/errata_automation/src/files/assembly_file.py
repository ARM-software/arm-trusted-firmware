# Copyright (c) 2026, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause

import logging
import re
from pathlib import Path

from src.files.file import FileUpdater
from src.models.cpuname import CpuName
from src.models.erratum import Erratum
from src.models.erratum_revision import RevisionRange

from .helpers import insert_text_before, replace_text_at_line, update_copyright_year

SYSTEM_REGISTER_PATTERN = re.compile(
    r"\bS\d+_\d+_C\d+_C\d+_\d+\b",
    re.IGNORECASE,
)


class AssemblyFile(FileUpdater):
    def __init__(self, file_path: Path) -> None:
        super().__init__(
            file_path,
            logging.getLogger(self.__class__.__name__)
        )

    def _update_file(self, cpu_name: CpuName, erratum: Erratum) -> bool:
        macro = AssemblyFile._create_errata_macro(cpu_name, erratum)

        stack = AssemblyFile._parse_asm_file(self.path, cpu_name)

        idx = len(stack)
        for i, item in enumerate(stack):
            erratum_number = item.get("erratum_number")
            if erratum_number and int(erratum_number) == int(erratum.id):
                line = item["end_line"]
                revision_macro = AssemblyFile._get_revision_macro(erratum, cpu_name)

                if not item.get("check"):
                    insert_text_before(self.path, line + 1, "\n" + revision_macro)
                else:
                    replace_text_at_line(self.path, line, revision_macro)

                return True

            elif erratum_number and int(erratum_number) > int(erratum.id):
                idx = i
                break

        # TODO: Verify how the insertion handles existing conditionally compiled
        # errata, ensuring that new errata are inserted within
        # the appropriate conditional compilation guard.
        if macro != None:
            if idx == len(stack):
                line = stack[-1]['end_line'] + 2
            else:
                line = stack[idx]['start_line']

            insert_text_before(self.path, line, macro)
            update_copyright_year(self.path)
            return True

        return False

    @staticmethod
    def _lowercase_assembly(code: str) -> str:
        """Lowercase assembly code while preserving system-register encodings."""

        return SYSTEM_REGISTER_PATTERN.sub(
            lambda match: match.group(0).upper(),
            code.lower(),
        )

    @staticmethod
    def _create_errata_macro(cpu_name: CpuName, erratum: Erratum) -> str:
        macro = None
        error_msg = set()
        macro_body = ""

        if len(erratum.workaround_details.chicken_bits) > 0:
            for chicken_bit in erratum.workaround_details.chicken_bits:
                register_macro = f"{cpu_name.name.upper()}_{chicken_bit.relaxed_register_name}"

                binary_value = chicken_bit.binary_value
                bit_position = chicken_bit.bit_position

                if not chicken_bit.is_valid:
                    macro_body += (
                        "\tInvalid chicken_bit! "
                        f"name: {chicken_bit.register_name}, "
                        f"field: {chicken_bit.register_field_name}, "
                        f"bits: {chicken_bit.bits}, "
                        f"value: {chicken_bit.value}\n"
                    )
                    error_msg.add("Invalid chicken_bit!")

                elif bit_position.width == 1:
                    if binary_value == 1:
                        macro_body += f"\tsysreg_bit_set {register_macro}, BIT({chicken_bit.bits})\n"

                    elif binary_value == 0:
                        macro_body += f"\tsysreg_bit_clear {register_macro}, BIT({chicken_bit.bits})\n"

                else:
                    if chicken_bit.register_field_name != "n/a":
                        macro_body += (
                            f"\tsysreg_bitfield_insert {register_macro}, \\\n"
                            f"\t\t{register_macro}_{chicken_bit.register_field_name}_BIT, \\\n"
                            f"\t\t{register_macro}_{chicken_bit.register_field_name}_SHIFT, \\\n"
                            f"\t\t{register_macro}_{chicken_bit.register_field_name}_WIDTH\n"
                        )
                    else:
                        macro_body += (
                            f"\tsysreg_bitfield_insert {register_macro}, {binary_value}, {bit_position.lsb} , {bit_position.width}\n"
                        )

        if len(erratum.workaround_details.sequence) > 0:
            sequence_lines = ""
            for sequence_step in erratum.workaround_details.sequence:
                line = re.sub(r",(?!\s)", ", ", sequence_step.instruction)
                line = line.strip()
                sequence_lines = sequence_lines + '\t' + line + '\n'

            sequence_lines = AssemblyFile._lowercase_assembly(sequence_lines)

            macro_body += sequence_lines

        if len(macro_body) == 0:
            macro_body += (
                "\tInvalid chicken_bit!\n"
                f"\tErratum is classified as {erratum.workaround_class},\n"
                "\tbut the expected chicken_bits are missing.\n"
            )
            error_msg.add("Invalid chicken_bit!")

        if erratum.workaround_details.chicken_bits and erratum.workaround_details.sequence:
            error_msg.add("Both sequence and chicken bits workarounds are present!")

        check_macro = AssemblyFile._get_revision_macro(erratum, cpu_name)
        review_marker = (
            f", __REQUIRES_REVIEW__{', '.join(map(str, error_msg))}__"
            if erratum.requires_review
            else ""
        )

        macro = (
            f"workaround_reset_start {cpu_name.name}, ERRATUM({erratum.id}), ERRATA_{cpu_name.flag_name}_{erratum.id}{review_marker}\n"
            f"{macro_body}"
            f"workaround_reset_end {cpu_name.name}, ERRATUM({erratum.id})\n"
            f"\n"
            f"{check_macro}\n"
            f"\n"
        )

        return macro

    @staticmethod
    def _parse_asm_file(path: Path, cpu_name: CpuName) -> list[dict]:
        """
        Parse assembly file to extract existing errata workarounds.

        Returns list of dicts with start/end line numbers, errata IDs,
        and revision check information.
        """
        # Read all lines in memory
        with open(path, "r") as f:
            lines = f.readlines()

        # We'll keep a stack of active "starts" that haven't yet found their "end"
        stack = []

        # Regex patterns for capturing ERRATUM
        errata_macro_name = f"ERRATA_{cpu_name.flag_name}"
        start_pattern = re.compile(
            rf"""
            ^\s*workaround_(?:reset|runtime)_start\b
            .*?\b{re.escape(errata_macro_name)}_(\d+)\b
            """,
            re.IGNORECASE | re.VERBOSE
        )

        end_pattern = re.compile(
            rf"""
            ^\s*workaround_(?:reset|runtime)_end\b
            .*?\b{re.escape(cpu_name.name)}\b
            .*?\bERRATUM\(\s*(\d+)\s*\)
            """,
            re.IGNORECASE | re.VERBOSE
        )

        range_ls_pattern = re.compile(
            r"""
            ^\s*check_erratum_ls
            \s+(\w+),                    # CPU name
            \s*ERRATUM\((\d+)\),
            \s*CPU_REV\((\d+)\s*,\s*(\d+)\)
            """,
            re.VERBOSE
        )

        range_hs_pattern = re.compile(
            r"""
            ^\s*check_erratum_hs
            \s+(\w+),                    # CPU name
            \s*ERRATUM\((\d+)\),
            \s*CPU_REV\((\d+)\s*,\s*(\d+)\)
            """,
            re.VERBOSE
        )

        range_range_pattern = re.compile(
            r"""
            ^\s*check_erratum_range
            \s+(\w+),                    # CPU name
            \s*ERRATUM\((\d+)\),
            \s*CPU_REV\((\d+)\s*,\s*(\d+)\),
            \s*CPU_REV\((\d+)\s*,\s*(\d+)\)
            """,
            re.VERBOSE
        )

        chosen_range_pattern = re.compile(
            rf"""
            ^\s*check_erratum_chosen\b
            .*?\bERRATUM\(\s*(\d+)\s*\)\s*,\s*
            {re.escape(errata_macro_name)}_\1\b
            """,
            re.IGNORECASE | re.VERBOSE
        )

        for i, line in enumerate(lines, start=1):
            stripped = line.strip()

            # ----------------------------------------------------------------------
            # 1) Check for "start" markers
            #    We look first for 'workaround_reset_start' or 'workaround_runtime_start'
            # ----------------------------------------------------------------------
            if "workaround_reset_start" in stripped:
                marker_type = "reset"
                # Attempt to extract ERRATUM
                erratum_match = start_pattern.search(stripped)

                if erratum_match:
                    erratum_number = int(erratum_match.group(1))

                    # Push onto the stack
                    stack.append({
                        "start_line": i,
                        "marker_type": marker_type,      # 'reset'
                        "erratum_number": erratum_number,
                    })

            elif "workaround_runtime_start" in stripped:
                marker_type = "runtime"
                # Attempt to extract ERRATUM
                erratum_match = start_pattern.search(stripped)

                if erratum_match:
                    erratum_number = int(erratum_match.group(1))

                    # Push onto the stack
                    stack.append({
                        "start_line": i,
                        "marker_type": marker_type,      # 'runtime'
                        "erratum_number": erratum_number,
                    })

            # ----------------------------------------------------------------------
            # 2) Check for "end" markers
            #    We look for 'workaround_reset_end' or 'workaround_runtime_end'
            # ----------------------------------------------------------------------
            elif "workaround_reset_end" in stripped:
                erratum_match = end_pattern.search(stripped)

                if erratum_match:
                    erratum_number = int(erratum_match.group(1))
                    if stack and stack[-1]['erratum_number'] == erratum_number:
                        stack[-1].update({
                                "end_line": i,
                                "check": False
                            })

            elif "workaround_runtime_end" in stripped:
                erratum_match = end_pattern.search(stripped)

                if erratum_match:
                    erratum_number = int(erratum_match.group(1))
                if stack and stack[-1]['erratum_number'] == erratum_number:
                    stack[-1].update({
                            "end_line": i,
                            "check": False
                        })

            elif "check_erratum_ls" in stripped:
                revision_match = range_ls_pattern.search(stripped)
                if revision_match and stack:
                    erratum_id = int(revision_match.group(2))
                    r = int(revision_match.group(3))
                    p = int(revision_match.group(4))
                    if stack[-1]['erratum_number'] == erratum_id:
                        stack[-1].update({
                            "end_line": i,
                            "check": True,
                            "end_r": r,
                            "end_p": p,
                        })

            elif "check_erratum_hs" in stripped:
                revision_match = range_hs_pattern.search(stripped)
                if revision_match and stack:
                    erratum_id = int(revision_match.group(2))
                    r = int(revision_match.group(3))
                    p = int(revision_match.group(4))
                    if stack[-1]['erratum_number'] == erratum_id:
                        stack[-1].update({
                            "end_line": i,
                            "check": True,
                            "end_r": r,
                            "end_p": p,
                        })

            elif "check_erratum_range" in stripped:
                revision_match = range_range_pattern.search(stripped)
                if revision_match and stack:
                    erratum_id = int(revision_match.group(2))
                    start_r = int(revision_match.group(3))
                    start_p = int(revision_match.group(4))
                    end_r = int(revision_match.group(5))
                    end_p = int(revision_match.group(6))
                    if stack[-1]['erratum_number'] == erratum_id:
                        stack[-1].update({
                            "end_line": i,
                            "check": True,
                            "start_r": int(start_r),
                            "start_p": int(start_p),
                            "end_r": int(end_r),
                            "end_p": int(end_p),
                        })

            elif "check_erratum_chosen" in stripped:
                erratum_match = chosen_range_pattern.search(stripped)

                if erratum_match:
                    erratum_number = int(erratum_match.group(1))
                    if stack and stack[-1]['erratum_number'] == erratum_number:
                        stack[-1].update({
                                "end_line": i,
                                "check": True,
                                "end_r": None,
                                "end_p": None,
                            })

            elif "cpu_reset_prologue" in stripped:
                stack.append({
                        "start_line": i,
                        "end_line": i,
                        "marker_type": None,
                        "erratum_number": None,
                        "check": False,
                    })

        return stack

    @staticmethod
    def _get_revision_macro(erratum: Erratum, cpu_name: CpuName) -> str:
        """
        Generate revision check macro based on affected and fixed versions.

        Returns appropriate check_erratum_* macro for the errata's revision range.
        """

        present_revisions = RevisionRange(revisions=erratum.present_revisions)

        if present_revisions.empty():
            raise ValueError(f"Present revision is empty for erratum {erratum.id}!")

        start_rev = present_revisions.start_revision
        end_rev = present_revisions.end_revision

        if erratum.is_open():
            if present_revisions.is_continuous_range_from_release_revision():
                return (
                    f"check_erratum_chosen {cpu_name.name}, "
                    f"ERRATUM({erratum.id}), "
                    f"ERRATA_{cpu_name.flag_name}_{erratum.id}"
                )
            else:
                return (
                    f"check_erratum_hs {cpu_name.name}, "
                    f"ERRATUM({erratum.id}), "
                    f"CPU_REV({start_rev.revision}, {start_rev.patch})"
                )
        else:
            if present_revisions.is_continuous_range_from_release_revision():
                return (
                    f"check_erratum_ls {cpu_name.name}, "
                    f"ERRATUM({erratum.id}), "
                    f"CPU_REV({end_rev.revision}, {end_rev.patch})"
                )
            else:
                return (
                    f"check_erratum_range {cpu_name.name}, "
                    f"ERRATUM({erratum.id}), "
                    f"CPU_REV({start_rev.revision}, {start_rev.patch}), "
                    f"CPU_REV({end_rev.revision}, {end_rev.patch})"
                )
