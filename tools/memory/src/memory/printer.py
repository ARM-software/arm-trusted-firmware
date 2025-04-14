#
# Copyright (c) 2023-2025, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

from typing import Any, Dict, List, Optional, Tuple

from anytree import RenderTree
from anytree.importer import DictImporter
from prettytable import PrettyTable

from memory.image import Region


class TfaPrettyPrinter:
    """A class for printing the memory layout of ELF files.

    This class provides interfaces for printing various memory layout views of
    ELF files in a TF-A build. It can be used to understand how the memory is
    structured and consumed.
    """

    def __init__(self, columns: int, as_decimal: bool = False) -> None:
        self.term_size: int = columns
        self._tree: Optional[List[str]] = None
        self._symbol_map: Optional[List[str]] = None
        self.as_decimal: bool = as_decimal

    def format_args(
        self,
        *args: Any,
        width: int = 10,
        fmt: Optional[str] = None,
    ) -> List[Any]:
        if not fmt and type(args[0]) is int:
            fmt = f">{width}x" if not self.as_decimal else f">{width}"
        return [f"{arg:{fmt}}" if fmt else arg for arg in args]

    def format_row(
        self,
        leading: str,
        *args: Any,
        width: int = 10,
        fmt: Optional[str] = None,
    ) -> str:
        formatted_args = self.format_args(*args, width=width, fmt=fmt)
        return leading + " ".join(formatted_args)

    @staticmethod
    def map_elf_symbol(
        leading: str,
        section_name: str,
        rel_pos: int,
        columns: int,
        width: int,
        is_edge: bool = False,
    ) -> str:
        empty_col = "{:{}{}}"

        # Some symbols are longer than the column width, truncate them until
        # we find a more elegant way to display them!
        len_over = len(section_name) - width
        if len_over > 0:
            section_name = section_name[len_over:-len_over]

        sec_row = f"+{section_name:-^{width - 1}}+"
        sep, fill = ("+", "-") if is_edge else ("|", "")

        sec_row_l = empty_col.format(sep, fill + "<", width) * rel_pos
        sec_row_r = empty_col.format(sep, fill + ">", width) * (columns - rel_pos - 1)

        return leading + sec_row_l + sec_row + sec_row_r

    def print_footprint(
        self,
        app_mem_usage: Dict[str, Dict[str, Region]],
    ):
        assert len(app_mem_usage), "Empty memory layout dictionary!"

        fields = ["Component", "Start", "Limit", "Size", "Free", "Total"]
        sort_key = fields[0]

        # Iterate through all the memory types, create a table for each
        # type, rows represent a single module.
        for mem in sorted(set(k for _, v in app_mem_usage.items() for k in v)):
            table = PrettyTable(
                sortby=sort_key,
                title=f"Memory Usage (bytes) [{mem.upper()}]",
                field_names=fields,
            )

            for mod, vals in app_mem_usage.items():
                if mem in vals.keys():
                    val = vals[mem]
                    table.add_row(
                        [
                            mod.upper(),
                            *self.format_args(
                                *[
                                    val.start if val.start is not None else "?",
                                    val.limit if val.limit is not None else "?",
                                    val.size if val.size is not None else "?",
                                    val.free if val.free is not None else "?",
                                    val.length if val.length is not None else "?",
                                ]
                            ),
                        ]
                    )
            print(table, "\n")

    def print_symbol_table(
        self,
        symbols: List[Tuple[str, int, str]],
        modules: List[str],
        start: int = 12,
    ) -> None:
        assert len(symbols), "Empty symbol list!"
        modules = sorted(modules)
        col_width = int((self.term_size - start) / len(modules))
        address_fixed_width = 11

        num_fmt = f"0=#0{address_fixed_width}x" if not self.as_decimal else ">10"

        _symbol_map = [
            " " * start + "".join(self.format_args(*modules, fmt=f"^{col_width}"))
        ]
        last_addr = None

        for i, (name, addr, mod) in enumerate(symbols):
            # Do not print out an address twice if two symbols overlap,
            # for example, at the end of one region and start of another.
            leading = f"{addr:{num_fmt}}" + " " if addr != last_addr else " " * start

            _symbol_map.append(
                self.map_elf_symbol(
                    leading,
                    name,
                    modules.index(mod),
                    len(modules),
                    col_width,
                    is_edge=(not i or i == len(symbols) - 1),
                )
            )

            last_addr = addr

        self._symbol_map = ["Memory Layout:"]
        self._symbol_map += list(reversed(_symbol_map))
        print("\n".join(self._symbol_map))

    def print_mem_tree(
        self,
        mem_map_dict: Dict[str, Any],
        modules: List[str],
        depth: int = 1,
        min_pad: int = 12,
        node_right_pad: int = 12,
    ) -> None:
        # Start column should have some padding between itself and its data
        # values.
        anchor = min_pad + node_right_pad * (depth - 1)
        headers = ["start", "end", "size"]

        self._tree = [
            (f"{'name':<{anchor}}" + " ".join(f"{arg:>10}" for arg in headers))
        ]

        for mod in sorted(modules):
            root = DictImporter().import_(mem_map_dict[mod])
            for pre, fill, node in RenderTree(root, maxlevel=depth):
                leading = f"{pre}{node.name}".ljust(anchor)
                self._tree.append(
                    self.format_row(
                        leading,
                        node.start,
                        node.end,
                        node.size,
                    )
                )
        print("\n".join(self._tree), "\n")
