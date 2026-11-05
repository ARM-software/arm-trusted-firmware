# Copyright (c) 2026, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause

import logging
import re
from pathlib import Path

from src.files.file import FileUpdater
from src.models.cpuname import CpuName
from src.models.erratum import Erratum

from .helpers import insert_text_before, update_copyright_year


class HeaderFile(FileUpdater):
    def __init__(self, file_path: Path) -> None:
        super().__init__(
            file_path,
            logging.getLogger(self.__class__.__name__)
        )

    def _update_file(self, cpu_name: CpuName, erratum: Erratum):
        """Add missing register definitions to CPU header file."""

        header_map = HeaderFile._parse_header_file(cpu_name, self.path)
        register_prefix = cpu_name.name.upper()
        define_prefix = f"#define {register_prefix}_"
        end_line = header_map['end']['line'] - 1

        updated = False

        chicken_bits = erratum.workaround_details.chicken_bits

        if len(chicken_bits) > 0:
            for chicken_bit in chicken_bits:
                if HeaderFile._is_register_name_encoding_format(chicken_bit.register_name):
                    continue

                lines = []

                if chicken_bit.register_encoding is not None and chicken_bit.relaxed_register_name not in header_map:
                    define_suffix = f"\t\t\t{chicken_bit.register_encoding.encoded_format()}"
                    lines.append((chicken_bit.relaxed_register_name, define_suffix))

                binary_value = chicken_bit.binary_value
                bit_position = chicken_bit.bit_position
                register_field_name = chicken_bit.register_field_name

                if bit_position is not None and bit_position.width > 1 and register_field_name != "n/a" and binary_value is not None:
                    macros = [
                        (f"{chicken_bit.relaxed_register_name}_{register_field_name}_BIT", f"U({binary_value})"),
                        (f"{chicken_bit.relaxed_register_name}_{register_field_name}_SHIFT", f"U({bit_position.lsb})"),
                        (f"{chicken_bit.relaxed_register_name}_{register_field_name}_WIDTH", f"U({bit_position.width})")
                    ]

                    for macro in macros:
                        if macro[0] not in header_map:
                            lines.append(macro)

                for register, suffix in lines:
                    line = f"{define_prefix}{register}\t\t{suffix}"

                    insert_text_before(self.path, end_line, line)

                    header_map[register] = {
                        "line": end_line,
                        "name": register
                    }
                    end_line += 1

                    updated = True

        if updated:
            update_copyright_year(self.path)

        return updated
    @staticmethod
    def _is_register_name_encoding_format(name: str):
        """Check if name is already in system register encoding format (sX_X_cX_cX_X)."""

        pattern = re.compile(r'^s\d+_\d+_c\d+_c\d+_\d+$', re.IGNORECASE)
        return bool(pattern.match(name))

    @staticmethod
    def _parse_header_file(cpu_name: CpuName, path: Path):
        """
        Parse CPU header file to find existing register definitions.

        Returns dict mapping register names to line numbers.
        """
        with open(path, "r") as f:
            lines = f.readlines()

        map = {}
        relaxed_header_macro = cpu_name.name.upper().replace("_", "") + "H"

        name = cpu_name.name.upper()

        register_pattern = re.compile(
            rf"^\s*#define\s+{re.escape(name)}_(?P<reg_name>[A-Z0-9_]+)\s+(?P<value>.+?)[ \t]*$"
        )

        for i, line in enumerate(lines, start=1):
            stripped = line.rstrip()

            if (
                "#ifndef __ASSEMBLER__" in stripped
                or f"#endif /* {relaxed_header_macro} */" in stripped.replace("_", "")
            ):
                map['end'] = {"line": i}
                break
            elif match:= register_pattern.search(stripped):
                    reg_name = match.group("reg_name")

                    map[reg_name] = {
                        "name": reg_name,
                        "line": i
                    }

        return map
