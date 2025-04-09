#
# Copyright (c) 2017-2018, ARM Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

PLAT_PATH	:=	plat/ti/k3
TARGET_BOARD	?=	generic

include plat/ti/common/plat_common.mk
include ${PLAT_PATH}/board/${TARGET_BOARD}/board.mk

BL32_BASE ?= 0x9e800000
$(eval $(call add_define,BL32_BASE))

PRELOADED_BL33_BASE ?= 0x80080000
$(eval $(call add_define,PRELOADED_BL33_BASE))

K3_HW_CONFIG_BASE ?= 0x82000000
$(eval $(call add_define,K3_HW_CONFIG_BASE))

PLAT_INCLUDES += -Iplat/ti/k3/board/${TARGET_BOARD}/include	\
		 -Iplat/ti/common/include			\

# modify BUILD_PLAT to point to board specific build directory
BUILD_PLAT := $(abspath ${BUILD_BASE})/${PLAT}/${TARGET_BOARD}/${BUILD_TYPE}

K3_PSCI_SOURCES		+=	\
				${PLAT_PATH}/common/k3_psci.c		\

K3_TI_SCI_TRANSPORT	:=	\
				drivers/ti/ipc/sec_proxy.c		\

BL31_SOURCES		+=	\
				${PLAT_PATH}/common/k3_bl31_setup.c	\
				${PLAT_PATH}/common/k3_topology.c	\
				${K3_TI_SCI_TRANSPORT}			\
				${K3_PSCI_SOURCES}			\
