#
# Copyright (c) 2021-2024, STMicroelectronics - All Rights Reserved
#
# SPDX-License-Identifier: BSD-3-Clause
#

# Name of the platform defined source file name,
# which contains platform defined UUID entries populated
# in the plat_def_toc_entries[].
PLAT_DEF_UUID_FILE_NAME	:= plat_def_uuid_config

FIPTOOL_INCLUDE_DIRS	+= ../../plat/st/common/include ./

PLAT_DEF_UUID		:= yes

ifeq (${PLAT_DEF_UUID},yes)
FIPTOOL_DEFINES += PLAT_DEF_FIP_UUID

FIPTOOL_SOURCES += plat_fiptool/st/${PLAT_DEF_UUID_FILE_NAME}.c
endif
