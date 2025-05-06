#
# Copyright (c) 2025, MediaTek Inc. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

LOCAL_DIR := $(call GET_LOCAL_DIR)

MODULE := slbc

LOCAL_SOC_DIR := $(LOCAL_DIR)/$(MTK_SOC)
PLATFORM_DIR_EXIST := $(shell test -f $(LOCAL_SOC_DIR)/rules.mk && echo yes)

ifeq ($(PLATFORM_DIR_EXIST), yes)
SUB_RULES-y := $(LOCAL_SOC_DIR)
$(eval $(call INCLUDE_MAKEFILE,$(SUB_RULES-y)))
endif
