#
# Copyright (c) 2016-2025, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: Apache-2.0
#

import json
import os
import re
from collections import defaultdict
from copy import deepcopy
from os.path import (
    abspath,
    basename,
    commonprefix,
    dirname,
    join,
    relpath,
    splitext,
)
from pathlib import Path
from sys import stdout
from typing import IO, Any, Dict, List, Optional, Pattern, Tuple, Union

from jinja2 import FileSystemLoader, StrictUndefined
from jinja2.environment import Environment
from prettytable import HEADER, PrettyTable

ModuleStats = Dict[str, int]
Modules = Dict[str, ModuleStats]

SECTIONS: Tuple[str, ...] = (".text", ".data", ".bss", ".heap", ".stack")
MISC_FLASH_SECTIONS: Tuple[str, ...] = (".interrupts", ".flash_config")
OTHER_SECTIONS: Tuple[str, ...] = (
    ".interrupts_ram",
    ".init",
    ".ARM.extab",
    ".ARM.exidx",
    ".ARM.attributes",
    ".eh_frame",
    ".init_array",
    ".fini_array",
    ".jcr",
    ".stab",
    ".stabstr",
    ".ARM.exidx",
    ".ARM",
)
ALL_SECTIONS: Tuple[str, ...] = (
    SECTIONS + OTHER_SECTIONS + MISC_FLASH_SECTIONS + ("unknown", "OUTPUT")
)


class Parser:
    """Internal interface for parsing"""

    _RE_OBJECT_FILE: Pattern[str] = re.compile(r"^(.+\/.+\.o(bj)?)$")
    _RE_LIBRARY_OBJECT: Pattern[str] = re.compile(
        r"((^.+" + r"" + r"lib.+\.a)\((.+\.o(bj)?)\))$"
    )
    _RE_STD_SECTION: Pattern[str] = re.compile(r"^\s+.*0x(\w{8,16})\s+0x(\w+)\s(.+)$")
    _RE_FILL_SECTION: Pattern[str] = re.compile(
        r"^\s*\*fill\*\s+0x(\w{8,16})\s+0x(\w+).*$"
    )
    _RE_TRANS_FILE: Pattern[str] = re.compile(r"^(.+\/|.+\.ltrans.o(bj)?)$")
    _OBJECT_EXTENSIONS: Tuple[str, ...] = (".o", ".obj")

    _modules: Modules
    _fill: bool

    def __init__(self, fill: bool = True):
        self._modules: Modules = {}
        self._fill = fill

    def module_add(self, object_name: str, size: int, section: str):
        """Adds a module or section to the list

        Positional arguments:
        object_name - name of the entry to add
        size - the size of the module being added
        section - the section the module contributes to
        """
        if (
            not object_name
            or not size
            or not section
            or (not self._fill and object_name == "[fill]")
        ):
            return

        if object_name in self._modules:
            self._modules[object_name].setdefault(section, 0)
            self._modules[object_name][section] += size
            return

        obj_split = os.sep + basename(object_name)
        for module_path, contents in self._modules.items():
            if module_path.endswith(obj_split) or module_path == object_name:
                contents.setdefault(section, 0)
                contents[section] += size
                return

        new_module: ModuleStats = defaultdict(int)
        new_module[section] = size
        self._modules[object_name] = new_module

    def module_replace(self, old_object: str, new_object: str):
        """Replaces an object name with a new one"""
        if old_object in self._modules:
            self._modules[new_object] = self._modules.pop(old_object)

    def check_new_section(self, line: str) -> Optional[str]:
        """Check whether a new section in a map file has been detected

        Positional arguments:
        line - the line to check for a new section

        return value - A section name, if a new section was found, None
                       otherwise
        """
        line_s = line.strip()
        for i in ALL_SECTIONS:
            if line_s.startswith(i):
                return i
        if line.startswith("."):
            return "unknown"
        else:
            return None

    def parse_object_name(self, line: str) -> str:
        """Parse a path to object file

        Positional arguments:
        line - the path to parse the object and module name from

        return value - an object file name
        """
        if re.match(self._RE_TRANS_FILE, line):
            return "[misc]"

        test_re_file_name = re.match(self._RE_OBJECT_FILE, line)

        if test_re_file_name:
            object_name = test_re_file_name.group(1)

            return object_name
        else:
            test_re_obj_name = re.match(self._RE_LIBRARY_OBJECT, line)

            if test_re_obj_name:
                return join(test_re_obj_name.group(2), test_re_obj_name.group(3))
            else:
                if not line.startswith("LONG") and not line.startswith("linker stubs"):
                    print("Unknown object name found in GCC map file: %s" % line)
                return "[misc]"

    def parse_section(self, line: str) -> Tuple[str, int]:
        """Parse data from a section of gcc map file

        examples:
                        0x00004308       0x7c ./BUILD/K64F/GCC_ARM/spi_api.o
         .text          0x00000608      0x198 ./BUILD/K64F/HAL_CM4.o

        Positional arguments:
        line - the line to parse a section from
        """
        is_fill = re.match(self._RE_FILL_SECTION, line)
        if is_fill:
            o_name: str = "[fill]"
            o_size: int = int(is_fill.group(2), 16)
            return o_name, o_size

        is_section = re.match(self._RE_STD_SECTION, line)
        if is_section:
            o_size: int = int(is_section.group(2), 16)
            if o_size:
                o_name: str = self.parse_object_name(is_section.group(3))
                return o_name, o_size

        return "", 0

    def parse_mapfile(self, file_desc: IO[str]) -> Modules:
        """Main logic to decode gcc map files

        Positional arguments:
        file_desc - a stream object to parse as a gcc map file
        """
        current_section: str = "unknown"

        with file_desc as infile:
            for line in infile:
                if line.startswith("Linker script and memory map"):
                    current_section = "unknown"
                    break

            for line in infile:
                next_section = self.check_new_section(line)

                if next_section == "OUTPUT":
                    break
                elif next_section:
                    current_section = next_section

                object_name, object_size = self.parse_section(line)
                self.module_add(object_name, object_size, current_section)

        def is_obj(name: str) -> bool:
            return not name.startswith("[") or not name.endswith("]")

        common_prefix: str = dirname(
            commonprefix([o for o in self._modules.keys() if is_obj(o)])
        )
        new_modules: Modules = {}
        for name, stats in self._modules.items():
            if is_obj(name):
                new_modules[relpath(name, common_prefix)] = stats
            else:
                new_modules[name] = stats
        return new_modules


class MapParser(object):
    """An object that represents parsed results, parses the memory map files,
    and writes out different file types of memory results
    """

    print_sections: Tuple[str, ...] = (".text", ".data", ".bss")
    delta_sections: Tuple[str, ...] = (".text-delta", ".data-delta", ".bss-delta")

    # sections to print info (generic for all toolchains)
    sections: Tuple[str, ...] = SECTIONS
    misc_flash_sections: Tuple[str, ...] = MISC_FLASH_SECTIONS
    other_sections: Tuple[str, ...] = OTHER_SECTIONS

    modules: Modules
    old_modules: Modules
    short_modules: Modules
    mem_report: List[Dict[str, Union[str, ModuleStats]]]
    mem_summary: Dict[str, int]
    subtotal: Dict[str, int]
    tc_name: Optional[str]

    RAM_FORMAT_STR: str = "Total Static RAM memory (data + bss): {}({:+}) bytes\n"
    ROM_FORMAT_STR: str = "Total Flash memory (text + data): {}({:+}) bytes\n"

    def __init__(self):
        # list of all modules and their sections
        # full list - doesn't change with depth
        self.modules: Modules = {}
        self.old_modules = {}
        # short version with specific depth
        self.short_modules: Modules = {}

        # Memory report (sections + summary)
        self.mem_report: List[Dict[str, Union[str, ModuleStats]]] = []

        # Memory summary
        self.mem_summary: Dict[str, int] = {}

        # Totals of ".text", ".data" and ".bss"
        self.subtotal: Dict[str, int] = {}

        # Name of the toolchain, for better headings
        self.tc_name = None

    def reduce_depth(self, depth: Optional[int]):
        """
        populates the short_modules attribute with a truncated module list

        (1) depth = 1:
        main.o
        mbed-os

        (2) depth = 2:
        main.o
        mbed-os/test.o
        mbed-os/drivers

        """
        if depth == 0 or depth is None:
            self.short_modules = deepcopy(self.modules)
        else:
            self.short_modules = dict()
            for module_name, v in self.modules.items():
                split_name = module_name.split(os.sep)
                if split_name[0] == "":
                    split_name = split_name[1:]
                new_name = join(*split_name[:depth])
                self.short_modules.setdefault(new_name, defaultdict(int))
                for section_idx, value in v.items():
                    self.short_modules[new_name][section_idx] += value
                    delta_name = section_idx + "-delta"
                    self.short_modules[new_name][delta_name] += value

            for module_name, v in self.old_modules.items():
                split_name = module_name.split(os.sep)
                if split_name[0] == "":
                    split_name = split_name[1:]
                new_name = join(*split_name[:depth])
                self.short_modules.setdefault(new_name, defaultdict(int))
                for section_idx, value in v.items():
                    delta_name = section_idx + "-delta"
                    self.short_modules[new_name][delta_name] -= value

    export_formats: List[str] = ["json", "html", "table"]

    def generate_output(
        self,
        export_format: str,
        depth: Optional[int],
        file_output: Optional[str] = None,
    ) -> Optional[bool]:
        """Generates summary of memory map data

        Positional arguments:
        export_format - the format to dump

        Keyword arguments:
        file_desc - descriptor (either stdout or file)
        depth - directory depth on report

        Returns: generated string for the 'table' format, otherwise Nonef
        """
        if depth is None or depth > 0:
            self.reduce_depth(depth)
        self.compute_report()
        try:
            if file_output:
                file_desc = open(file_output, "w")
            else:
                file_desc = stdout
        except IOError as error:
            print("I/O error({0}): {1}".format(error.errno, error.strerror))
            return False

        to_call = {
            "json": self.generate_json,
            "html": self.generate_html,
            "table": self.generate_table,
        }[export_format]
        to_call(file_desc)

        if file_desc is not stdout:
            file_desc.close()

    @staticmethod
    def _move_up_tree(tree: Dict[str, Any], next_module: str) -> Dict[str, Any]:
        tree.setdefault("children", [])
        for child in tree["children"]:
            if child["name"] == next_module:
                return child

        new_module = {"name": next_module, "value": 0, "delta": 0}
        tree["children"].append(new_module)

        return new_module

    def generate_html(self, file_desc: IO[str]):
        """Generate a json file from a memory map for D3

        Positional arguments:
        file_desc - the file to write out the final report to
        """

        tree_text = {"name": ".text", "value": 0, "delta": 0}
        tree_bss = {"name": ".bss", "value": 0, "delta": 0}
        tree_data = {"name": ".data", "value": 0, "delta": 0}

        def accumulate(tree_root: Dict[str, Any], size_key: str, stats: ModuleStats):
            parts = module_name.split(os.sep)

            val = stats.get(size_key, 0)
            tree_root["value"] += val
            tree_root["delta"] += val

            cur = tree_root
            for part in parts:
                cur = self._move_up_tree(cur, part)
                cur["value"] += val
                cur["delta"] += val

        def subtract(tree_root: Dict[str, Any], size_key: str, stats: ModuleStats):
            parts = module_name.split(os.sep)

            cur = tree_root
            cur["delta"] -= stats.get(size_key, 0)

            for part in parts:
                children = {c["name"]: c for c in cur.get("children", [])}
                if part not in children:
                    return

                cur = children[part]
                cur["delta"] -= stats.get(size_key, 0)

        for module_name, dct in self.modules.items():
            accumulate(tree_text, ".text", dct)
            accumulate(tree_data, ".data", dct)
            accumulate(tree_bss, ".bss", dct)

        for module_name, dct in self.old_modules.items():
            subtract(tree_text, ".text", dct)
            subtract(tree_data, ".data", dct)
            subtract(tree_bss, ".bss", dct)

        jinja_loader = FileSystemLoader(dirname(abspath(__file__)))
        jinja_environment = Environment(loader=jinja_loader, undefined=StrictUndefined)
        template = jinja_environment.get_template("templates/summary-flamegraph.html")

        name, _ = splitext(basename(file_desc.name))

        if name.endswith("_map"):
            name = name[:-4]
        if self.tc_name:
            name = f"{name} {self.tc_name}"

        file_desc.write(
            template.render(
                {
                    "name": name,
                    "rom": json.dumps(
                        {
                            "name": "ROM",
                            "value": tree_text["value"] + tree_data["value"],
                            "delta": tree_text["delta"] + tree_data["delta"],
                            "children": [tree_text, tree_data],
                        }
                    ),
                    "ram": json.dumps(
                        {
                            "name": "RAM",
                            "value": tree_bss["value"] + tree_data["value"],
                            "delta": tree_bss["delta"] + tree_data["delta"],
                            "children": [tree_bss, tree_data],
                        }
                    ),
                }
            )
        )

    def generate_json(self, file_desc: IO[str]):
        """Generate a json file from a memory map

        Positional arguments:
        file_desc - the file to write out the final report to
        """
        file_desc.write(json.dumps(self.mem_report, indent=4))
        file_desc.write("\n")

    def generate_table(self, file_desc: IO[str]):
        """Generate a table from a memory map

        Returns: string of the generated table
        """
        # Create table
        columns = ["Module"]
        columns.extend(self.print_sections)

        table = PrettyTable(columns, junction_char="|", hrules=HEADER)
        table.align["Module"] = "l"

        for col in self.print_sections:
            table.align[col] = "r"

        for i in sorted(self.short_modules):
            row = [i]

            for k in self.print_sections:
                row.append(
                    "{}({:+})".format(
                        self.short_modules[i][k], self.short_modules[i][k + "-delta"]
                    )
                )

            table.add_row(row)

        subtotal_row = ["Subtotals"]
        for k in self.print_sections:
            subtotal_row.append(
                "{}({:+})".format(self.subtotal[k], self.subtotal[k + "-delta"])
            )

        table.add_row(subtotal_row)

        output = table.get_string()
        output += "\n"

        output += self.RAM_FORMAT_STR.format(
            self.mem_summary["static_ram"], self.mem_summary["static_ram_delta"]
        )
        output += self.ROM_FORMAT_STR.format(
            self.mem_summary["total_flash"], self.mem_summary["total_flash_delta"]
        )
        file_desc.write(output)

    def compute_report(self):
        """Generates summary of memory usage for main areas"""
        self.subtotal = defaultdict(int)

        for mod in self.modules.values():
            for k in self.sections:
                self.subtotal[k] += mod[k]
                self.subtotal[k + "-delta"] += mod[k]

        for mod in self.old_modules.values():
            for k in self.sections:
                self.subtotal[k + "-delta"] -= mod[k]

        self.mem_summary = {
            "static_ram": self.subtotal[".data"] + self.subtotal[".bss"],
            "static_ram_delta": self.subtotal[".data-delta"]
            + self.subtotal[".bss-delta"],
            "total_flash": (self.subtotal[".text"] + self.subtotal[".data"]),
            "total_flash_delta": self.subtotal[".text-delta"]
            + self.subtotal[".data-delta"],
        }

        self.mem_report = []
        if self.short_modules:
            for name, sizes in sorted(self.short_modules.items()):
                self.mem_report.append(
                    {
                        "module": name,
                        "size": {
                            k: sizes.get(k, 0)
                            for k in (self.print_sections + self.delta_sections)
                        },
                    }
                )

        self.mem_report.append({"summary": self.mem_summary})

    def parse(
        self, mapfile: Path, oldfile: Optional[Path] = None, no_fill: bool = False
    ) -> bool:
        """Parse and decode map file depending on the toolchain

        Positional arguments:
        mapfile - the file name of the memory map file
        toolchain - the toolchain used to create the file
        """
        try:
            with open(mapfile, "r") as file_input:
                self.modules = Parser(not no_fill).parse_mapfile(file_input)
            try:
                if oldfile is not None:
                    with open(oldfile, "r") as old_input:
                        self.old_modules = Parser(not no_fill).parse_mapfile(old_input)
                else:
                    self.old_modules = self.modules
            except IOError:
                self.old_modules = {}
            return True

        except IOError as error:
            print("I/O error({0}): {1}".format(error.errno, error.strerror))
            return False
