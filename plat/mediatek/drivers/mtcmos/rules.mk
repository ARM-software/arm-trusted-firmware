#
# Copyright (c) 2025, MediaTek Inc. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
LOCAL_DIR := $(call GET_LOCAL_DIR)

MODULE := mtcmos
LOCAL_SRCS-y := $(LOCAL_DIR)/${MTK_SOC}/mtcmos.c

PLAT_INCLUDES += -I${LOCAL_DIR}/${MTK_SOC}

$(eval $(call MAKE_MODULE,$(MODULE),$(LOCAL_SRCS-y),$(MTK_BL)))
