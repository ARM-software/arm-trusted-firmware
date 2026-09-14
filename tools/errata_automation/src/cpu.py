# Copyright (c) 2026, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause

import logging
import re
from pathlib import Path

from src.document import Document
from src.files.assembly_file import AssemblyFile
from src.files.header_file import HeaderFile
from src.models.cpuname import CpuName
from src.models.erratum import Erratum
from src.models.workaround_details import WorkaroundClass
from src.resolvers.trm import TechnicalReferenceManual

_cpu_marker_re = re.compile(r"cpu_reset_prologue\s+(?P<cpu_name>.+)")
_trm_re = re.compile(r"TRM:\s+(?P<url>.+)")
_sden_re = re.compile(r"SDEN:\s+(?P<url>.+)")
_errata_re = re.compile(
    r"(?P<workaround_class>add_erratum_entry|workaround_reset_start|workaround_runtime_start)\s*.*?,\s*"
    r"(?P<category>ERRATUM)\((?P<id>.*?)\),"
    r"\s*[^,\n]+"
    r"(?:,\s*(?P<review_marker>__REQUIRES_REVIEW__).*)?"
)


class CPU:
    _logger: logging.Logger

    name: CpuName

    assembly_file: AssemblyFile
    header_file: HeaderFile

    missing_errata_from_sden: list[Erratum]
    implemented_errata_from_sden: list[Erratum]

    sden_document: Document
    trm: TechnicalReferenceManual

    _assembly_file_lines: list[str]

    _doc_dir_path: Path
    _assembly_file_path: Path

    _implemented_errata_from_cpu_file: list[dict]

    def __init__(
            self,
            assembly_file_path: str | Path,
            doc_dir_path: str | Path
    ):
        self._logger = logging.getLogger(self.__class__.__name__)

        self._doc_dir_path = Path(doc_dir_path)
        self._assembly_file_path = Path(assembly_file_path)

        self.missing_errata_from_sden = []
        self.implemented_errata_from_sden = []
        self._implemented_errata_from_cpu_file = []

    async def _initialize(self):
        with self._assembly_file_path.open("r", encoding="utf-8", newline="") as file:
            self.assembly_file_lines = file.readlines()

        self.name = self._get_cpu_name(self.assembly_file_lines)

        self.assembly_file = AssemblyFile(self._assembly_file_path)

        header_file_path = Path(
            str(self._assembly_file_path).replace(
                "/lib/",
                "/include/lib/",
                1)
            ).with_suffix(".h")
        self.header_file = HeaderFile(header_file_path)

        self._implemented_errata_from_cpu_file = self._get_errata(
            self.assembly_file_lines
        )

        sden_link, trm_link = self._get_sden_trm_links(self.assembly_file_lines)
        self.sden_document = await Document(sden_link, self._doc_dir_path)
        self.trm =  await TechnicalReferenceManual(trm_link, self._doc_dir_path)

        return self

    def __await__(self):
        return self._initialize().__await__()

    def update_implemented_errata(self, errata: list[Erratum]):
        cpu_implemented_by_id = {
            erratum["id"]: erratum
            for erratum in self._implemented_errata_from_cpu_file
        }

        implemented = []
        seen_ids: set[int] = set()

        for erratum in errata:
            if impld_erratum := cpu_implemented_by_id.get(erratum.id):
                if  erratum.id in seen_ids:
                    raise ValueError(
                        f"Duplicate erratum ID {erratum.id} for CPU {self.name.human_readable_name}"
                    )

                erratum.implemented = True
                erratum.workaround_class = impld_erratum["workaround_class"]
                implemented.append(erratum)
                seen_ids.add(erratum.id)

        self.implemented_errata_from_sden = implemented

    def update_missing_errata(self, errata: list[Erratum]) -> None:
        implemented_ids = {
            implemented["id"]
            for implemented in self._implemented_errata_from_cpu_file
        }

        missing = []
        seen_ids: set[int] = set()

        for erratum in errata:
            if erratum.id not in implemented_ids:
                if  erratum.id in seen_ids:
                    raise ValueError(
                        f"Duplicate erratum ID {erratum.id} for CPU {self.name.human_readable_name}"
                    )

                erratum.implemented = False
                missing.append(erratum)
                seen_ids.add(erratum.id)

        self.missing_errata_from_sden = missing

    def _get_errata(self, asm_source: list[str]):
        implemented_errata: list[dict] = []

        for line in asm_source:
            match = _errata_re.search(line)

            if not match:
                continue

            erratum_id: str = match.group("id")
            workaround_class: str = match.group("workaround_class")
            requires_review = match.group("review_marker") is not None

            if requires_review:
                raise ValueError(
                    f"CPU {self.name.human_readable_name} cannot be loaded, it requires review."
                )

            implemented_errata.append(
                {
                    "id": int(erratum_id),
                    "workaround_class": (
                        WorkaroundClass.CPU_RESET
                        if workaround_class == "workaround_reset_start"
                        else WorkaroundClass.UNCLASSIFIED
                    )
                }
            )

        return implemented_errata

    @staticmethod
    def _get_cpu_name(assembly_file_lines) -> CpuName:
        for line in assembly_file_lines:
            if match := _cpu_marker_re.search(line):
                return CpuName(name=match.group("cpu_name"))

        raise ValueError("CPU name not found in assembly file.")

    def _get_sden_trm_links(self, assembly_file_lines) -> tuple[str, str]:
        sden_link = None
        trm_link = None

        for line in assembly_file_lines:
            if match := _sden_re.search(line):
                sden_link = match.group("url")
            elif match := _trm_re.search(line):
                trm_link = match.group("url")

        if sden_link is None or trm_link is None:
            raise ValueError(
                f"Missing SDEN or TRM link in assembly file {self._assembly_file_path.name}."
            )

        return sden_link, trm_link
