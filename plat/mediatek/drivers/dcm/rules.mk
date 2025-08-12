#
# Copyright (c) 2022, MediaTek Inc. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

LOCAL_DIR := $(call GET_LOCAL_DIR)

MODULE := mtk_dcm

LOCAL_SRCS-y := ${LOCAL_DIR}/mtk_dcm.c
LOCAL_SRCS-y += ${LOCAL_DIR}/mtk_dcm_utils.c

PLAT_INCLUDES += -I${LOCAL_DIR}

$(eval $(call MAKE_MODULE,$(MODULE),$(LOCAL_SRCS-y),$(MTK_BL)))
