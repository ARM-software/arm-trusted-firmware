#
# Copyright (c) 2023, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

from typing import BinaryIO

from elftools.elf.elffile import ELFFile


class TfaElfParser:
    """A class representing an ELF file built for TF-A.

    Provides a basic interface for reading the symbol table and other
    attributes of an ELF file. The constructor accepts a file-like object with
    the contents an ELF file.
    """

    def __init__(self, elf_file: BinaryIO):
        self._segments = {}
        self._memory_layout = {}

        elf = ELFFile(elf_file)

        self._symbols = {
            sym.name: sym.entry["st_value"]
            for sym in elf.get_section_by_name(".symtab").iter_symbols()
        }

    @property
    def symbols(self):
        return self._symbols.items()
