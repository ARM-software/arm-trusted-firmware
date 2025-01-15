#
# Copyright (c) 2025, MediaTek Inc. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

LOCAL_DIR := $(call GET_LOCAL_DIR)

MODULE := topology

ifneq (,$(wildcard $(LOCAL_DIR)/$(CPU_PWR_TOPOLOGY)/topology_conf.mk))
include $(LOCAL_DIR)/$(CPU_PWR_TOPOLOGY)/topology_conf.mk
endif

ifneq ($(CPU_PWR_TOPOLOGY),)
LOCAL_SRCS-y := $(LOCAL_DIR)/$(CPU_PWR_TOPOLOGY)/topology.c
else
LOCAL_SRCS-y := $(LOCAL_DIR)/$(ARCH_VERSION)/topology.c
endif

$(eval $(call MAKE_MODULE,$(MODULE),$(LOCAL_SRCS-y),$(MTK_BL)))
