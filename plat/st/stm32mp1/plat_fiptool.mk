#
# Copyright (c) 2021-2022, STMicroelectronics - All Rights Reserved
#
# SPDX-License-Identifier: BSD-3-Clause
#

# Name of the platform defined source file name,
# which contains platform defined UUID entries populated
# in the plat_def_toc_entries[].
PLAT_DEF_UUID_FILE_NAME	:= plat_def_uuid_config

INCLUDE_PATHS		+= -I${PLAT_DIR}/include -I./

PLAT_DEF_UUID		:= yes

ifeq (${PLAT_DEF_UUID},yes)
HOSTCCFLAGS += -DPLAT_DEF_FIP_UUID

${PLAT_DEF_UUID_FILE_NAME}.o: ${PLAT_DIR}${PLAT_DEF_UUID_FILE_NAME}.c
	${HOSTCC} -c ${CPPFLAGS} ${HOSTCCFLAGS} ${INCLUDE_PATHS} $< -o $@

PLAT_OBJECTS += ${PLAT_DEF_UUID_FILE_NAME}.o
endif

OBJECTS += ${PLAT_OBJECTS}
