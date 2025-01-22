#
# Copyright (c) 2024, Texas Instruments Inc. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

# We dont have system level coherency capability
USE_COHERENT_MEM	:=	0
K3_TI_SCI_MAILBOX	:=	1
$(eval $(call add_define,K3_TI_SCI_MAILBOX))

K3_TI_SCI_TRANSPORT    =      ${PLAT_PATH}/common/drivers/mailbox/mailbox.c
K3_PSCI_SOURCES		+=	\
				${PLAT_PATH}/common/am62l_psci.c	\

BL32_BASE ?= 0x80200000
$(eval $(call add_define,BL32_BASE))

PRELOADED_BL33_BASE ?= 0x82000000
$(eval $(call add_define,PRELOADED_BL33_BASE))

K3_HW_CONFIG_BASE ?= 0x88000000
$(eval $(call add_define,K3_HW_CONFIG_BASE))

include ${PLAT_PATH}/common/drivers/scmi/ti_scmi.mk
include ${PLAT_PATH}/common/drivers/pm/pm.mk
include ${PLAT_PATH}/board/am62l/pm/soc_pm.mk

BL31_SOURCES		+=	\
				${PLAT_PATH}/common/k3_svc.c		\
				drivers/scmi-msg/base.c			\
				drivers/scmi-msg/entry.c			\
				drivers/scmi-msg/smt.c			\
