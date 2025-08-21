#
# Copyright (c) 2025, MediaTek Inc. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

LOCAL_DIR := $(call GET_LOCAL_DIR)

MODULE := thermal_${MTK_SOC}

LOCAL_SRCS-y := ${LOCAL_DIR}/soc_temp_lvts.c
LOCAL_SRCS-y += ${LOCAL_DIR}/../src/thermal_lvts.c

$(eval $(call MAKE_MODULE,$(MODULE),$(LOCAL_SRCS-y),$(MTK_BL)))
