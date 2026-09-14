# Copyright (c) 2026, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause

import logging
import shutil
import tarfile
import urllib.request
import xml.etree.ElementTree as ET
from functools import cached_property
from pathlib import Path
from urllib.parse import urlparse

from src.models.register import RegisterBitPosition, RegisterEncoding, RegisterType
from src.resolvers.architecture_archive_extractor import ArchitectureArchiveExtractor

_DOWNLOAD_PAGE = "https://support.arm.com/architectures/a-profile-architecture#Downloads"


class RegisterXml:
    _xml: ET.ElementTree
    _root: ET.Element

    def __init__(self, xml: ET.ElementTree) -> None:
        self._xml = xml

        root = self._xml.getroot()
        if root is None:
            raise ValueError("XML document has no root element")

        self._root = root

    @cached_property
    def execution_state(self) -> str | None:
        register = self._root.find(".//register")
        return register.get("execution_state") if register is not None else None

    @cached_property
    def mapped_file_name(self) -> str | None:
        mapped = self._root.find(".//mapped_name")
        return mapped.get("filename") if mapped is not None else None

    @cached_property
    def mapped_register_name(self) -> str | None:
        return self._root.findtext(".//mapped_name")

    @cached_property
    def register_short_name(self):
        return self._root.findtext(".//reg_short_name")

    @cached_property
    def reg_long_name(self):
        return self._root.findtext(".//reg_long_name")


    @cached_property
    def register_encoding(self):
        accessor = self._root.find(f".//access_mechanism[@accessor='MRS {self.register_short_name}']/encoding")

        if accessor is None:
            raise LookupError(f"{self.__class__.__name__}: Encodings not found for {self.register_short_name!r}")

        encodings = {
            enc.get("n"): int(enc.get("v"), 2)
            for enc in accessor.findall("enc")
        }

        if not encodings:
            raise LookupError(f"{self.__class__.__name__}: Encodings not found for {self.register_short_name!r}")

        register_encoding = RegisterEncoding(
            op0=encodings["op0"],
            op1=encodings["op1"],
            op2=encodings["op2"],
            crm=encodings["CRm"],
            crn=encodings["CRn"]
        )

        return register_encoding

    @cached_property
    def register_type(self) -> RegisterType:
        return (
            RegisterType.ImplementationDefined
            if self._root.findtext(".//reg_group") == "IMP DEF"
            else RegisterType.Architectural
        )

    def register_field_to_bit_position(self, register_field: str) -> RegisterBitPosition:
        for field in self._root.findall(".//reg_fieldsets/fields/field"):
            if field.findtext("field_name") == register_field:
                position =  RegisterBitPosition(
                    lsb=int(field.findtext("field_lsb")),
                    msb=int(field.findtext("field_msb"))
                )

                return position

        raise LookupError(f"{self.__class__.__name__}: Register field {register_field!r} not found")

    def bit_postion_to_register_field(self, bit_position: RegisterBitPosition) -> str:
        field_name = next(
            (
                field.findtext("field_name")
                for field in self._root.findall(".//field")
                if (
                    field.findtext("field_msb") == str(bit_position.msb)
                    and field.findtext("field_lsb") == str(bit_position.lsb)
                    and field.findtext("field_name") is not None
                )
            ),
            None,
        )

        if field_name is None:
            raise LookupError(
                f"Field name for "
                f"{self.register_short_name}[{bit_position.bitrange}] not found"
            )

        return field_name

class ArchitectureRegisters:
    _cache: dict[str, RegisterXml]
    _logger: logging.Logger

    _sysregxml_dir: Path

    def __init__(self, download_dir: Path) -> None:
        self._cache = {}
        self._logger = logging.getLogger(self.__class__.__name__)

        self._sysregxml_dir = download_dir / "SysRegXml"

        extractor = ArchitectureArchiveExtractor(_DOWNLOAD_PAGE)
        latest_release = extractor.get_latest_release()

        self._download_and_extract(latest_release.url)

    def get_register_encoding(self, register_name: str) -> RegisterEncoding:
        self._logger.info(
            "Getting register encodig for %s",
            register_name
        )

        xml = self._get_register_xml_by_name(register_name)

        register_encoding = xml.register_encoding

        self._logger.info(
            "Resgister %s encoded to %s",
            register_name,
            register_encoding
        )

        return register_encoding

    def get_register_type(self, register_name: str) -> RegisterType:
        xml = self._get_register_xml_by_name(register_name)
        return xml.register_type

    def register_field_to_bit_position(self, register_name: str, register_field: str) -> RegisterBitPosition:
        self._logger.info(
            "Resolving bit position for %s.%s",
            register_name,
            register_field
        )

        xml = self._get_register_xml_by_name(register_name)

        position = xml.register_field_to_bit_position(register_field)

        self._logger.info(
            "Bit position resolved to %s[%s]",
            register_name,
            position.bitrange,
        )

        return position

    def bit_postion_to_register_field(self, register_name: str, bit_position: RegisterBitPosition):
        self._logger.info(
            "Resolving field name %s[%s] to bit position",
            register_name,
            bit_position.bitrange
        )

        xml = self._get_register_xml_by_name(register_name)

        field_name = xml.bit_postion_to_register_field(bit_position)

        self._logger.info(
            "Field name %s[%s] resolved to %s",
            register_name,
            bit_position.bitrange,
            field_name
        )

        return field_name

    def _get_register_xml_by_name(self, register_name: str):
        if register_name in self._cache:
            return self._cache[register_name]

        reg_name = register_name.lower()

        matches = [
            path
            for prefix in ("AArch64", "AArch32")
            for path in self._sysregxml_dir.glob(f"{prefix}-{reg_name}.xml")
        ]

        if not matches:
            raise LookupError(
                f"{self.__class__.__name__}: XML file not found for register {register_name!r}!"
            )
        elif len(matches) > 1:
            raise LookupError(
                f"{self.__class__.__name__}: Multiple XML file found for register {register_name!r}!"
            )

        tree = ET.parse(matches[0])

        xml = RegisterXml(tree)

        if xml.execution_state == "AArch32":
            mapped_xml = self._get_register_xml_by_name(xml.mapped_register_name)

            self._logger.info(
                "AArch32 %s resgister is mapped to AArch64 %s resgister",
                xml.register_short_name,
                mapped_xml.register_short_name
            )

            xml = mapped_xml

        self._cache[register_name] = xml

        if xml.mapped_register_name:
            self._cache[xml.mapped_register_name] = xml

        return xml

    def _download_and_extract(self, url: str):
        archive = self._sysregxml_dir / Path(urlparse(url).path).name

        if archive.exists():
            self._logger.info(
            "SysRegXmls already downloaded, %s already exists.",
                archive.name
            )
            return

        self._sysregxml_dir.mkdir(parents=True, exist_ok=True)

        self._logger.info(
            "Downloading SysRegXml archive: %s",
            archive.name
        )

        with urllib.request.urlopen(url) as response, archive.open("wb") as file:
            while chunk := response.read(1024 * 1024):
                file.write(chunk)

        self._logger.info(
            "Extractring XMLs from %s",
            archive.name
        )

        with tarfile.open(archive, "r:gz") as tar:
            for member in tar:
                if member.isfile() and member.name.lower().endswith(".xml"):
                    src = tar.extractfile(member)

                    if src is not None:
                        with (self._sysregxml_dir / Path(member.name).name).open("wb") as dst:
                            shutil.copyfileobj(src, dst)
