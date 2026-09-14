# Copyright (c) 2026, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause

from functools import cached_property

from pydantic import BaseModel, ConfigDict, Field, model_serializer


class CpuName(BaseModel):
    model_config = ConfigDict(frozen=True)

    name: str = Field(default="")

    def __init__(self, name: str = "") -> None:
        super().__init__(name=name)

    @model_serializer
    def serialize_model(self) -> str:
        return self.name

    def __str__(self) -> str:
        return self.name

    @cached_property
    def flag_name(self) -> str:
        """
        Example: "neoverse_n2" → "N2"
        """
        return (
            self.name
            .replace("cortex_", "")
            .replace("neoverse_", "")
            .replace("_", "")
            .upper()
        )

    @cached_property
    def human_readable_name(self) -> str:
        """
        Example: cortex_a78 → Cortex-A78
                 cortex_a76ae → Cortex-A76AE
        """
        prefix, *parts = self.name.split("_")

        return "-".join([
            prefix.capitalize(),
            *(part.upper() for part in parts),
        ])
