#
# Copyright (c) 2025, Texas Instruments Incorporated - https://www.ti.com/
#
# SPDX-License-Identifier: BSD-3-Clause
#

PLAT_PATH	:=	plat/ti/k3low
TARGET_BOARD	?=	am62lx

include plat/ti/common/plat_common.mk
include ${PLAT_PATH}/board/${TARGET_BOARD}/board.mk

PLAT_INCLUDES +=	\
			-I${PLAT_PATH}/board/${TARGET_BOARD}/include	\
			-I${PLAT_PATH}					\
			-Iplat/ti/common/include			\

K3_PSCI_SOURCES		+=	\
				${PLAT_PATH}/common/am62l_psci.c	\

K3_TI_SCI_TRANSPORT	:=	\
				drivers/ti/ipc/mailbox.c		\

BL31_SOURCES		+=	\
				drivers/delay_timer/delay_timer.c		\
				drivers/delay_timer/generic_delay_timer.c	\
				${K3_PSCI_SOURCES}				\
				${K3_TI_SCI_TRANSPORT}				\
				${PLAT_PATH}/common/am62l_bl31_setup.c		\
				${PLAT_PATH}/common/am62l_topology.c		\
