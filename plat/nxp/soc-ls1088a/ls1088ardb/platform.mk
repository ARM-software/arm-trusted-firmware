#
# Copyright 2022 NXP
#
# SPDX-License-Identifier: BSD-3-Clause
#

# board-specific build parameters
BOOT_MODE	?= 	qspi
BOARD		:=	ls1088ardb

# DDR Compilation Configs
NUM_OF_DDRC	:=	1
DDRC_NUM_DIMM	:=      1
DDR_ECC_EN	:=      yes

# On-Board Flash Details
QSPI_FLASH_SZ	:=      0x4000000

# Adding Platform files build files
BL2_SOURCES	+=	${BOARD_PATH}/ddr_init.c \
			${BOARD_PATH}/platform.c

SUPPORTED_BOOT_MODE	:=	qspi	\
				sd

# Adding platform board build info
include plat/nxp/common/plat_make_helper/plat_common_def.mk

# Adding SoC build info
include plat/nxp/soc-ls1088a/soc.mk
