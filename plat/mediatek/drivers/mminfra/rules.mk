#
# Copyright (c) 2025, MediaTek Inc. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

LOCAL_DIR := $(call GET_LOCAL_DIR)

MODULE := mminfra

PLAT_INCLUDES += -I${MTK_PLAT}/include/drivers/

ifeq ($(MTKLIB_PATH),)
LOCAL_SRCS-y := ${LOCAL_DIR}/mminfra_stub.c
endif

$(eval $(call MAKE_MODULE,$(MODULE),$(LOCAL_SRCS-y),$(MTK_BL)))
