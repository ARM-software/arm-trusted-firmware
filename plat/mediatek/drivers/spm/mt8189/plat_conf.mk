#
# Copyright (c) 2025, MediaTek Inc. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

NOTIFIER_VER := v4

#TRACER_VER := v1

#SPM_VER := v2

#COND_CHECK_VER := v1

PMIC_WRAP_VER := v1

MTK_SPM_COMMON_DRV := y

# Enable or disable spm feature
MT_SPM_FEATURE_SUPPORT := y

# Enable or disable cirq restore
MT_SPM_CIRQ_FEATURE_SUPPORT := y

# Enable or disable get dram type from dramc
MT_SPMFW_LOAD_BY_DRAM_TYPE := n

# Enable or disable sspm sram
MT_SPMFW_SPM_SRAM_SLEEP_SUPPORT := n

# Enable or disable uart save/restore
#uart not ready -- /common/drivers/uart is not exist, /drivers/uart no rule.mk
MT_SPM_UART_SUSPEND_SUPPORT := n

# Enable or disable pmic wrap reg dump
MT_SPM_PMIC_WRAP_DUMP_SUPPORT := n

# spm timestamp support
MT_SPM_TIMESTAMP_SUPPORT := n

MTK_SPM_PMIC_LP_SUPPORT := y

CONSTRAINT_ID_ALL := 0xff
$(eval $(call add_defined_option,CONSTRAINT_ID_ALL))

ifeq (${MT_SPM_FEATURE_SUPPORT},n)
$(eval $(call add_define,MTK_PLAT_SPM_UNSUPPORT))
else
$(eval $(call add_define,MT_SPM_FEATURE_SUPPORT))
endif

ifeq (${MT_SPMFW_LOAD_BY_DRAM_TYPE},n)
$(eval $(call add_define,MTK_PLAT_DRAMC_UNSUPPORT))
endif

ifeq (${MT_SPM_CIRQ_FEATURE_SUPPORT},n)
$(eval $(call add_define,MTK_PLAT_CIRQ_UNSUPPORT))
endif

ifeq (${MT_SPMFW_SPM_SRAM_SLEEP_SUPPORT},n)
$(eval $(call add_define,MTK_PLAT_SPM_SRAM_SLP_UNSUPPORT))
endif

ifeq (${NOTIFIER_VER},)
$(eval $(call add_define,MTK_PLAT_SPM_SSPM_NOTIFIER_UNSUPPORT))
endif

ifeq (${MT_SPM_UART_SUSPEND_SUPPORT},n)
$(eval $(call add_define,MTK_PLAT_SPM_UART_UNSUPPORT))
endif

ifeq (${MT_SPM_PMIC_WRAP_DUMP_SUPPORT},n)
$(eval $(call add_define,MTK_PLAT_SPM_PMIC_WRAP_DUMP_UNSUPPORT))
endif

ifeq (${TRACER_VER},)
$(eval $(call add_define,MTK_PLAT_SPM_TRACE_UNSUPPORT))
endif

ifeq (${MT_SPM_TIMESTAMP_SUPPORT},y)
$(eval $(call add_define,MT_SPM_TIMESTAMP_SUPPORT))
endif

ifeq ($(MTK_VOLTAGE_BIN_VCORE),y)
$(eval $(call add_define,MTK_VOLTAGE_BIN_VCORE_SUPPORT))
endif

ifeq (${MTK_SPM_PMIC_LP_SUPPORT},y)
$(eval $(call add_define,MTK_SPM_PMIC_LP_SUPPORT))
endif
