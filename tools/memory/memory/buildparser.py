#
# Copyright (c) 2023, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

import re
from pathlib import Path

from memory.elfparser import TfaElfParser
from memory.mapparser import TfaMapParser


class TfaBuildParser:
    """A class for performing analysis on the memory layout of a TF-A build."""

    def __init__(self, path: Path, map_backend=False):
        self._modules = dict()
        self._path = path
        self.map_backend = map_backend
        self._parse_modules()

    def __getitem__(self, module: str):
        """Returns an TfaElfParser instance indexed by module."""
        return self._modules[module]

    def _parse_modules(self):
        """Parse the build files using the selected backend."""
        backend = TfaElfParser
        files = list(self._path.glob("**/*.elf"))
        io_perms = "rb"

        if self.map_backend or len(files) == 0:
            backend = TfaMapParser
            files = self._path.glob("**/*.map")
            io_perms = "r"

        for file in files:
            module_name = file.name.split("/")[-1].split(".")[0]
            with open(file, io_perms) as f:
                self._modules[module_name] = backend(f)

        if not len(self._modules):
            raise FileNotFoundError(
                f"failed to find files to analyse in path {self._path}!"
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

    def get_mem_usage_dict(self) -> dict:
        """Returns map of memory usage per memory type for each module."""
        mem_map = {}
        for k, v in self._modules.items():
            mod_mem_map = v.get_memory_layout()
            if len(mod_mem_map):
                mem_map[k] = mod_mem_map
        return mem_map

    def get_mem_tree_as_dict(self) -> dict:
        """Returns _tree of modules, segments and segments and their total
        memory usage."""
        return {
            k: {
                "name": k,
                **v.get_mod_mem_usage_dict(),
                **{"children": v.get_seg_map_as_dict()},
            }
            for k, v in self._modules.items()
        }

    @property
    def module_names(self):
        """Returns sorted list of module names."""
        return sorted(self._modules.keys())
