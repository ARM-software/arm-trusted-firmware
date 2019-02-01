#
# Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#


# Process LS1043_DISABLE_TRUSTED_WDOG flag
# TODO:Temparally disabled it on development phase, not implemented yet
LS1043_DISABLE_TRUSTED_WDOG	:=	1

# On Layerscape platforms, separate the code and read-only data sections to allow
# mapping the former as executable and the latter as execute-never.
SEPARATE_CODE_AND_RODATA	:=	1

COLD_BOOT_SINGLE_CPU		:=	1

PLAT_BL_COMMON_SOURCES	+=	plat/layerscape/common/${ARCH}/ls_helpers.S		\
				plat/layerscape/common/ls_common.c

include lib/xlat_tables_v2/xlat_tables.mk

PLAT_BL_COMMON_SOURCES	+=	${XLAT_TABLES_LIB_SRCS}

BL1_SOURCES		+=			\
				drivers/io/io_fip.c				\
				drivers/io/io_memmap.c				\
				drivers/io/io_storage.c				\
				plat/layerscape/common/ls_timer.c			\
				plat/layerscape/common/ls_bl1_setup.c			\
				plat/layerscape/common/ls_io_storage.c

BL2_SOURCES		+=	drivers/io/io_fip.c				\
				drivers/io/io_memmap.c				\
				drivers/io/io_storage.c				\
				plat/layerscape/common/ls_timer.c			\
				plat/layerscape/common/ls_bl2_setup.c			\
				plat/layerscape/common/ls_io_storage.c
BL2_SOURCES		+=	plat/layerscape/common/${ARCH}/ls_bl2_mem_params_desc.c
BL2_SOURCES		+=	plat/layerscape/common/ls_image_load.c		\
					common/desc_image_load.c

BL31_SOURCES		+=	plat/layerscape/common/ls_bl31_setup.c		\
				plat/layerscape/common/ls_timer.c			\
				plat/layerscape/common/ls_topology.c			\
				plat/layerscape/common/ns_access.c		\
				plat/common/plat_psci_common.c
