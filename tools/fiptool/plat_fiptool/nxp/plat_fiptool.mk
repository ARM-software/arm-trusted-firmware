#
# Copyright (c) 2021, 2025 NXP. All rights reserved.
# Copyright (c) 2023-2025, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

# Name of the platform defined source file name,
# which contains platform defined UUID entries populated
# in the plat_def_toc_entries[].
ifeq (,$(findstring s32,${PLAT}))
PLAT_DEF_UUID_CONFIG_FILE_NAME	:= plat_def_uuid_config

PLAT_DEF_UUID_CONFIG_FILE_PATH := plat_fiptool/nxp/

PLAT_DEF_OID := yes
PLAT_DEF_UUID := yes
PLAT_DEF_UUID_OID_CONFIG_PATH := ../../plat/nxp/common/fip_handler/common


FIPTOOL_INCLUDE_DIRS += ${PLAT_DEF_UUID_OID_CONFIG_PATH} \
			./

ifeq (${PLAT_DEF_OID},yes)
FIPTOOL_DEFINES += PLAT_DEF_OID
endif

ifeq (${PLAT_DEF_UUID},yes)
FIPTOOL_DEFINES += PLAT_DEF_FIP_UUID
FIPTOOL_SOURCES += ${PLAT_DEF_UUID_CONFIG_FILE_PATH}/${PLAT_DEF_UUID_CONFIG_FILE_NAME}.c
endif

endif
