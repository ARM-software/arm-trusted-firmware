# Copyright (c) 2026, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause

from pydantic import BaseModel, ConfigDict
from src.models.workaround_details import WorkaroundClass, WorkaroundDetails


class ErratumClassification(BaseModel):
    model_config = ConfigDict(extra="forbid")

    workaround_class: WorkaroundClass
    workaround_details: WorkaroundDetails

    def __str__(self) -> str:
        return f"{self.workaround_class.value}"
