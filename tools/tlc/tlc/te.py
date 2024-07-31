#!/usr/bin/env python3

#
# Copyright (c) 2024, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

"""Module containing definitions pertaining to the 'Transfer Entry' (TE) type."""

from typing import ClassVar

import struct
from dataclasses import dataclass


@dataclass
class TransferEntry:
    """Class representing a Transfer Entry."""

    id: int
    data_size: int
    data: bytes
    hdr_size: int = 8
    offset: int = 0
    # Header encoding, with little-endian byte order.
    encoding: ClassVar[str] = "<BI"

    def __post_init__(self):
        if self.id < 0 or self.id > 0xFFFFFF:
            raise ValueError(
                f"Out of bounds tag ID: {self.id:x}.\n"
                f"Valid range is from 0 to 0xFFFFFF. Please ensure the tag ID is within this range."
            )

    def __str__(self) -> str:
        return "\n".join(
            [
                f"{k:<10} {hex(v)}"
                for k, v in vars(self).items()
                if not isinstance(v, bytes)
            ]
        )

    @property
    def size(self) -> int:
        return self.hdr_size + len(self.data)

    @property
    def sum_of_bytes(self) -> int:
        return (sum(self.header_to_bytes()) + sum(self.data)) % 256

    def header_to_bytes(self) -> bytes:
        return self.id.to_bytes(3, "little") + struct.pack(
            self.encoding, self.hdr_size, self.data_size
        )
