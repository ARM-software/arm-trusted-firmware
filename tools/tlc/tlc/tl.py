#!/usr/bin/env python3

#
# Copyright (c) 2024, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

"""Module containing definitions pertaining to the 'Transfer List' (TL) type."""

from typing import Any, Dict, List, Optional

import math
import struct
from dataclasses import dataclass
from functools import reduce
from pathlib import Path

from tlc.te import TransferEntry

TRANSFER_LIST_ENABLE_CHECKSUM = 0b1

# Description of each TE type. For each TE, there is a tag ID, a format (to be
# used in struct.pack to encode the TE), and a list of field names that can
# appear in the yaml file for that TE. Some fields are missing, if that TE has
# to be processed differently, or if it can only be added with a blob file.
transfer_entry_formats: Dict[int, Any] = {
    0: {
        "tag_name": "empty",
        "format": "4x",
        "fields": [],
    },
    1: {
        "tag_name": "fdt",
    },
    2: {
        "tag_name": "hob_block",
    },
    3: {
        "tag_name": "hob_list",
    },
    4: {
        "tag_name": "acpi_table_aggregate",
    },
    5: {
        "tag_name": "tpm_event_log_table",
        "fields": ["event_log", "flags"],
    },
    6: {
        "tag_name": "tpm_crb_base_address_table",
        "format": "QI",
        "fields": ["crb_base_address", "crb_size"],
    },
    0x100: {
        "tag_name": "optee_pageable_part",
        "format": "Q",
        "fields": ["pp_addr"],
    },
    0x101: {
        "tag_name": "dt_spmc_manifest",
    },
    0x102: {
        "tag_name": "exec_ep_info",
        "format": "2BHIQI4x8Q",
        "fields": ["ep_info"],
    },
    0x104: {
        "tag_name": "sram_layout",
        "format": "2Q",
        "fields": ["addr", "size"],
    },
}
tag_name_to_tag_id = {
    te["tag_name"]: tag_id for tag_id, te in transfer_entry_formats.items()
}


class TransferList:
    """Class representing a Transfer List based on version 1.0 of the Firmware Handoff specification."""

    # Header encoding, with little-endian byte order.
    encoding = "<I4B4I"
    hdr_size = 0x18
    signature = 0x4A0FB10B
    version = 1

    def __init__(
        self, max_size: int = hdr_size, flags: int = TRANSFER_LIST_ENABLE_CHECKSUM
    ) -> None:
        assert max_size >= self.hdr_size
        self.checksum: int = 0
        self.alignment: int = 3
        self.size = self.hdr_size
        self.total_size = max_size
        self.flags = flags
        self.entries: List[TransferEntry] = []
        self.update_checksum()

    def __str__(self) -> str:
        return "\n".join(
            [
                f"{k:<10} {hex(v)}"
                for k, v in vars(self).items()
                if not isinstance(v, list)
            ]
        )

    def get_transfer_entries_str(self):
        return "\n----\n".join([str(te) for _, te in enumerate(self.entries)])

    @classmethod
    def fromfile(cls, filepath: Path) -> "TransferList":
        tl = cls()

        with open(filepath, "rb") as f:
            (
                tl.signature,
                tl.checksum,
                tl.version,
                tl.hdr_size,
                tl.alignment,
                used_size,
                tl.total_size,
                tl.flags,
                _,
            ) = struct.unpack(
                cls.encoding,
                f.read(tl.hdr_size),
            )

            if tl.signature != TransferList.signature:
                raise ValueError(f"Invalid TL signature 0x{tl.signature:x}!")
            elif tl.version == 0 or tl.version > 0xFF:
                raise ValueError(f"Invalid TL version 0x{tl.version:x}!")
            else:
                while tl.size < used_size:
                    # We add an extra padding byte into the header so we can extract
                    # the 3-byte wide ID as a 4-byte uint, shift out this padding
                    # once we have the id.
                    te_base = f.tell()
                    (id, hdr_size, data_size) = struct.unpack(
                        TransferEntry.encoding[0] + "I" + TransferEntry.encoding[1:],
                        b"\x00" + f.read(TransferEntry.hdr_size),
                    )

                    id >>= 8

                    te = tl.add_transfer_entry(id, f.read(data_size))
                    te.offset = te_base
                    f.seek(align(te_base + hdr_size + data_size, 2**tl.alignment))

        return tl

    @classmethod
    def from_dict(cls, config: Dict[str, Any]) -> "TransferList":
        """Create a TL from data in a dictionary

        The dictionary should have the same format as the yaml config files.
        See the readme for more detail.

        :param config: Dictionary containing the data described above.
        """
        # get settings from config and set defaults
        max_size = config.get("max_size", 0x1000)
        has_checksum = config.get("has_checksum", True)

        flags = TRANSFER_LIST_ENABLE_CHECKSUM if has_checksum else 0

        tl = cls(max_size, flags)

        for entry in config["entries"]:
            tl.add_transfer_entry_from_dict(entry)

        return tl

    def header_to_bytes(self) -> bytes:
        return struct.pack(
            self.encoding,
            self.signature,
            self.checksum,
            self.version,
            self.hdr_size,
            self.alignment,
            self.size,
            self.total_size,
            self.flags,
            0,
        )

    def update_checksum(self) -> None:
        """Calculates the checksum based on the sum of bytes."""
        self.checksum = 256 - ((self.sum_of_bytes() - self.checksum) % 256)

    def sum_of_bytes(self) -> int:
        """Sum of all bytes between the base address and the end of that last TE (modulo 0xff)."""
        return (
            sum(self.header_to_bytes()) + sum(te.sum_of_bytes for te in self.entries)
        ) % 256

    def get_entry(self, tag_id: int) -> Optional[TransferEntry]:
        for te in self.entries:
            if te.id == tag_id:
                return te

        return None

    def get_entry_data_offset(self, tag_id: int) -> int:
        """Returns offset of data of a TE from the base of the TL."""
        te = self.get_entry(tag_id)

        if not te:
            raise ValueError(f"Tag {tag_id} not found in TL!")

        return te.offset + te.hdr_size

    def add_transfer_entry(self, tag_id: int, data: bytes) -> TransferEntry:
        """Appends a TransferEntry into the internal list of TE's."""
        if not (self.total_size >= self.size + TransferEntry.hdr_size + len(data)):
            raise MemoryError(
                f"TL size has exceeded the maximum allocation {self.total_size}."
            )
        else:
            te = TransferEntry(tag_id, len(data), data)
            self.entries.append(te)
            self.size += te.size
            self.update_checksum()
            return te

    def add_transfer_entry_from_struct_format(
        self, tag_id: int, struct_format: str, *args: Any
    ) -> TransferEntry:
        struct_format = "<" + struct_format
        data = struct.pack(struct_format, *args)
        return self.add_transfer_entry(tag_id, data)

    def add_entry_point_info_transfer_entry(
        self, entry: Dict[str, Any]
    ) -> TransferEntry:
        """Add entry_point_info transfer entry

        :param entry: Dictionary of the transfer entry, in the same format as
        the YAML file.
        """
        ep_info = entry["ep_info"]
        header = ep_info["h"]

        # size of the entry_point_info struct
        entry_point_size = 88

        attr = header["attr"]
        if type(attr) is str:
            # convert string of flags names to an integer

            # bit number  | 0                     | 1                    |
            # ------------|-----------------------|----------------------|
            # 0           | secure                | non-secure           |
            # 1           | little endian         | big-endian           |
            # 2           | disable secure timer  | enable secure timer  |
            # 3           | executable            | non-executable       |
            # 4           | first exe             | not first exe        |
            #
            # Bit 5 and bit 0 are used to determine the security state.

            flag_names = {
                "EP_SECURE": 0x0,
                "EP_NON_SECURE": 0x1,
                "EP_REALM": 0x21,
                "EP_EE_LITTLE": 0x0,
                "EP_EE_BIG": 0x2,
                "EP_ST_DISABLE": 0x0,
                "EP_ST_ENABLE": 0x4,
                "EP_NON_EXECUTABLE": 0x0,
                "EP_EXECUTABLE": 0x8,
                "EP_FIRST_EXE": 0x10,
            }

            # create list of integer flags, then bitwise-or them together
            flags = [flag_names[f.strip()] for f in attr.split("|")]
            attr = reduce(lambda x, y: x | y, flags)

        return self.add_transfer_entry_from_struct_format(
            0x102,
            transfer_entry_formats[0x102]["format"],
            header["type"],
            header["version"],
            entry_point_size,
            attr,
            ep_info["pc"],
            ep_info["spsr"],
            *ep_info["args"],
        )

    def add_transfer_entry_from_dict(
        self,
        entry: Dict[str, Any],
    ) -> TransferEntry:
        """Add a transfer entry from data in a dictionary

        The dictionary should have the same format as the entries in the yaml
        config files. See the readme for more detail.

        :param entry: Dictionary containing the data described above.
        """
        # Tag_id is either a tag name or a tag id. Use it to get the TE format.
        tag_id = entry["tag_id"]
        if tag_id in tag_name_to_tag_id:
            tag_id = tag_name_to_tag_id[tag_id]
        te_format = transfer_entry_formats[tag_id]
        tag_name = te_format["tag_name"]

        if "blob_file_path" in entry:
            return self.add_transfer_entry_from_file(tag_id, entry["blob_file_path"])
        elif tag_name == "tpm_event_log_table":
            with open(entry["event_log"], "rb") as f:
                event_log_data = f.read()

            flags_bytes = entry["flags"].to_bytes(4, "little")
            data = flags_bytes + event_log_data

            return self.add_transfer_entry(tag_id, data)
        elif tag_name == "exec_ep_info":
            return self.add_entry_point_info_transfer_entry(entry)
        elif "format" in te_format and "fields" in te_format:
            fields = [entry[field] for field in te_format["fields"]]
            return self.add_transfer_entry_from_struct_format(
                tag_id, te_format["format"], *fields
            )
        else:
            raise ValueError(f"Invalid transfer entry {entry}.")

    def add_transfer_entry_from_file(self, tag_id: int, path: Path) -> TransferEntry:
        with open(path, "rb") as f:
            return self.add_transfer_entry(tag_id, f.read())

    def write_to_file(self, file: Path) -> None:
        """Write the contents of the TL to a file."""
        with open(file, "wb") as f:
            f.write(self.header_to_bytes())
            for te in self.entries:
                assert f.tell() + te.hdr_size + te.data_size < self.total_size
                te_base = f.tell()
                f.write(te.header_to_bytes())
                f.write(te.data)
                # Ensure the next TE has the correct alignment
                f.write(
                    bytes(
                        (
                            align(
                                te_base + te.hdr_size + te.data_size, 2**self.alignment
                            )
                            - f.tell()
                        )
                    )
                )

    def remove_tag(self, tag: int) -> None:
        self.entries = list(filter(lambda te: te.id != tag, self.entries))
        self.size = self.hdr_size + sum(map(lambda te: te.size, self.entries))
        self.update_checksum()


def align(n, alignment):
    return int(math.ceil(n / alignment) * alignment)
