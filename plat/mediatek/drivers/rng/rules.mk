#
# Copyright (c) 2024, MediaTek Inc. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

LOCAL_DIR := $(call GET_LOCAL_DIR)

MODULE := rng

PLAT_INCLUDES += -I${LOCAL_DIR}/${MTK_SOC}
PLAT_INCLUDES += -I${LOCAL_DIR}

LOCAL_SRCS-y := ${LOCAL_DIR}/rng.c
LOCAL_SRCS-y += ${LOCAL_DIR}/${MTK_SOC}/rng_plat.c

$(eval $(call MAKE_MODULE,$(MODULE),$(LOCAL_SRCS-y),$(MTK_BL)))
