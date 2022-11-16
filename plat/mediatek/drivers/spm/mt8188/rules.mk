#
# Copyright (c) 2023, MediaTek Inc. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

LOCAL_DIR := $(call GET_LOCAL_DIR)
MODULE := spm_${MTK_SOC}

define GET_UPPER_DIR
$(shell dirname ${LOCAL_DIR})
endef
UPPER_DIR := $(call GET_UPPER_DIR)

MT_SPM_FEATURE_SUPPORT := y
MT_SPM_CIRQ_FEATURE_SUPPORT := n
MT_SPMFW_SPM_SRAM_SLEEP_SUPPORT := n
MT_SPM_SSPM_NOTIFIER_SUPPORT := y
MT_SPM_UART_SUSPEND_SUPPORT := n
MT_SPM_RGU_SUPPORT := n

LOCAL_SRCS-y := ${LOCAL_DIR}/mt_spm.c
LOCAL_SRCS-y += ${LOCAL_DIR}/mt_spm_conservation.c
LOCAL_SRCS-y += ${LOCAL_DIR}/mt_spm_internal.c
LOCAL_SRCS-y += ${LOCAL_DIR}/mt_spm_pmic_wrap.c
LOCAL_SRCS-${MT_SPM_FEATURE_SUPPORT} += ${LOCAL_DIR}/mt_spm_cond.c
LOCAL_SRCS-${MT_SPM_FEATURE_SUPPORT} += ${LOCAL_DIR}/mt_spm_idle.c
LOCAL_SRCS-${MT_SPM_FEATURE_SUPPORT} += ${LOCAL_DIR}/mt_spm_suspend.c
LOCAL_SRCS-${MT_SPM_FEATURE_SUPPORT} += ${LOCAL_DIR}/constraints/mt_spm_rc_api.c
LOCAL_SRCS-${MT_SPM_FEATURE_SUPPORT} += ${LOCAL_DIR}/constraints/mt_spm_rc_bus26m.c
LOCAL_SRCS-${MT_SPM_FEATURE_SUPPORT} += ${LOCAL_DIR}/constraints/mt_spm_rc_cpu_buck_ldo.c
LOCAL_SRCS-${MT_SPM_FEATURE_SUPPORT} += ${LOCAL_DIR}/constraints/mt_spm_rc_dram.c
LOCAL_SRCS-${MT_SPM_FEATURE_SUPPORT} += ${LOCAL_DIR}/constraints/mt_spm_rc_syspll.c
LOCAL_SRCS-${MT_SPM_SSPM_NOTIFIER_SUPPORT} += ${UPPER_DIR}/version/notifier/v1/mt_spm_sspm_notifier.c

ifeq (${MT_SPM_FEATURE_SUPPORT},n)
$(eval $(call add_define,MTK_PLAT_SPM_UNSUPPORT))
endif

ifeq (${MT_SPM_CIRQ_FEATURE_SUPPORT},n)
$(eval $(call add_define,MTK_PLAT_CIRQ_UNSUPPORT))
endif

ifeq (${MT_SPMFW_SPM_SRAM_SLEEP_SUPPORT},n)
$(eval $(call add_define,MTK_PLAT_SPM_SRAM_SLP_UNSUPPORT))
endif

ifeq (${MT_SPM_SSPM_NOTIFIER_SUPPORT},n)
$(eval $(call add_define,MTK_PLAT_SPM_SSPM_NOTIFIER_UNSUPPORT))
endif

ifeq (${MT_SPM_UART_SUSPEND_SUPPORT},n)
$(eval $(call add_define,MTK_PLAT_SPM_UART_UNSUPPORT))
endif

ifeq ($(MTK_VOLTAGE_BIN_VCORE),y)
$(eval $(call add_define,MTK_VOLTAGE_BIN_VCORE_SUPPORT))
endif

ifeq ($(MT_SPM_RGU_SUPPORT),n)
$(eval $(call add_define,MTK_PLAT_SPM_RGU_UNSUPPORT))
endif

$(eval $(call MAKE_MODULE,$(MODULE),$(LOCAL_SRCS-y),$(MTK_BL)))
