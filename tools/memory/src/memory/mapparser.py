#
# Copyright (c) 2023-2025, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

from collections import defaultdict
from re import match, search
from typing import Dict, TextIO

from memory.image import Image, Region


class TfaMapParser(Image):
    """A class representing a map file built for TF-A.

    Provides a basic interface for reading the symbol table. The constructor
    accepts a file-like object with the contents a Map file. Only GNU map files
    are supported at this stage.
    """

    def __init__(self, map_file: TextIO) -> None:
        self._symbols: Dict[str, int] = self.read_symbols(map_file)
        assert self._symbols, "Symbol table is empty!"

        self._footprint: Dict[str, Region] = defaultdict(Region)

        expr = r".*(.?R.M)_REGION.*(START|END|LENGTH)"
        for symbol in filter(lambda s: match(expr, s), self._symbols):
            region, _, attr = symbol.lower().strip("__").split("_")

            if attr == "start":
                self._footprint[region].start = self._symbols[symbol]
            elif attr == "end":
                self._footprint[region].end = self._symbols[symbol]
            if attr == "length":
                self._footprint[region].length = self._symbols[symbol]

    @property
    def symbols(self) -> Dict[str, int]:
        return self._symbols

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

    @property
    def footprint(self) -> Dict[str, Region]:
        return self._footprint
