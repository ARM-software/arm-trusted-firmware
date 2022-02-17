#
# Copyright 2018-2020 NXP
#
# SPDX-License-Identifier: BSD-3-Clause
#
#

CREATE_PBL	?=	${CREATE_PBL_TOOL_PATH}/create_pbl${BIN_EXT}
BYTE_SWAP	?=	${CREATE_PBL_TOOL_PATH}/byte_swap${BIN_EXT}

HOST_GCC	:= gcc

#SWAP is required for Chassis 2 platforms - LS102, ls1043 and ls1046 for QSPI
ifeq (${SOC},ls1046a)
SOC_NUM :=	1046a
SWAP	= 	1
CH	=	2
else ifeq (${SOC},ls1043a)
SOC_NUM :=	1043a
SWAP	= 	1
CH	=	2
else ifeq (${SOC},ls1012a)
SOC_NUM :=	1012a
SWAP	= 	1
CH	=	2
else ifeq (${SOC},ls1088a)
SOC_NUM :=	1088a
CH	=	3
else ifeq (${SOC},ls2088a)
SOC_NUM :=	2088a
CH	=	3
else ifeq (${SOC},lx2160a)
SOC_NUM :=	2160a
CH	=	3
else ifeq (${SOC},ls1028a)
SOC_NUM :=	1028a
CH	=	3
else
$(error "Check SOC Not defined in create_pbl.mk.")
endif

ifeq (${CH},2)

include ${CREATE_PBL_TOOL_PATH}/pbl_ch2.mk

endif #CH2

ifeq (${CH},3)

include ${CREATE_PBL_TOOL_PATH}/pbl_ch3.mk

endif #CH3
