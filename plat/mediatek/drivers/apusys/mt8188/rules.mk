#
# Copyright (c) 2023, MediaTek Inc. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

LOCAL_DIR := $(call GET_LOCAL_DIR)

MODULE := apusys_${MTK_SOC}

LOCAL_SRCS-y := ${LOCAL_DIR}/apusys_devapc.c
LOCAL_SRCS-y += ${LOCAL_DIR}/apusys_power.c
LOCAL_SRCS-y += ${LOCAL_DIR}/apusys_security_ctrl_plat.c

$(eval $(call MAKE_MODULE,$(MODULE),$(LOCAL_SRCS-y),$(MTK_BL)))
