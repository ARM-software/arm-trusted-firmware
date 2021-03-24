#
# Copyright (c) 2021, NXP. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

# Name of the platform defined source file name,
# which contains platform defined UUID entries populated
# in the plat_def_toc_entries[].
PLAT_DEF_UUID_CONFIG_FILE_NAME	:= plat_def_uuid_config

PLAT_DEF_UUID_CONFIG_FILE_PATH := ../nxp/plat_fiptool

PLAT_DEF_OID := yes
PLAT_DEF_UUID := yes
PLAT_DEF_UUID_OID_CONFIG_PATH := ../../plat/nxp/common/fip_handler/common


INCLUDE_PATHS += -I${PLAT_DEF_UUID_OID_CONFIG_PATH} \
		 -I./
# Clean the stale object file.
$(shell rm ${PLAT_DEF_UUID_CONFIG_FILE_PATH}/${PLAT_DEF_UUID_CONFIG_FILE_NAME}.o)

ifeq (${PLAT_DEF_OID},yes)
HOSTCCFLAGS += -DPLAT_DEF_OID
endif

ifeq (${PLAT_DEF_UUID},yes)
HOSTCCFLAGS += -DPLAT_DEF_FIP_UUID
PLAT_OBJECTS += ${PLAT_DEF_UUID_CONFIG_FILE_PATH}/${PLAT_DEF_UUID_CONFIG_FILE_NAME}.o
endif

OBJECTS += ${PLAT_OBJECTS}
