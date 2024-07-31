#!/usr/bin/env python3

#
# Copyright (c) 2024, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

"""Module containing definitions pertaining to the 'Transfer List' (TL) type."""

import typing

import math
import struct
from dataclasses import dataclass
from pathlib import Path

from tlc.te import TransferEntry

TRANSFER_LIST_ENABLE_CHECKSUM = 0b1


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
        self.entries: typing.List["TransferEntry"] = []
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

    def get_entry_data_offset(self, tag_id: int) -> int:
        """Returns offset of data of a TE from the base of the TL."""
        for te in self.entries:
            if te.id == tag_id:
                return te.offset + te.hdr_size

        raise ValueError(f"Tag {tag_id} not found in TL!")

    def add_transfer_entry(self, tag_id: int, data: bytes) -> "TransferEntry":
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

    def add_transfer_entry_from_file(self, tag_id: int, path: Path) -> "TransferEntry":
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
