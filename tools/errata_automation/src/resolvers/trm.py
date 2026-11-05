# Copyright (c) 2026, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause

import asyncio
import logging
import re
import shutil
from pathlib import Path

import aiofiles
from src.document import Document
from src.models.register import RegisterBitPosition, RegisterEncoding


class TechnicalReferenceManual:
    _logger: logging.Logger
    _doc_dir_path: Path

    _trm_document: Document

    _pdf_text: str
    _url: str

    def __init__(self, url: str, doc_dir_path: Path) -> None:
        self._logger = logging.getLogger(self.__class__.__name__)

        self._doc_dir_path = doc_dir_path
        self._url = url

    async def _initialize(self):
        trm_document = await Document(self._url, self._doc_dir_path)

        await trm_document.fetch()
        self._pdf_text = await self._extract_text(trm_document.path)

        return self

    def __await__(self):
        return self._initialize().__await__()

    def register_field_to_bit_position(self, register_name: str, register_field: str) -> RegisterBitPosition:
        pattern = (
            rf"(?ms)^[ \t]*Table\s+\S+:?\s*"
            rf"{re.escape(register_name)}"
            rf"(?:\[\s*\d+(?:\s*:\s*\d+)?\s*\])?"
            rf"\s+bit\s+(?:descriptions|assignments)"
            rf"(?:\s+\(continued\))?\s*$"
            rf"(?:(?!^[ \t]*Table\s).)*?"
            rf"^[ \t]*\[\s*(?P<msb>\d+)"
            rf"(?:\s*:\s*(?P<lsb>\d+))?"
            rf"\s*\][a-z]*[ \t]+"
            rf"{re.escape(register_field)}\b"
        )

        match = re.search(
            pattern,
            self._pdf_text,
            re.IGNORECASE | re.DOTALL
        )

        if match is None:
            raise LookupError(
                f"{self.__class__.__name__}: "
                f"register entry {register_name!r}.{register_field!r} not found!"
            )

        msb = match.group("msb")
        lsb = match.group("lsb")

        if lsb is None:
            lsb = msb

        return RegisterBitPosition(
            lsb=int(lsb),
            msb=int(msb)
        )

    def bit_postion_to_register_field(self, register_name: str, bit_position: RegisterBitPosition):
        bit_range = (
            rf"\[\s*{bit_position.msb}\s*\]"
            if bit_position.width == 1
            else rf"\[\s*{bit_position.msb}\s*:\s*{bit_position.lsb}\s*\]"
        )

        patterns = [
            (
                rf"(?ms)^Table\s+\S+:\s*"
                rf"{re.escape(register_name)}"
                rf"\s+bit\s+descriptions\s*$"
                rf"(?:(?!^Table\s).)*?"
                rf"^{bit_range}\s+"
                rf"(?P<register_field>\w+)\b"
            ),
            (
                rf"(?ms)^[ \t]*Table\s+\S+\s+"
                rf"{re.escape(register_name)}"
                rf"\s+bit\s+assignments"
                rf"(?:\s+\(continued\))?\s*$"
                rf"(?:(?!^[ \t]*Table\s).)*?"
                rf"^[ \t]*{bit_range}\s+"
                rf"(?P<register_field>[A-Za-z0-9_]+)\b"
            ),
        ]

        self._logger.info(
            "Resolving field name %s[%s] to bit position",
            register_name,
            bit_position.bitrange
        )

        match = next(
            (
                match
                for pattern in patterns
                if (
                    match := re.search(
                        pattern,
                        self._pdf_text,
                        re.IGNORECASE,
                    )
                )
            ),
            None,
        )

        if match is None:
            raise LookupError(
                f"Field name for "
                f"{register_name}[{bit_position.bitrange}] not found"
            )

        field_name = match.group("register_field")

        self._logger.info(
            "Field name %s[%s] resolved to %s",
            register_name,
            bit_position.bitrange,
            field_name
        )

        return field_name

    def get_register_encoding(self, register_name: str) -> RegisterEncoding:
        #TODO: maybe debug level
        self._logger.info(
            "Getting encodig for resgister: %s",
            register_name
        )

        register_encoding = self._lookup_register_mapping_from_trm(self._pdf_text, register_name)

        #TODO: maybe debug level
        self._logger.info(
            "Resgister %s encoded to %s",
            register_name,
            register_encoding
        )

        return register_encoding

    async def _extract_text(self, source: Path) -> str:
        output_path = source.with_suffix(".txt")

        self._logger.info("Extracting %s to text", source.name)

        if output_path.is_file():
            self._logger.info("Extracted documenet alerady exists. Using existing text file: %s", output_path.name)
            return output_path.read_text(encoding="utf-8", errors="replace")

        if source.suffix.lower() != ".pdf":
            return source.read_text(encoding="utf-8", errors="replace")

        pdftotext = shutil.which("pdftotext")
        if not pdftotext:
            raise RuntimeError(
                f"{self.__class__.__name__}: "
                "pdftotext is required. Please install poppler-utils"
            )

        process = await asyncio.create_subprocess_exec(
            pdftotext,
            "-layout",
            str(source),
            str(output_path),
            stdout=asyncio.subprocess.PIPE,
            stderr=asyncio.subprocess.PIPE,
        )

        _, stderr = await process.communicate()

        if process.returncode != 0:
            raise RuntimeError(
                stderr.decode(errors="replace").strip()
                or "pdftotext failed"
            )

        async with aiofiles.open(
            output_path,
            "r",
            encoding="utf-8",
            errors="replace",
        ) as file:
            return await file.read()

    def _lookup_register_mapping_from_trm(self, trm_text, register_name) -> RegisterEncoding:
        """Resolve one system register encoding from TRM text."""
        name = re.escape(register_name.strip().upper())

        name_pattern = f"(?:IMP_{name}|{name})"

        patterns = [
            (#Format: Name Op0 Op1 CRn CRm Op2
                rf"\b{name_pattern}\b\s+"
                rf"(?P<Op0>[0-3])\s+"
                rf"(?P<Op1>[0-7])\s+"
                rf"C(?P<CRn>\d+)\s+"
                rf"C(?P<CRm>\d+)\s+"
                rf"(?P<Op2>[0-7])\b"
            ),
            (#Format: Name Op0 CRn Op1 CRm Op2
                rf"\b{name_pattern}\b\s+"
                rf"(?P<Op0>[0-3])\s+"
                rf"C(?P<CRn>\d+)\s+"
                rf"(?P<Op1>[0-7])\s+"
                rf"C(?P<CRm>\d+)\s+"
                rf"(?P<Op2>[0-7])\b"
            ),
            (#Format: Op0 Op1 CRn CRm Op2 Name
                rf"(?P<Op0>[0-3])\s+"
                rf"(?P<Op1>[0-7])\s+"
                rf"C(?P<CRn>\d+)\s+"
                rf"C(?P<CRm>\d+)\s+"
                rf"(?P<Op2>[0-7])\s+"
                rf"{name_pattern}(?![A-Za-z0-9_])"
            ),
            (#Format: Name S3_0_C15_C1_0
                rf"\b{re.escape(register_name.strip().upper())}\b.{{0,200}}?\b"
                rf"S(?P<Op0>\d+)_(?P<Op1>\d+)_C(?P<CRn>\d+)_C(?P<CRm>\d+)_(?P<Op2>\d+)\b"
            )
        ]

        match = None

        for pattern in patterns:
            match = re.search(
                pattern,
                trm_text,
                re.IGNORECASE | re.DOTALL
            )

            if match:
                return RegisterEncoding(
                    op0 = int(match.group("Op0")),
                    op1 = int(match.group("Op1")),
                    op2 = int(match.group("Op2")),
                    crn = int(match.group("CRn")),
                    crm = int(match.group("CRm")),
                )

        raise LookupError(f"{self.__class__.__name__}: Register {register_name!r} not found!")
