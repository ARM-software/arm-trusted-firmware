#
# Copyright (c) 2022, MediaTek Inc. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

LOCAL_DIR := $(call GET_LOCAL_DIR)

MODULE := rtc

LOCAL_SRCS-y := ${LOCAL_DIR}/rtc_common.c

ifeq (${USE_RTC_MT6359P}, 1)
LOCAL_SRCS-y += ${LOCAL_DIR}/rtc_mt6359p.c
PLAT_INCLUDES += -I${LOCAL_DIR}
endif

PLAT_INCLUDES += -I${LOCAL_DIR}/${MTK_SOC}

$(eval $(call MAKE_MODULE,$(MODULE),$(LOCAL_SRCS-y),$(MTK_BL)))
