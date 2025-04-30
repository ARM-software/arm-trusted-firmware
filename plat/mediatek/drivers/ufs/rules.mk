#
# Copyright (c) 2025, MediaTek Inc. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

LOCAL_DIR := $(call GET_LOCAL_DIR)

MODULE := ufs

PLAT_INCLUDES += -I$(LOCAL_DIR)
PLAT_INCLUDES += -I$(MTK_PLAT)/include/drivers/

LOCAL_SRCS-y := $(LOCAL_DIR)/ufs_ctrl.c
LOCAL_SRCS-y += $(LOCAL_DIR)/$(MTK_SOC)/ufs_ctrl_soc.c
ifeq ($(MTKLIB_PATH),)
LOCAL_SRCS-y += $(LOCAL_DIR)/ufs_stub.c
endif

$(eval $(call MAKE_MODULE,$(MODULE),$(LOCAL_SRCS-y),$(MTK_BL)))
