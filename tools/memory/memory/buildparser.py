#
# Copyright (c) 2023, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

import re
from pathlib import Path

from memory.elfparser import TfaElfParser


class TfaBuildParser:
    """A class for performing analysis on the memory layout of a TF-A build."""

    def __init__(self, path: Path):
        self._modules = dict()
        self._path = path
        self._parse_modules()

    def __getitem__(self, module: str):
        """Returns an TfaElfParser instance indexed by module."""
        return self._modules[module]

    def _parse_modules(self):
        """Parse ELF files in the build path."""
        for elf_file in self._path.glob("**/*.elf"):
            module_name = elf_file.name.split("/")[-1].split(".")[0]
            with open(elf_file, "rb") as file:
                self._modules[module_name] = TfaElfParser(file)

        if not len(self._modules):
            raise FileNotFoundError(
                f"failed to find ELF files in path {self._path}!"
            )

    @property
    def symbols(self) -> list:
        return [
            (*sym, k) for k, v in self._modules.items() for sym in v.symbols
        ]

    @staticmethod
    def filter_symbols(symbols: list, regex: str = None) -> list:
        """Returns a map of symbols to modules."""
        regex = r".*" if not regex else regex
        return sorted(
            filter(lambda s: re.match(regex, s[0]), symbols),
            key=lambda s: (-s[1], s[0]),
            reverse=True,
        )

    @property
    def module_names(self):
        """Returns sorted list of module names."""
        return sorted(self._modules.keys())
