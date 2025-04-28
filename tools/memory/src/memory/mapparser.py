#
# Copyright (c) 2023-2025, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

from re import match, search
from typing import Dict, ItemsView, TextIO


class TfaMapParser:
    """A class representing a map file built for TF-A.

    Provides a basic interface for reading the symbol table. The constructor
    accepts a file-like object with the contents a Map file. Only GNU map files
    are supported at this stage.
    """

    def __init__(self, map_file: TextIO) -> None:
        self._symbols: Dict[str, int] = self.read_symbols(map_file)

    @property
    def symbols(self) -> ItemsView[str, int]:
        return self._symbols.items()

    @staticmethod
    def read_symbols(file: TextIO) -> Dict[str, int]:
        pattern = r"\b(0x\w*)\s*(\w*)\s="
        symbols: Dict[str, int] = {}

        for line in file.readlines():
            match = search(pattern, line)

            if match is not None:
                value, name = match.groups()
                symbols[name] = int(value, 16)

        return symbols

    def get_memory_layout(self) -> Dict[str, Dict[str, int]]:
        """Get the total memory consumed by this module from the memory
        configuration.
            {"rom": {"start": 0x0, "end": 0xFF, "length": ... }
        """
        assert len(self._symbols), "Symbol table is empty!"
        expr = r".*(.?R.M)_REGION.*(START|END|LENGTH)"
        memory_layout: Dict[str, Dict[str, int]] = {}

        region_symbols = filter(lambda s: match(expr, s), self._symbols)

        for symbol in region_symbols:
            region, _, attr = tuple(symbol.lower().strip("__").split("_"))
            if region not in memory_layout:
                memory_layout[region] = {}

            memory_layout[region][attr] = self._symbols[symbol]

            if "start" and "length" and "end" in memory_layout[region]:
                memory_layout[region]["limit"] = (
                    memory_layout[region]["start"] + memory_layout[region]["length"]
                )
                memory_layout[region]["free"] = (
                    memory_layout[region]["limit"] - memory_layout[region]["end"]
                )
                memory_layout[region]["total"] = memory_layout[region]["length"]
                memory_layout[region]["size"] = (
                    memory_layout[region]["end"] - memory_layout[region]["start"]
                )

        return memory_layout
