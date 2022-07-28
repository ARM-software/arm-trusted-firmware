#
# Copyright (c) 2022, MediaTek Inc. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

LOCAL_DIR := $(call GET_LOCAL_DIR)

MODULE := pmic_wrap

ifeq (${USE_PMIC_WRAP_INIT_V2}, 1)
LOCAL_SRCS-y += ${LOCAL_DIR}/pmic_wrap_init_v2.c
else
LOCAL_SRCS-y += ${LOCAL_DIR}/pmic_wrap_init.c
endif

PLAT_INCLUDES += -I${LOCAL_DIR}/
PLAT_INCLUDES += -I${LOCAL_DIR}/${MTK_SOC}

$(eval $(call MAKE_MODULE,$(MODULE),$(LOCAL_SRCS-y),$(MTK_BL)))
