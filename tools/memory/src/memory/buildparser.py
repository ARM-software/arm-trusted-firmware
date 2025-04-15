#
# Copyright (c) 2023-2025, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

import re
from pathlib import Path
from typing import (
    Any,
    Dict,
    List,
    Union,
)

from memory.elfparser import TfaElfParser
from memory.image import Region
from memory.mapparser import TfaMapParser


class TfaBuildParser:
    """A class for performing analysis on the memory layout of a TF-A build."""

    def __init__(self, path: Path, map_backend: bool = False) -> None:
        self._modules: Dict[str, Union[TfaElfParser, TfaMapParser]] = {}
        self._path: Path = path
        self.map_backend: bool = map_backend
        self._parse_modules()

    def __getitem__(self, module: str) -> Union[TfaElfParser, TfaMapParser]:
        """Returns an TfaElfParser instance indexed by module."""
        return self._modules[module]

    def _parse_modules(self) -> None:
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
    def symbols(self) -> Dict[str, Dict[str, int]]:
        return {k: v.symbols for k, v in self._modules.items()}

    @staticmethod
    def filter_symbols(
        images: Dict[str, Dict[str, int]], regex: str
    ) -> Dict[str, Dict[str, int]]:
        """Returns a map of symbols to modules."""

        return {
            image: {
                symbol: symbol_value
                for symbol, symbol_value in symbols.items()
                if re.match(regex, symbol)
            }
            for image, symbols in images.items()
        }

    def get_mem_usage_dict(self) -> Dict[str, Dict[str, Region]]:
        """Returns map of memory usage per memory type for each module."""
        return {k: v.footprint for k, v in self._modules.items()}

    def get_mem_tree_as_dict(self) -> Dict[str, Dict[str, Any]]:
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
    def module_names(self) -> List[str]:
        """Returns sorted list of module names."""
        return sorted(self._modules.keys())
