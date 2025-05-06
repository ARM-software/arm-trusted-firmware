#
# Copyright (c) 2025, MediaTek Inc. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

LOCAL_DIR := $(call GET_LOCAL_DIR)

SUB_RULES := $(LOCAL_DIR)/$(MTK_SOC)

$(eval $(call INCLUDE_MAKEFILE,$(SUB_RULES)))
