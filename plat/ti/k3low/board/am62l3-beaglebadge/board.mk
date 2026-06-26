#
# Copyright (C) 2026 Texas Instruments Incorporated - https://www.ti.com/
#
# SPDX-License-Identifier: BSD-3-Clause
#

K3LOW_DDR_CFG_HEADER	:=	am62l3_beaglebadge_lp4_50_800.h
$(eval $(call add_define_val,K3LOW_DDR_CFG_HEADER,'"$(K3LOW_DDR_CFG_HEADER)"'))

BL1_SOURCES	+=	\
	${PLAT_PATH}/board/${TARGET_BOARD}/board_config.c
