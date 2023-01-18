#
# Copyright (c) 2023, MediaTek Inc. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

LOCAL_DIR := $(call GET_LOCAL_DIR)
MODULE := spm

$(eval $(call MAKE_MODULE,$(MODULE),$(LOCAL_SRCS-y),$(MTK_BL)))

ifneq ($(CONFIG_MTK_SPM_VERSION),)
PLAT_INCLUDES += -I${LOCAL_DIR}/$(MTK_SOC)
PLAT_INCLUDES += -I${LOCAL_DIR}/version/notifier/inc

SUB_RULES-y += ${LOCAL_DIR}/$(CONFIG_MTK_SPM_VERSION)
$(eval $(call add_define,SPM_PLAT_IMPL))
endif

$(eval $(call INCLUDE_MAKEFILE,$(SUB_RULES-y)))
