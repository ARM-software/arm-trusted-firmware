#
# Copyright (c) 2024, MediaTek Inc. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

LOCAL_DIR := $(call GET_LOCAL_DIR)

MODULE := vcp

SUB_RULES-y := $(LOCAL_DIR)/rv

$(eval $(call INCLUDE_MAKEFILE,$(SUB_RULES-y)))
