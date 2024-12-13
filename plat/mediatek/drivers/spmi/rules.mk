#
# Copyright (c) 2025, MediaTek Inc. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

LOCAL_DIR := $(call GET_LOCAL_DIR)

MODULE := spmi

PLAT_INCLUDES += -I${MTK_PLAT}/drivers/spmi/pmif_v1

#Add your source code here
LOCAL_SRCS-y := ${LOCAL_DIR}/pmif_common.c
LOCAL_SRCS-y += ${LOCAL_DIR}/spmi_common.c
LOCAL_SRCS-y += ${LOCAL_DIR}/${MTK_SOC}/platform_pmif_spmi.c

#Epilogue, build as module
$(eval $(call MAKE_MODULE,$(MODULE),$(LOCAL_SRCS-y),$(MTK_BL)))
