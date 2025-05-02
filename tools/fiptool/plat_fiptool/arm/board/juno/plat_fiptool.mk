#
# Copyright (c) 2023-2025, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

PLAT_DEF_UUID := yes

ifeq (${PLAT_DEF_UUID}, yes)
FIPTOOL_DEFINES += PLAT_DEF_FIP_UUID
ifeq (${ETHOSN_NPU_TZMP1},1)
FIPTOOL_DEFINES += ETHOSN_NPU_TZMP1
endif
FIPTOOL_INCLUDE_DIRS += ./ ../../plat/arm/board/juno/fip ../../include
FIPTOOL_SOURCES += plat_fiptool/arm/board/juno/plat_def_uuid_config.c
endif
