#
# Copyright (c) 2022, MediaTek Inc. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

LOCAL_DIR := $(call GET_LOCAL_DIR)

MODULE := gic600
LOCAL_SRCS-y := $(LOCAL_DIR)/mt_gic_v3.c

PLAT_INCLUDES += -I${LOCAL_DIR}

$(eval $(call MAKE_MODULE,$(MODULE),$(LOCAL_SRCS-y),$(MTK_BL)))
