#
# Copyright (c) 2025, MediaTek Inc. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

LOCAL_DIR := $(call GET_LOCAL_DIR)

MODULE := smmu

LOCAL_SRCS-y := ${LOCAL_DIR}/smmu.c

ifeq ($(MTK_SOC), mt8196)
CFLAGS += -DMTK_SMMU_MT8196
endif

$(eval $(call MAKE_MODULE,$(MODULE),$(LOCAL_SRCS-y),$(MTK_BL)))
