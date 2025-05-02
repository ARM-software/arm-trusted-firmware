#
# Copyright (c) 2021, NXP. All rights reserved.
# Copyright (c) 2022-2025, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

FIPTOOL_INCLUDE_DIRS += ./ \
		 ../../plat/arm/board/tc

FIPTOOL_DEFINES += PLAT_DEF_FIP_UUID
FIPTOOL_SOURCES += plat_fiptool/arm/board/tc/plat_def_uuid_config.c
