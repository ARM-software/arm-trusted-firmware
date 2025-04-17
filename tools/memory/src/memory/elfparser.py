#
# Copyright (c) 2023-2025, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

import re
from dataclasses import asdict, dataclass
from typing import (
    Any,
    BinaryIO,
    Dict,
    Iterable,
    List,
    Optional,
    Tuple,
    Union,
)

from elftools.elf.elffile import ELFFile
from elftools.elf.sections import Section, SymbolTableSection
from elftools.elf.segments import Segment

from memory.image import Image, Region


@dataclass(frozen=True)
class TfaMemObject:
    name: str
    start: int
    end: int
    size: int
    children: List["TfaMemObject"]


class TfaElfParser(Image):
    """A class representing an ELF file built for TF-A.

    Provides a basic interface for reading the symbol table and other
    attributes of an ELF file. The constructor accepts a file-like object with
    the contents an ELF file.
    """

    def __init__(self, elf_file: BinaryIO) -> None:
        self._segments: Dict[int, TfaMemObject] = {}
        self._memory_layout: Dict[str, Dict[str, int]] = {}

        elf = ELFFile(elf_file)

        symtab = elf.get_section_by_name(".symtab")
        assert isinstance(symtab, SymbolTableSection)

        self._symbols: Dict[str, int] = {
            sym.name: sym.entry["st_value"] for sym in symtab.iter_symbols()
        }

        self.set_segment_section_map(elf.iter_segments(), elf.iter_sections())
        self._memory_layout = self.get_memory_layout_from_symbols()
        self._start: int = elf["e_entry"]
        self._size: int
        self._free: int
        self._size, self._free = self._get_mem_usage()
        self._end: int = self._start + self._size

        self._footprint: Dict[str, Region] = {}

        for mem, attrs in self._memory_layout.items():
            self._footprint[mem] = Region(
                attrs["start"],
                attrs["end"],
                attrs["length"],
            )

    @property
    def symbols(self) -> Dict[str, int]:
        return self._symbols

    @staticmethod
    def tfa_mem_obj_factory(
        elf_obj: Union[Segment, Section],
        name: Optional[str] = None,
        children: Optional[List[TfaMemObject]] = None,
    ) -> TfaMemObject:
        """Converts a pyelfparser Segment or Section to a TfaMemObject."""
        # Ensure each segment is provided a name since they aren't in the
        # program header.
        assert not (isinstance(elf_obj, Segment) and name is None), (
            "Attempting to make segment without a name"
        )

        # Segment and sections header keys have different prefixes.
        vaddr = "p_vaddr" if isinstance(elf_obj, Segment) else "sh_addr"
        size = "p_memsz" if isinstance(elf_obj, Segment) else "sh_size"

        name = name if isinstance(elf_obj, Segment) else elf_obj.name
        assert name is not None

        # TODO figure out how to handle free space for sections and segments
        return TfaMemObject(
            name,
            elf_obj[vaddr],
            elf_obj[vaddr] + elf_obj[size],
            elf_obj[size],
            children or [],
        )

    def _get_mem_usage(self) -> Tuple[int, int]:
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

    def set_segment_section_map(
        self,
        segments: Iterable[Segment],
        sections: Iterable[Section],
    ) -> None:
        """Set segment to section mappings."""
        segments = filter(lambda seg: seg["p_type"] == "PT_LOAD", segments)
        segments_list = list(segments)

        for sec in sections:
            for n, seg in enumerate(segments_list):
                if seg.section_in_segment(sec):
                    if n not in self._segments:
                        self._segments[n] = self.tfa_mem_obj_factory(
                            seg, name=f"{n:#02}"
                        )

                    self._segments[n].children.append(self.tfa_mem_obj_factory(sec))

    def get_memory_layout_from_symbols(self) -> Dict[str, Dict[str, int]]:
        """Retrieve information about the memory configuration from the symbol
        table.
        """
        assert self._symbols, "Symbol table is empty!"

        expr = r".*(.?R.M)_REGION.*(START|END|LENGTH)"
        region_symbols = filter(lambda s: re.match(expr, s), self._symbols)
        memory_layout: Dict[str, Dict[str, int]] = {}

        for symbol in region_symbols:
            region, _, attr = symbol.lower().strip("__").split("_")
            if region not in memory_layout:
                memory_layout[region] = {}

            # Retrieve the value of the symbol using the symbol as the key.
            memory_layout[region][attr] = self._symbols[symbol]

        return memory_layout

    def get_seg_map_as_dict(self) -> List[Dict[str, Any]]:
        """Get a dictionary of segments and their section mappings."""
        return [asdict(segment) for segment in self._segments.values()]

    def get_memory_layout(self) -> Dict[str, Region]:
        """Get the total memory consumed by this module from the memory
        configuration.
        """
        mem_dict: Dict[str, Region] = {}

        for mem, attrs in self._memory_layout.items():
            mem_dict[mem] = Region(
                attrs["start"],
                attrs["end"],
                attrs["length"],
            )

        return mem_dict

    @property
    def footprint(self) -> Dict[str, Region]:
        return self._footprint

    def get_mod_mem_usage_dict(self) -> Dict[str, int]:
        """Get the total memory consumed by the module, this combines the
        information in the memory configuration.
        """
        return {
            "start": self._start,
            "end": self._end,
            "size": self._size,
            "free": self._free,
        }
