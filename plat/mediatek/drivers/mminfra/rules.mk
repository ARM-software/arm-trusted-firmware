#
# Copyright (c) 2025, MediaTek Inc. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

LOCAL_DIR := $(call GET_LOCAL_DIR)

MODULE := mminfra

PLAT_INCLUDES += -I$(MTK_PLAT)/include/drivers/
PLAT_INCLUDES += -I$(MTK_PLAT)/drivers/mminfra/$(MTK_SOC)

LOCAL_SRCS-y := $(LOCAL_DIR)/$(MTK_SOC)/mminfra.c

$(eval $(call MAKE_MODULE,$(MODULE),$(LOCAL_SRCS-y),$(MTK_BL)))
