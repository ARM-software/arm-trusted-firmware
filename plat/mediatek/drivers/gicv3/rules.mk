#
# Copyright (c) 2025, MediaTek Inc. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

LOCAL_DIR := $(call GET_LOCAL_DIR)

MODULE := gicv3
LOCAL_SRCS-y := $(LOCAL_DIR)/mt_gic_v3.c

PLAT_INCLUDES += -I${LOCAL_DIR} \
		-Iinclude/plat/common/ \
		-Idrivers/arm/gic/v3/ \

$(eval $(call MAKE_MODULE,$(MODULE),$(LOCAL_SRCS-y),$(MTK_BL)))
