#
# Copyright 2018-2019 NXP
#
# SPDX-License-Identifier: BSD-3-Clause
#

# board-specific build parameters
BOOT_MODE	:= nor
BOARD		:= ardb

 # get SoC common build parameters
include plat/nxp/soc-ls1043/soc.mk

BL2_SOURCES	+=	${BOARD_PATH}/ddr_init.c


