# Copyright (c) 2026, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause

from abc import ABC, abstractmethod
from logging import Logger
from pathlib import Path

from src.models.cpuname import CpuName
from src.models.erratum import Erratum


class FileUpdater(ABC):
    path: Path
    logger: Logger

    def __init__(self, path: Path, logger: Logger) -> None:
        self.path = path
        self.logger = logger

    def create_and_add_workaround(self, cpu_name: CpuName, erratum: Erratum) -> bool:
        try:
            return self._update_file(cpu_name, erratum)
        except (ValueError) as e:
            self.logger.warning("%s", e)

        return False

    @abstractmethod
    def _update_file(self, cpu_name: CpuName, erratum: Erratum) -> bool:
        pass
