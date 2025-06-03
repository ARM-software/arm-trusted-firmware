#
# Copyright (c) 2025, MediaTek Inc. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

LOCAL_DIR := $(call GET_LOCAL_DIR)
MODULE := spm

ifneq ($(CONFIG_MTK_SPM_VERSION),)
PLAT_INCLUDES += -I${LOCAL_DIR}/$(MTK_SOC)
PLAT_INCLUDES += -I${LOCAL_DIR}/version/notifier/inc

SUB_RULES-y += ${LOCAL_DIR}/$(CONFIG_MTK_SPM_VERSION)
$(eval $(call add_define,SPM_PLAT_IMPL))
endif

ifneq ($(filter mt8196 mt8189, $(CONFIG_MTK_SPM_VERSION)),)
ifeq ($(CONFIG_MTK_SPM_SUPPORT), y)
ifeq ($(CONFIG_MTK_SPM_COMMON_SUPPORT), y)
include ${LOCAL_DIR}/$(CONFIG_MTK_SPM_VERSION)/plat_conf.mk
PLAT_INCLUDES += -I${LOCAL_DIR}
PLAT_INCLUDES += -I${LOCAL_DIR}/version
PLAT_INCLUDES += -I${LOCAL_DIR}/common

LOCAL_SRCS-y += ${LOCAL_DIR}/mt_spm_rc_api_common.c
LOCAL_SRCS-y += ${LOCAL_DIR}/mt_spm_dispatcher.c
endif
endif

$(eval $(call add_defined_option,CONFIG_MTK_VCOREDVFS_SUPPORT))

ifneq ($(CONFIG_MTK_SPM_COMMON_SUPPORT), y)
LOCAL_SRCS-${CONFIG_MTK_SPM_SUPPORT} += ${LOCAL_DIR}/mt_spm_early_init.c
endif

ifneq ($(CONFIG_MTK_SPM_COMMON_SUPPORT), y)
LOCAL_SRCS-${CONFIG_MTK_SPM_SUPPORT} += ${LOCAL_DIR}/mt_spm_early_init.c
endif

LOCAL_SRCS-${CONFIG_MTK_VCOREDVFS_SUPPORT} += ${LOCAL_DIR}/mt_spm_vcorefs_smc.c

$(eval $(call MAKE_MODULE,$(MODULE),$(LOCAL_SRCS-y),$(MTK_BL)))

SUB_RULES-${MTK_SPM_COMMON_DRV} += ${LOCAL_DIR}/common
SUB_RULES-${MTK_SPM_COMMON_DRV} += ${LOCAL_DIR}/version

$(eval $(call add_defined_option,CONFIG_MTK_SPM_COMMON_SUPPORT))
else
$(eval $(call MAKE_MODULE,$(MODULE),$(LOCAL_SRCS-y),$(MTK_BL)))
endif

$(eval $(call INCLUDE_MAKEFILE,$(SUB_RULES-y)))
