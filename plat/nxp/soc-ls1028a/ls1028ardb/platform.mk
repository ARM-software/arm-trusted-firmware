#
# Copyright 2020-2021 NXP
#
# SPDX-License-Identifier: BSD-3-Clause
#

# Board-specific build parameters
BOOT_MODE	?=	flexspi_nor
BOARD		:=	ls1028ardb
POVDD_ENABLE	:=	no
WARM_BOOT	:=	no

# DDR build parameters
NUM_OF_DDRC		:=	1
CONFIG_DDR_NODIMM	:=	1
DDR_ECC_EN		:=	yes

# On-board flash
FLASH_TYPE	:=	MT35XU02G
XSPI_FLASH_SZ	:=	0x10000000

BL2_SOURCES	+=	${BOARD_PATH}/ddr_init.c \
			${BOARD_PATH}/platform.c

SUPPORTED_BOOT_MODE	:=	flexspi_nor	\
				sd		\
				emmc

# Add platform board build info
include plat/nxp/common/plat_make_helper/plat_common_def.mk

# Add SoC build info
include plat/nxp/soc-ls1028a/soc.mk
