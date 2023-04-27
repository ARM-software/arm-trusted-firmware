#
# Copyright (c) 2023, MediaTek Inc. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

LOCAL_DIR := $(call GET_LOCAL_DIR)

MODULE := apusys_rv_${MTK_SOC}

PLAT_INCLUDES += -I${MTK_PLAT}/drivers/apusys/${MTK_SOC}

LOCAL_SRCS-y := ${LOCAL_DIR}/apusys_rv.c

$(eval $(call MAKE_MODULE,$(MODULE),$(LOCAL_SRCS-y),$(MTK_BL)))
