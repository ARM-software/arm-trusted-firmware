#
# Copyright (c) 2023, MediaTek Inc. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

LOCAL_DIR := $(call GET_LOCAL_DIR)

MODULE := lpm

LOCAL_SRCS-y := $(LOCAL_DIR)/mt_lp_api.c
LOCAL_SRCS-y += $(LOCAL_DIR)/mt_lp_rm.c
LOCAL_SRCS-y += $(LOCAL_DIR)/mt_lp_rq.c

PLAT_INCLUDES += -I${LOCAL_DIR}

$(eval $(call MAKE_MODULE,$(MODULE),$(LOCAL_SRCS-y),$(MTK_BL)))
