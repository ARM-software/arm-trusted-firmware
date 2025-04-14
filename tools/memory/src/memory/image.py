#
# Copyright (c) 2023-2025, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

from dataclasses import dataclass
from typing import Optional


@dataclass
class Region:
    """Represents a memory region."""

    start: Optional[int] = None
    """Memory address of the beginning of the region."""

    end: Optional[int] = None
    """Memory address of the end of the region."""

    length: Optional[int] = None
    """Current size of the region in bytes."""

    @property
    def limit(self) -> Optional[int]:
        """Largest possible end memory address of the region."""

        if self.start is None:
            return None

        if self.length is None:
            return None

        return self.start + self.length

    @property
    def size(self) -> Optional[int]:
        """Maximum possible size of the region in bytes."""

        if self.end is None:
            return None

        if self.start is None:
            return None

        return self.end - self.start

    @property
    def free(self) -> Optional[int]:
        """Number of bytes that the region is permitted to further expand."""

        if self.limit is None:
            return None

        if self.end is None:
            return None

        return self.limit - self.end
