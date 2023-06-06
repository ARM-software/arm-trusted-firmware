#
# Copyright (c) 2023, MediaTek Inc. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

LOCAL_DIR := $(call GET_LOCAL_DIR)

MODULE := apusys

LOCAL_SRCS-y:= ${LOCAL_DIR}/apusys.c

PLAT_INCLUDES += -I${LOCAL_DIR} -I${LOCAL_DIR}/${MTK_SOC} -I${LOCAL_DIR}/apusys_rv/2.0

$(eval $(call MAKE_MODULE,$(MODULE),$(LOCAL_SRCS-y),$(MTK_BL)))

SUB_RULES-y := ${LOCAL_DIR}/${MTK_SOC}
SUB_RULES-y += ${LOCAL_DIR}/devapc
SUB_RULES-y += ${LOCAL_DIR}/apusys_rv/2.0

$(eval $(call INCLUDE_MAKEFILE,$(SUB_RULES-y)))
