#
# Copyright (c) 2023, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

import re
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

        self._memory_layout = self.get_memory_layout_from_symbols()

    @property
    def symbols(self):
        return self._symbols.items()

    def get_memory_layout_from_symbols(self, expr=None) -> dict:
        """Retrieve information about the memory configuration from the symbol
        table.
        """
        assert len(self._symbols), "Symbol table is empty!"

        expr = r".*(.?R.M)_REGION.*(START|END|LENGTH)" if not expr else expr
        region_symbols = filter(lambda s: re.match(expr, s), self._symbols)
        memory_layout = {}

        for symbol in region_symbols:
            region, _, attr = tuple(symbol.lower().strip("__").split("_"))
            if region not in memory_layout:
                memory_layout[region] = {}

            # Retrieve the value of the symbol using the symbol as the key.
            memory_layout[region][attr] = self._symbols[symbol]

        return memory_layout

    def get_elf_memory_layout(self):
        """Get the total memory consumed by this module from the memory
        configuration.
            {"rom": {"start": 0x0, "end": 0xFF, "length": ... }
        """
        mem_dict = {}

        for mem, attrs in self._memory_layout.items():
            limit = attrs["start"] + attrs["length"]
            mem_dict[mem] = {
                "start": attrs["start"],
                "limit": limit,
                "size": attrs["end"] - attrs["start"],
                "free": limit - attrs["end"],
                "total": attrs["length"],
            }
        return mem_dict
