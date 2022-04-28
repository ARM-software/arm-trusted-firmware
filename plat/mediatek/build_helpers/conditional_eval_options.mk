#
# Copyright (c) 2022, MediaTek Inc. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

# Conditional makefile variable assignment

# Options depend on BUILD_TYPE variable
ifeq ($(BUILD_TYPE),release)
MTK_DEBUGSYS_LOCK := 1
MTK_GET_PERM_DIS := 1
ERRATA_KLEIN_2218950 := 0
ERRATA_KLEIN_2184257 := 0
ERRATA_KLEIN_BOOKER := 0
ERRATA_MTH_BOOKER := 0
ERRATA_MTHELP_BOOKER := 0
CRASH_REPORTING := 1
CONFIG_MTK_BL31_RAMDUMP := 0
endif

ifeq ($(BUILD_TYPE),debug)
MTK_PTP3_PROC_DEBUG := 1
MTK_SRAMRC_DEBUG := 1
MTK_IOMMU_DEBUG := 1
MTK_DCM_DEBUG := 1
MTK_EMI_MPU_DEBUG := 1
endif

ifeq (${SPD},none)
SPD_NONE:=1
$(eval $(call add_define,SPD_NONE))
endif

# TEE OS config
ifeq ($(SPD), tbase)
CONFIG_TBASE := y
endif

# MICROTRUST OS config
ifeq ($(SPD), teeid)
CONFIG_MICROTRUST_TEEI := y
endif

ifeq (${CONFIG_ARCH_ARM_V8_2},y)
ARCH_VERSION := armv8_2
endif

ifeq (${CONFIG_ARCH_ARM_V9},y)
ARCH_VERSION := armv9
endif
