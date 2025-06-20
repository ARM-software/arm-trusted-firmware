#
# Copyright (c) 2022-2025, MediaTek Inc. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

LOCAL_DIR := $(call GET_LOCAL_DIR)

MODULE := pmic

ifeq (${CONFIG_MTK_PMIC_SHUTDOWN_V2}, y)
LOCAL_SRCS-y := ${LOCAL_DIR}/pmic_common_swap_api.c
LOCAL_SRCS-y += ${LOCAL_DIR}/pmic_psc.c
LOCAL_SRCS-${CONFIG_MTK_PMIC_LOWPOWER} += ${LOCAL_DIR}/${MTK_SOC}/pmic_lowpower_init.c
LOCAL_SRCS-${CONFIG_MTK_PMIC_LOWPOWER} += ${LOCAL_DIR}/${MTK_SOC}/pmic_swap_api.c
LOCAL_SRCS-${CONFIG_MTK_PMIC_SHUTDOWN_CFG} += ${LOCAL_DIR}/${MTK_SOC}/pmic_shutdown_cfg.c
else
LOCAL_SRCS-y += ${LOCAL_DIR}/pmic.c
PLAT_INCLUDES += -I${LOCAL_DIR}/
endif

$(eval $(call MAKE_MODULE,$(MODULE),$(LOCAL_SRCS-y),$(MTK_BL)))
$(eval $(call add_defined_option,CONFIG_MTK_PMIC_SPT_SUPPORT))
$(eval $(call add_defined_option,CONFIG_MTK_PMIC_SHUTDOWN_CFG))

#Include sub rules.mk
ifneq (${PMIC_CHIP},)
SUB_RULES-y := $(LOCAL_DIR)/$(PMIC_CHIP)
#Expand sub rules.mk
$(eval $(call INCLUDE_MAKEFILE,$(SUB_RULES-y)))
endif
