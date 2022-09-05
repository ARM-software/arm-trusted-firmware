#
# Copyright (c) 2022, MediaTek Inc. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

LOCAL_DIR := $(call GET_LOCAL_DIR)

MODULE := cpu_pm

SUB_RULES-${CONFIG_MTK_CPU_PM_SUPPORT} := $(LOCAL_DIR)/cpcv${CONFIG_MTK_CPU_PM_ARCH}

$(eval $(call INCLUDE_MAKEFILE,$(SUB_RULES-y)))
