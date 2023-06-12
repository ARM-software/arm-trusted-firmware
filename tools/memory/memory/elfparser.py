#
# Copyright (c) 2023, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

import re
from dataclasses import asdict, dataclass
from typing import BinaryIO

from elftools.elf.elffile import ELFFile


@dataclass(frozen=True)
class TfaMemObject:
    name: str
    start: int
    end: int
    size: int
    children: list


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

        self.set_segment_section_map(elf.iter_segments(), elf.iter_sections())
        self._memory_layout = self.get_memory_layout_from_symbols()
        self._start = elf["e_entry"]
        self._size, self._free = self._get_mem_usage()
        self._end = self._start + self._size

    @property
    def symbols(self):
        return self._symbols.items()

    @staticmethod
    def tfa_mem_obj_factory(elf_obj, name=None, children=None, segment=False):
        """Converts a pyelfparser Segment or Section to a TfaMemObject."""
        # Ensure each segment is provided a name since they aren't in the
        # program header.
        assert not (
            segment and name is None
        ), "Attempting to make segment without a name"

        if children is None:
            children = list()

        # Segment and sections header keys have different prefixes.
        vaddr = "p_vaddr" if segment else "sh_addr"
        size = "p_memsz" if segment else "sh_size"

        # TODO figure out how to handle free space for sections and segments
        return TfaMemObject(
            name if segment else elf_obj.name,
            elf_obj[vaddr],
            elf_obj[vaddr] + elf_obj[size],
            elf_obj[size],
            [] if not children else children,
        )

    def _get_mem_usage(self) -> (int, int):
        """Get total size and free space for this component."""
        size = free = 0

        # Use information encoded in the segment header if we can't get a
        # memory configuration.
        if not self._memory_layout:
            return sum(s.size for s in self._segments.values()), 0

        for v in self._memory_layout.values():
            size += v["length"]
            free += v["start"] + v["length"] - v["end"]

        return size, free

    def set_segment_section_map(self, segments, sections):
        """Set segment to section mappings."""
        segments = list(
            filter(lambda seg: seg["p_type"] == "PT_LOAD", segments)
        )

        for sec in sections:
            for n, seg in enumerate(segments):
                if seg.section_in_segment(sec):
                    if n not in self._segments.keys():
                        self._segments[n] = self.tfa_mem_obj_factory(
                            seg, name=f"{n:#02}", segment=True
                        )

                    self._segments[n].children.append(
                        self.tfa_mem_obj_factory(sec)
                    )

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

    def get_seg_map_as_dict(self):
        """Get a dictionary of segments and their section mappings."""
        return [asdict(v) for k, v in self._segments.items()]

    def get_memory_layout(self):
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

    def get_mod_mem_usage_dict(self):
        """Get the total memory consumed by the module, this combines the
        information in the memory configuration.
        """
        return {
            "start": self._start,
            "end": self._end,
            "size": self._size,
            "free": self._free,
        }
