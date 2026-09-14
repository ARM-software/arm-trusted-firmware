# Copyright (c) 2026, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause

import re
from functools import total_ordering
from typing import Any

from pydantic import BaseModel, ConfigDict, Field, field_validator, model_validator


@total_ordering
class Revision(BaseModel):
    model_config = ConfigDict(frozen=True)

    revision: int = Field(ge=0)
    patch: int = Field(ge=0)

    @model_validator(mode="before")
    @classmethod
    def from_string(cls, value: Any) -> Any:
        if isinstance(value, str):
            return cls._parse_string(value)

        return value

    @staticmethod
    def _parse_string(value: str) -> dict[str, int]:
        match = re.fullmatch(r"r(\d+)p(\d+)", value, re.IGNORECASE)

        if match is None:
            raise ValueError(f"Invalid revision: {value!r}")

        return {
            "revision": int(match.group(1)),
            "patch": int(match.group(2)),
        }

    def __str__(self) -> str:
        return f"r{self.revision}p{self.patch}"

    def __eq__(self, other: object) -> bool:
        if not isinstance(other, Revision):
            return NotImplemented

        return (self.revision, self.patch) == (other.revision, other.patch)

    def __lt__(self, other: "Revision") -> bool:
        if not isinstance(other, Revision):
            return NotImplemented

        return (self.revision, self.patch) < (other.revision, other.patch)

    def is_next(self, other: "Revision") -> bool:
        return (
            other.revision == self.revision
            and other.patch == self.patch + 1
        ) or (
            other.revision == self.revision + 1
            and other.patch == 0
        )


class RevisionRange(BaseModel):
    model_config = ConfigDict(frozen=True)

    revisions: list[Revision] = Field(min_length=1)

    @field_validator("revisions")
    @classmethod
    def sort_revisions(cls, revisions: list[Revision]) -> list[Revision]:
        return sorted(revisions)

    @property
    def start_revision(self) -> Revision:
        return self.revisions[0]

    @property
    def end_revision(self) -> Revision:
        return self.revisions[-1]

    def empty(self) -> bool:
        return len(self.revisions) == 0

    def is_continuous(self) -> bool:
        return bool(self.revisions) and all(
            current.is_next(next_revision)
            for current, next_revision in zip(
                self.revisions,
                self.revisions[1:],
            )
        )

    def is_continuous_range_from_release_revision(self) -> bool:
        return (
            self.start_revision == Revision(revision=0, patch=0)
            and self.is_continuous()
        )
