#
# Copyright (c) 2025, MediaTek Inc. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

#Prologue, init variable
LOCAL_DIR := $(call GET_LOCAL_DIR)

#Define your module name
MODULE := spm_${MTK_SOC}

#Add your source code here

LOCAL_SRCS-${CONFIG_MTK_SPM_SUPPORT} := ${LOCAL_DIR}/mt_spm.c
LOCAL_SRCS-${CONFIG_MTK_SPM_SUPPORT} += ${LOCAL_DIR}/mt_spm_conservation.c
LOCAL_SRCS-${CONFIG_MTK_SPM_SUPPORT} += ${LOCAL_DIR}/mt_spm_internal.c
LOCAL_SRCS-${MT_SPM_FEATURE_SUPPORT} += ${LOCAL_DIR}/mt_plat_spm_setting.c
LOCAL_SRCS-${MT_SPM_FEATURE_SUPPORT} += ${LOCAL_DIR}/mt_spm_idle.c
LOCAL_SRCS-${MT_SPM_FEATURE_SUPPORT} += ${LOCAL_DIR}/mt_spm_suspend.c
LOCAL_SRCS-${MT_SPM_FEATURE_SUPPORT} += ${LOCAL_DIR}/mt_spm_stats.c
LOCAL_SRCS-${MT_SPM_FEATURE_SUPPORT} += ${LOCAL_DIR}/constraints/mt_spm_rc_api.c
LOCAL_SRCS-${MT_SPM_FEATURE_SUPPORT} += ${LOCAL_DIR}/constraints/mt_spm_rc_bus26m.c
LOCAL_SRCS-${MT_SPM_FEATURE_SUPPORT} += ${LOCAL_DIR}/constraints/mt_spm_rc_vcore.c
LOCAL_SRCS-${MT_SPM_FEATURE_SUPPORT} += ${LOCAL_DIR}/constraints/mt_spm_rc_syspll.c
LOCAL_SRCS-${MTK_SPM_PMIC_LP_SUPPORT} += ${LOCAL_DIR}/mt_spm_pmic_lp.c
LOCAL_SRCS-${MT_SPM_FEATURE_SUPPORT} += ${LOCAL_DIR}/mt_spm_hwreq.c
LOCAL_SRCS-${CONFIG_MTK_VCOREDVFS_SUPPORT} += ${LOCAL_DIR}/mt_spm_vcorefs.c
LOCAL_SRCS-${CONFIG_MTK_VCOREDVFS_SUPPORT} += ${LOCAL_DIR}/mt_vcore_dvfsrc_plat.c

#Epilogue, build as module
$(eval $(call MAKE_MODULE,$(MODULE),$(LOCAL_SRCS-y),$(MTK_BL)))
$(eval $(call add_defined_option,CONFIG_MTK_VCOREDVFS_SUPPORT))
$(eval $(call add_defined_option,CONFIG_MTK_VCOREDVFS_LK_SUPPORT))
$(eval $(call add_defined_option,CONFIG_MTK_VCOREDVFS_PLAT_CMD))
$(eval $(call add_defined_option,MTK_VCORE_DVFS_PAUSE))
