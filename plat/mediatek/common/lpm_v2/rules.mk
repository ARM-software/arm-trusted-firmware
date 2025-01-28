#
# Copyright (c) 2025, MediaTek Inc. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

LOCAL_DIR := $(call GET_LOCAL_DIR)

MODULE := lpm_v2

LOCAL_SRCS-y += ${LOCAL_DIR}/mt_lp_api.c
LOCAL_SRCS-y += ${LOCAL_DIR}/mt_lp_rm.c
LOCAL_SRCS-y += ${LOCAL_DIR}/mt_lp_rq.c
LOCAL_SRCS-y += ${LOCAL_DIR}/mt_lp_mmap.c
LOCAL_SRCS-y += ${LOCAL_DIR}/mt_lpm_dispatch.c

PLAT_INCLUDES += -I${LOCAL_DIR}

$(eval $(call MAKE_MODULE,$(MODULE),$(LOCAL_SRCS-y),$(MTK_BL)))
