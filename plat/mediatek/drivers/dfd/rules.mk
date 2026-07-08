#
# Copyright (c) 2022, MediaTek Inc. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

LOCAL_DIR := $(call GET_LOCAL_DIR)

MODULE := mtk_dfd

PLAT_INCLUDES += -I${LOCAL_DIR}
PLAT_INCLUDES += -I${LOCAL_DIR}/$(MTK_SOC)

ifeq ($(MTK_DFD_SUPPORT),1)
LOCAL_SRCS-y := ${LOCAL_DIR}/dfd.c
LOCAL_SRCS-y += ${LOCAL_DIR}/$(MTK_SOC)/plat_dfd.c

$(eval $(call MAKE_MODULE,$(MODULE),$(LOCAL_SRCS-y),$(MTK_BL)))
endif
