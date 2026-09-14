# Copyright (c) 2026, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause

from enum import Enum

from pydantic import BaseModel, ConfigDict, Field
from src.models.register import RegisterSetting


class WorkaroundClass(str, Enum):
    CPU_RESET = "CPU_RESET"
    POWERDOWN = "POWERDOWN"
    RUNTIME_CALLSITE = "RUNTIME_CALLSITE"
    IGNORED = "IGNORED"
    UNCLASSIFIED = "UNCLASSIFIED"

class SequenceStep(BaseModel):
    model_config = ConfigDict(extra="forbid")

    order: int = Field(ge=1)
    instruction: str = ""

class WorkaroundDetails(BaseModel):
    model_config = ConfigDict(extra="forbid")

    chicken_bits: list[RegisterSetting] = Field(default_factory=list)
    sequence: list[SequenceStep] = Field(default_factory=list)
