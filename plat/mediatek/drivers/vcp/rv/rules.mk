#
# Copyright (c) 2024, MediaTek Inc. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

LOCAL_DIR := $(call GET_LOCAL_DIR)

MODULE := vcp_rv_${MTK_SOC}

PLAT_INCLUDES += -I${MTK_PLAT}/drivers/vcp/${MTK_SOC}

LOCAL_SRCS-${CONFIG_MTK_TINYSYS_VCP} := ${LOCAL_DIR}/vcp_common.c
LOCAL_SRCS-${CONFIG_MTK_TINYSYS_VCP} += ${LOCAL_DIR}/mmup_common.c

$(eval $(call MAKE_MODULE,$(MODULE),$(LOCAL_SRCS-y),$(MTK_BL)))
