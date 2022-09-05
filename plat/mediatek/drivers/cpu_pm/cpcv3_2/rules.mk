#
# Copyright (c) 2022, MediaTek Inc. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

LOCAL_DIR := $(call GET_LOCAL_DIR)

MODULE := cpcv${CONFIG_MTK_CPU_PM_ARCH}

LOCAL_SRCS-y := ${LOCAL_DIR}/mt_cpu_pm.c ${LOCAL_DIR}/mt_cpu_pm_cpc.c

LOCAL_SRCS-$(CPU_PM_TINYSYS_SUPPORT) += ${LOCAL_DIR}/mt_cpu_pm_mbox.c
LOCAL_SRCS-$(CONFIG_MTK_SMP_EN) += ${LOCAL_DIR}/mt_smp.c

$(eval $(call add_defined_option,CPU_PM_TINYSYS_SUPPORT))

$(eval $(call MAKE_MODULE,$(MODULE),$(LOCAL_SRCS-y),$(MTK_BL)))

