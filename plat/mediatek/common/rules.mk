#
# Copyright (c) 2022, MediaTek Inc. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

LOCAL_DIR := $(call GET_LOCAL_DIR)

MODULE := mtk_common

LOCAL_SRCS-y := ${LOCAL_DIR}/mtk_bl31_setup.c
LOCAL_SRCS-y += ${LOCAL_DIR}/mtk_smc_handlers.c
LOCAL_SRCS-$(MTK_SIP_KERNEL_BOOT_ENABLE) += ${LOCAL_DIR}/cold_boot.c

$(eval $(call MAKE_LOCALS,$(LOCAL_SRCS-y),$(MTK_BL)))
