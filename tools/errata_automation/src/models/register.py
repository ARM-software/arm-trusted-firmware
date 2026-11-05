# Copyright (c) 2026, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause

import re
from enum import Enum
from functools import cached_property

from pydantic import BaseModel, ConfigDict, Field, model_validator
from typing_extensions import Self

_BIT_POSITION_PATTERN = re.compile(
    r"^\[?(?P<msb>\d+)(?::(?P<lsb>\d+))?\]?$"
)

_BINARY_PATTERN = re.compile(
    r"(?P<width>\d+)?'?[bB](?P<bits>[01]+)"
)

class RegisterBitPosition(BaseModel):
    msb: int = Field(ge=0, le=63)
    lsb: int = Field(ge=0, le=63)

    @model_validator(mode="after")
    def validate_range(self) -> "RegisterBitPosition":
        if self.lsb > self.msb:
            raise ValueError(
                f"Invalid bit position: LSB ({self.lsb}) cannot be greater than MSB ({self.msb})"
            )

        return self

    @classmethod
    def from_string(cls, value: str) -> Self:
        pattern_match = _BIT_POSITION_PATTERN.fullmatch(value.strip())

        if pattern_match is None:
            raise ValueError(f"Invalid bit position: {value!r}")

        msb = int(pattern_match.group("msb"))
        lsb = int(pattern_match.group("lsb") or msb)

        return cls (msb=msb, lsb=lsb)

    @property
    def bitrange(self) -> str:
        return (
            str(self.lsb)
            if self.msb == self.lsb
            else f"{self.msb}:{self.lsb}"
        )

    @property
    def width(self) -> int:
        return self.msb - self.lsb + 1

class RegisterEncoding(BaseModel):
    model_config = ConfigDict(extra="forbid")

    op0: int = Field(ge=0, le=3)
    op1: int = Field(ge=0, le=7)
    crn: int = Field(ge=0, le=15)
    crm: int = Field(ge=0, le=15)
    op2: int = Field(ge=0, le=7)

    def encoded_format(self) -> str:
        return f"S{self.op0}_{self.op1}_C{self.crn}_C{self.crm}_{self.op2}"

    def __str__(self) -> str:
        return (
            f"Op0={self.op0}, Op1={self.op1}, "
            f"CRn={self.crn}, CRm={self.crm}, Op2={self.op2} "
            f"(S{self.op0}_{self.op1}_C{self.crn}_C{self.crm}_{self.op2})"
        )

class RegisterType(str, Enum):
    Unknown = "Unknown"
    Architectural = "Architectural"
    ImplementationDefined = "ImplementationDefined"

class RegisterSetting(BaseModel):
    model_config = ConfigDict(extra="forbid")

    register_name: str = Field(min_length = 1)
    register_field_name: str = Field(min_length = 1)
    bits: str = Field(min_length = 1)
    value: str = Field(min_length = 1)

    register_encoding: RegisterEncoding | None = None

    register_type: RegisterType = RegisterType.Unknown

    @cached_property
    def relaxed_register_name(self) -> str:
        return "_".join(self.register_name.split("_")[-2:]).upper()

    @property
    def bit_position(self) -> RegisterBitPosition | None:
        try:
            return RegisterBitPosition.from_string(self.bits.strip())
        except ValueError:
            return None

    @property
    def binary_value(self) -> int | None:
        value = self.value.strip()

        if value.isdigit():
            return int(value)

        if regex_match := _BINARY_PATTERN.fullmatch(value):
            bits = regex_match.group("bits")
            return int(bits, 2)

        return None

    @property
    def is_valid(self) -> bool:
        return (
            self.bit_position is not None
            and self.binary_value is not None
            and self.register_encoding is not None
            and self.binary_value < (1 << self.bit_position.width)
        )
