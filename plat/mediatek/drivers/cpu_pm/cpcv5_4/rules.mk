#
# Copyright (c) 2025, MediaTek Inc. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

#Prologue, init variable
LOCAL_DIR := $(call GET_LOCAL_DIR)

CPU_PM_PWR_REQ := y
CPU_PM_PWR_REQ_DEBUG := n

#Define your module name
MODULE := cpcv${CONFIG_MTK_CPU_PM_ARCH}

#Add your source code here
LOCAL_SRCS-y := ${LOCAL_DIR}/mt_cpu_pm.c \
		${LOCAL_DIR}/mt_cpu_pm_cpc.c \
		${LOCAL_DIR}/mt_cpu_pm_smc.c \
		${LOCAL_DIR}/mt_ppu.c

LOCAL_SRCS-$(CPU_PM_TINYSYS_SUPPORT) += ${LOCAL_DIR}/mt_cpu_pm_mbox.c
LOCAL_SRCS-$(CONFIG_MTK_SMP_EN) += ${LOCAL_DIR}/mt_smp.c

LOCAL_SRCS-${CPU_PM_IRQ_REMAIN_ENABLE} += ${LOCAL_DIR}/mt_lp_irqremain.c
$(eval $(call add_defined_option,CPU_PM_IRQ_REMAIN_ENABLE))

$(eval $(call add_defined_option,CPU_PM_DOMAIN_CORE_ONLY))
$(eval $(call add_defined_option,CPU_PM_CORE_ARCH64_ONLY))
$(eval $(call add_defined_option,CPU_PM_TINYSYS_SUPPORT))

$(eval $(call add_defined_option,CPU_PM_PWR_REQ))
$(eval $(call add_defined_option,CPU_PM_PWR_REQ_DEBUG))

$(eval $(call add_defined_option,CONFIG_MTK_CPU_ILDO))
$(eval $(call add_defined_option,CPU_PM_CPU_RET_MASK))

#Epilogue, build as module
$(eval $(call MAKE_MODULE,$(MODULE),$(LOCAL_SRCS-y),$(MTK_BL)))

$(eval $(call add_defined_option,CPU_PM_ACP_FSM))
