#
# Copyright (c) 2022, MediaTek Inc. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

LOCAL_DIR := $(call GET_LOCAL_DIR)

MODULE := mtk_init

LOCAL_SRCS-y := $(LOCAL_DIR)/mtk_init.c
LOCAL_SRCS-y += $(LOCAL_DIR)/mtk_mmap_init.c

$(eval $(call MAKE_MODULE,$(MODULE),$(LOCAL_SRCS-y),$(MTK_BL)))
