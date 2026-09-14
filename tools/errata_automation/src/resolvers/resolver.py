# Copyright (c) 2026, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause

import logging
import re
from pathlib import Path

from src.cpu import CPU
from src.models.erratum import Erratum
from src.models.register import RegisterType
from src.resolvers.architecture_registers import ArchitectureRegisters

_BIT_POSITION_PATTERN = re.compile(r"^\[?\d+(?::\d+)?\]?$")

class RegisterResolver:
    _archregs: ArchitectureRegisters

    logger: logging.Logger

    def __init__(self, doc_dir_path: Path) -> None:
        self.logger = logging.getLogger(self.__class__.__name__)

        self._archregs = ArchitectureRegisters(doc_dir_path)

    def resolve_registers(self, cpu: CPU, erratum: Erratum):
        self._resovle_register_fields_to_bit_poisitions(cpu, erratum)

        self._resovle_bit_poisitions_to_register_fields(cpu, erratum)

        self._set_register_encodings(cpu, erratum)

    def _set_register_encodings(self, cpu: CPU, erratum: Erratum):
        for chicken_bit in erratum.workaround_details.chicken_bits:
            try:
                chicken_bit.register_encoding = self._archregs.get_register_encoding(chicken_bit.register_name)
                chicken_bit.register_type = self._archregs.get_register_type(chicken_bit.register_name)
            except LookupError as aarch_error:
                try:
                    chicken_bit.register_encoding = cpu.trm.get_register_encoding(chicken_bit.register_name)
                    chicken_bit.register_type = RegisterType.ImplementationDefined
                except (LookupError, RuntimeError) as trm_error:
                    self.logger.warning(
                        "Failed to encode register:\n"
                        "\tRegister: %s\n"
                        "\t%s\n"
                        "\t%s",
                        chicken_bit.register_name,
                        aarch_error,
                        trm_error
                    )

    def _resovle_register_fields_to_bit_poisitions(self, cpu: CPU, erratum: Erratum):
        for chicken_bit in erratum.workaround_details.chicken_bits:
            if not _BIT_POSITION_PATTERN.fullmatch(chicken_bit.bits):
                try:
                    bit_position = self._archregs.register_field_to_bit_position(
                        chicken_bit.register_name,
                        chicken_bit.register_field_name,
                    )

                    chicken_bit.bits = bit_position.bitrange
                except (LookupError, RuntimeError) as aarch_error:
                    try:
                        bit_position = cpu.trm.register_field_to_bit_position(
                            chicken_bit.register_name,
                            chicken_bit.register_field_name
                        )

                        chicken_bit.bits = bit_position.bitrange
                    except (LookupError, RuntimeError) as trm_error:
                        self.logger.warning(
                            "Failed to resolve register field.\n"
                            "\tRegister: %s\n"
                            "\tField: %s\n"
                            "\t%s\n"
                            "\t%s",
                            chicken_bit.register_name,
                            chicken_bit.register_field_name,
                            aarch_error,
                            trm_error,
                        )

    def _resovle_bit_poisitions_to_register_fields(self, cpu: CPU, erratum: Erratum):
        for chicken_bit in erratum.workaround_details.chicken_bits:
            bit_position = chicken_bit.bit_position

            if bit_position is not None and bit_position.width > 1:
                try:
                    chicken_bit.register_field_name  = self._archregs.bit_postion_to_register_field(
                        chicken_bit.register_name,
                        bit_position
                    )

                except (LookupError, RuntimeError, ValueError) as aarch_error:
                    try:
                        chicken_bit.register_field_name = cpu.trm.bit_postion_to_register_field(
                            chicken_bit.register_name,
                            bit_position
                        )

                    except (LookupError, RuntimeError, ValueError) as trm_error:
                        self.logger.warning(
                            "Failed to resolve bit position.\n"
                            "\tRegister: %s\n"
                            "\tField: %s\n"
                            "\t%s\n"
                            "\t%s",
                            chicken_bit.register_name,
                            bit_position.bitrange,
                            aarch_error,
                            trm_error,
                        )
