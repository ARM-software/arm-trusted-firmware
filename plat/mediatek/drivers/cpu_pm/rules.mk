#
# Copyright (c) 2025, MediaTek Inc. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

LOCAL_DIR := $(call GET_LOCAL_DIR)

MODULE := cpu_pm

$(eval $(call add_defined_option,CPU_PM_CACHE_AUTO_DORMANT_DIS))

SUB_RULES-${CONFIG_MTK_CPU_PM_SUPPORT} := $(LOCAL_DIR)/cpcv${CONFIG_MTK_CPU_PM_ARCH}

ifneq ($(CPU_PWR_TOPOLOGY),)
SUB_RULES-${CONFIG_MTK_CPU_PM_SUPPORT} += $(LOCAL_DIR)/topology/$(CPU_PWR_TOPOLOGY)
else
SUB_RULES-${CONFIG_MTK_CPU_PM_SUPPORT} += $(LOCAL_DIR)/topology/default
endif

$(eval $(call INCLUDE_MAKEFILE,$(SUB_RULES-y)))
