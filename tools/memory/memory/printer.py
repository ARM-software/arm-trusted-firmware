#
# Copyright (c) 2023, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

from anytree import RenderTree
from anytree.importer import DictImporter
from prettytable import PrettyTable


class TfaPrettyPrinter:
    """A class for printing the memory layout of ELF files.

    This class provides interfaces for printing various memory layout views of
    ELF files in a TF-A build. It can be used to understand how the memory is
    structured and consumed.
    """

    def __init__(self, columns: int = None, as_decimal: bool = False):
        self.term_size = columns if columns and columns > 120 else 120
        self._tree = None
        self._footprint = None
        self._symbol_map = None
        self.as_decimal = as_decimal

    def format_args(self, *args, width=10, fmt=None):
        if not fmt and type(args[0]) is int:
            fmt = f">{width}x" if not self.as_decimal else f">{width}"
        return [f"{arg:{fmt}}" if fmt else arg for arg in args]

    def format_row(self, leading, *args, width=10, fmt=None):
        formatted_args = self.format_args(*args, width=width, fmt=fmt)
        return leading + " ".join(formatted_args)

    @staticmethod
    def map_elf_symbol(
        leading: str,
        section_name: str,
        rel_pos: int,
        columns: int,
        width: int = None,
        is_edge: bool = False,
    ):
        empty_col = "{:{}{}}"

        # Some symbols are longer than the column width, truncate them until
        # we find a more elegant way to display them!
        len_over = len(section_name) - width
        if len_over > 0:
            section_name = section_name[len_over:-len_over]

        sec_row = f"+{section_name:-^{width-1}}+"
        sep, fill = ("+", "-") if is_edge else ("|", "")

        sec_row_l = empty_col.format(sep, fill + "<", width) * rel_pos
        sec_row_r = empty_col.format(sep, fill + ">", width) * (
            columns - rel_pos - 1
        )

        return leading + sec_row_l + sec_row + sec_row_r

    def print_footprint(
        self, app_mem_usage: dict, sort_key: str = None, fields: list = None
    ):
        assert len(app_mem_usage), "Empty memory layout dictionary!"
        if not fields:
            fields = ["Component", "Start", "Limit", "Size", "Free", "Total"]

        sort_key = fields[0] if not sort_key else sort_key

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
                                *[val[k.lower()] for k in fields[1:]]
                            ),
                        ]
                    )
            print(table, "\n")

    def print_symbol_table(
        self,
        symbols: list,
        modules: list,
        start: int = 12,
    ):
        assert len(symbols), "Empty symbol list!"
        modules = sorted(modules)
        col_width = int((self.term_size - start) / len(modules))
        address_fixed_width = 11

        num_fmt = (
            f"0=#0{address_fixed_width}x" if not self.as_decimal else ">10"
        )

        _symbol_map = [
            " " * start
            + "".join(self.format_args(*modules, fmt=f"^{col_width}"))
        ]
        last_addr = None

        for i, (name, addr, mod) in enumerate(symbols):
            # Do not print out an address twice if two symbols overlap,
            # for example, at the end of one region and start of another.
            leading = (
                f"{addr:{num_fmt}}" + " " if addr != last_addr else " " * start
            )

            _symbol_map.append(
                self.map_elf_symbol(
                    leading,
                    name,
                    modules.index(mod),
                    len(modules),
                    width=col_width,
                    is_edge=(not i or i == len(symbols) - 1),
                )
            )

            last_addr = addr

        self._symbol_map = ["Memory Layout:"]
        self._symbol_map += list(reversed(_symbol_map))
        print("\n".join(self._symbol_map))

    def print_mem_tree(
        self, mem_map_dict, modules, depth=1, min_pad=12, node_right_pad=12
    ):
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
