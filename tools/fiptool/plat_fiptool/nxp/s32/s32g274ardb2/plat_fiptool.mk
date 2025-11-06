#
# Copyright 2025 NXP. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

# Name of the platform defined source file name,
# which contains platform defined UUID entries populated
# in the plat_def_toc_entries[].
PLAT_DEF_UUID_CONFIG_FILE_NAME	:= plat_def_uuid_config

PLAT_DEF_UUID_CONFIG_FILE_PATH := plat_fiptool/nxp/s32/s32g274ardb2

PLAT_DEF_UUID := yes
PLAT_DEF_UUID_OID_CONFIG_PATH := ../../plat/nxp/s32/s32g274ardb2


FIPTOOL_INCLUDE_DIRS += ${PLAT_DEF_UUID_OID_CONFIG_PATH}/include ./

ifeq (${PLAT_DEF_UUID},yes)
FIPTOOL_CFLAGS += -DPLAT_DEF_FIP_UUID

FIPTOOL_SOURCES += ${PLAT_DEF_UUID_CONFIG_FILE_PATH}/${PLAT_DEF_UUID_CONFIG_FILE_NAME}.c
endif
