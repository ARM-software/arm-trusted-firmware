# Copyright (c) 2026, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause

import logging
import re
from enum import Enum, auto
from functools import cached_property, total_ordering
from pathlib import Path
from typing import Any
from urllib.parse import urlparse

import aiofiles
import httpx
from pydantic import BaseModel, ConfigDict, field_validator

logger = logging.getLogger(__name__)

class DocumentType(Enum):
    SDEN = auto()
    TRM = auto()

_VERSION_PATTERN = re.compile(
    r"^(?:[vV]?(?P<decimal>\d+(?:\.\d+)?)|"
    r"[rR](?P<revision>\d+)[pP](?P<patch>\d+))$"
)


@total_ordering
class DocumentVersion(BaseModel):
    """Represent an immutable document version with numeric comparisons.

    This class is needed because version strings must be compared numerically
    rather than lexicographically. For example, ``"9.0" < "10.0"`` is false
    when compared as strings, but should be true for document versions.

    Decimal versions may have a ``v`` prefix, while revision-patch versions use
    the ``r<number>p<number>`` form. Equality, ordering, and hashing use normalized
    numeric components, so prefixes and trailing zero components do not affect
    comparisons. Instances can also be compared with supported version strings.
    """

    model_config = ConfigDict(extra="forbid", frozen=True)

    version: str

    @field_validator("version")
    @classmethod
    def validate_document_version(cls, value: str) -> str:
        value = value.strip()
        if not _VERSION_PATTERN.fullmatch(value):
            raise ValueError(f"unsupported version format: {value!r}")
        return value

    def __eq__(self, other: object) -> bool:
        other_version = self._coerce(other)
        if other_version is None:
            return NotImplemented

        return self._numeric_version == other_version._numeric_version

    def __lt__(self, other: object) -> bool:
        other_version = self._coerce(other)
        if other_version is None:
            return NotImplemented

        return self._numeric_version < other_version._numeric_version

    def __hash__(self) -> int:
        return hash(self._numeric_version)

    @cached_property
    def _numeric_version(self) -> tuple[int, ...]:
        match = _VERSION_PATTERN.fullmatch(self.version)

        if match is None:  # Guaranteed by the field validator.
            raise ValueError(f"unsupported version format: {self.version!r}")

        if decimal_version := match.group("decimal"):
            parts = tuple(map(int, decimal_version.split(".")))
        else:
            parts = (
                int(match.group("revision")),
                int(match.group("patch")),
            )

        while len(parts) > 1 and parts[-1] == 0:
            parts = parts[:-1]

        return parts

    def _coerce(self, other: object) -> "DocumentVersion | None":
        """Convert a supported comparison operand to DocumentVersion.

        Example: DocumentVersion("24.0") < "25.0"
        """

        if isinstance(other, DocumentVersion):
            return other

        if isinstance(other, str):
            return DocumentVersion(version=other)

        return None


class Document:
    _doc_base = "https://documentation-service.arm.com/documentation/"
    _request_timeout = 30
    _chunk_size = 1024 * 1024

    _id: str
    _download_url: str
    path: Path
    _doc_dir_path: Path
    version: DocumentVersion

    def __init__(self, url: str, doc_dir_path: Path):
        self._doc_dir_path = doc_dir_path
        self._id = Document._extract_doc_id(url)

    async def _initialize(self):
        meta_data = await self._load_metadata(self._id)
        self._download_url, filename = Document._find_pdf_resource(meta_data, self._id)

        self.version = DocumentVersion(
            version=meta_data.get("versionLabel", "n/a")
        )

        self.path = self._doc_dir_path / filename

        return self

    def __await__(self):
        return self._initialize().__await__()

    @staticmethod
    def _extract_doc_id(url: str) -> str:
        parsed = urlparse(url.strip())
        path_parts = [part for part in parsed.path.split("/") if part]

        if (
            parsed.scheme not in ("http", "https")
            or parsed.netloc != "support.arm.com"
            or len(path_parts) < 3
            or path_parts[0] != "documentation"
            or path_parts[-1] != "latest"
        ):
            raise ValueError(f"Unsupported Arm documentation URL: {url}")

        return "/".join(path_parts[1:-1])

    @classmethod
    async def _load_metadata(cls, doc_id: str) -> dict[str, Any]:
        meta_url = f"{cls._doc_base}{doc_id}"

        logger.info(
            "Loading metadata for %s from %s",
            doc_id,
            meta_url,
        )

        async with httpx.AsyncClient() as client:
            response = await client.get(
                meta_url,
                timeout=cls._request_timeout,
            )

        response.raise_for_status()
        return response.json()

    @staticmethod
    def _find_pdf_resource(data: dict[str, Any], doc_id: str) -> tuple[str, str]:
        resources = data.get("_links", {}).get("resources", [])

        for resource in resources:
            if (resource.get("extension") or "").lower() != "pdf":
                continue

            href = resource.get("href")
            name = resource.get("name")
            if href and name:
                return href, Path(name).name

        raise LookupError(f"No PDF resource found for document: {doc_id}")

    async def fetch(self):
        if not self._download_url:
            logger.warning(
                "Document %s has no download URL",
                self._id,
            )
            return False

        if self.path.is_file():
            logger.info(
                "Document(%s) %s already exists.",
                self._id,
                self.path.name,
            )
            return True

        logger.info("GET: %s", self._download_url)

        self.path.parent.mkdir(parents=True, exist_ok=True)

        tmp_path = self.path.with_suffix(f"{self.path.suffix}.part")

        async with (
            httpx.AsyncClient() as client,
            client.stream(
                "GET",
                self._download_url,
                timeout=self._request_timeout,
            ) as response
        ):
            response.raise_for_status()

            async with aiofiles.open(tmp_path, "wb") as file:
                async for chunk in response.aiter_bytes(
                    chunk_size=self._chunk_size,
                ):
                    await file.write(chunk)

            tmp_path.replace(self.path)
