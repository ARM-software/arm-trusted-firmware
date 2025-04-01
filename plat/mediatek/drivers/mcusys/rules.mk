#
# Copyright (c) 2025, MediaTek Inc. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

LOCAL_DIR := $(call GET_LOCAL_DIR)

MODULE := mcusys

ifneq ($(filter $(MTK_SOC), mt8196 mt8189),)
PLAT_INCLUDES += -I${MTK_PLAT}/include/drivers/mcusys/$(MCUSYS_VERSION)
ifneq ($(MCUPM_VERSION),)
PLAT_INCLUDES += -I${MTK_PLAT}/include/drivers/mcusys/mcupm
CFLAGS += -DMCUPM_VERSION_${MCUPM_VERSION}
endif
else
PLAT_INCLUDES += -I$(LOCAL_DIR)/$(MCUSYS_VERSION)
endif

LOCAL_SRCS-y := $(LOCAL_DIR)/mcusys.c

$(eval $(call MAKE_MODULE,$(MODULE),$(LOCAL_SRCS-y),$(MTK_BL)))
