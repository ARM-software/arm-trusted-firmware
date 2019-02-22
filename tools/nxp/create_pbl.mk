#
# Copyright 2018-2019 NXP
#
# SPDX-License-Identifier: BSD-3-Clause
#

CREATE_PBL	?=	${PLAT_TOOL_PATH}/create_pbl${BIN_EXT}
BYTE_SWAP	?=	${PLAT_TOOL_PATH}/byte_swap${BIN_EXT}

HOST_GCC	:= gcc

SOC_NUM :=	1043
SWAP	= 	1
CH	=	2

ifeq (${CH},2)
include ${PLAT_TOOL_PATH}/pbl_ch2.mk
endif #CH2

