# Copyright (c) 2026, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause

from enum import Enum

from pydantic import BaseModel, ConfigDict, Field, field_serializer, field_validator
from src.models.cpuname import CpuName
from src.models.erratum_revision import Revision
from src.models.workaround_details import WorkaroundClass, WorkaroundDetails


class ErratumStatus(str, Enum):
    Open = "Open"
    Fixed = "Fixed"

    @classmethod
    def _missing_(cls, value):
        if isinstance(value, str):
            return next(
                (member for member in cls if member.value.lower() == value.lower()),
                None
            )
        return None

class Erratum(BaseModel):
    """Validate a CPU erratum and its workaround data from PDFs or LLM output.

    Construction validates field types and rejects unknown fields. An erratum
    is open when its fixed revision is empty; status is derived from that revision.
    ``set_require_review()`` updates the review flag for CPU reset workarounds
    when their register settings or instruction sequence need review; other
    workaround classes leave the flag unchanged.
    """

    model_config = ConfigDict(extra="forbid")
    implemented: bool = False
    requires_review: bool = False
    document_version: str
    cpu_name: CpuName
    id: int
    category: str
    present_revisions: list[str] = Field(default_factory=list)
    fixed_revision: Revision | None
    summary: str
    workaround: str
    workaround_details: WorkaroundDetails = Field(
        default_factory=WorkaroundDetails
    )
    workaround_class: WorkaroundClass = Field(
        default=WorkaroundClass.UNCLASSIFIED
    )

    @field_validator("fixed_revision", mode="before")
    @classmethod
    def empty_fixed_to_none(cls, value: object) -> object:
        if isinstance(value, str):
            return value.strip() or None

        return value

    @field_serializer("fixed_revision", when_used="json")
    def serialize_fixed(self, value: Revision | None) -> str:
        return str(value) if value is not None else ""

    @property
    def status(self) -> ErratumStatus:
        return ErratumStatus.Open if self.fixed_revision is None else ErratumStatus.Fixed

    def is_open(self) -> bool:
        return self.status == ErratumStatus.Open

    def set_require_review(self):
        details = self.workaround_details

        if self.workaround_class == WorkaroundClass.CPU_RESET:
            self.requires_review = bool(
                (not details.chicken_bits and not details.sequence)
                or (details.chicken_bits and details.sequence)
                or any(
                    not chicken_bit.register_encoding
                    or not chicken_bit.is_valid
                    for chicken_bit in details.chicken_bits
                )
            )
