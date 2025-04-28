#
# Copyright (c) 2023-2024, MediaTek Inc. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

LOCAL_DIR := $(call GET_LOCAL_DIR)

MODULE := apusys

LOCAL_SRCS-y:= ${LOCAL_DIR}/apusys.c

PLAT_INCLUDES += -I${LOCAL_DIR} -I${LOCAL_DIR}/${MTK_SOC} -I${LOCAL_DIR}/apusys_rv/2.0

$(eval $(call add_defined_option,CONFIG_MTK_APUSYS_CE_SUPPORT))
$(eval $(call add_defined_option,CONFIG_MTK_APUSYS_LOGTOP_SUPPORT))
$(eval $(call add_defined_option,CONFIG_MTK_APUSYS_RV_APUMMU_SUPPORT))
$(eval $(call add_defined_option,CONFIG_MTK_APUSYS_RV_COREDUMP_WA_SUPPORT))
$(eval $(call add_defined_option,CONFIG_MTK_APUSYS_RV_IOMMU_HW_SEM_SUPPORT))
$(eval $(call add_defined_option,CONFIG_MTK_APUSYS_SEC_CTRL))

$(eval $(call MAKE_MODULE,$(MODULE),$(LOCAL_SRCS-y),$(MTK_BL)))

SUB_RULES-y := ${LOCAL_DIR}/${MTK_SOC}
SUB_RULES-y += ${LOCAL_DIR}/devapc
SUB_RULES-y += ${LOCAL_DIR}/apusys_rv/2.0
SUB_RULES-${CONFIG_MTK_APUSYS_SEC_CTRL} += $(LOCAL_DIR)/security_ctrl

$(eval $(call INCLUDE_MAKEFILE,$(SUB_RULES-y)))
