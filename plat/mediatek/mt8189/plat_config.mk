#
# Copyright (c) 2025, MediaTek Inc. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

# Separate text code and read only data
SEPARATE_CODE_AND_RODATA := 1

# ARMv8.2 and above need enable HW assist coherence
HW_ASSISTED_COHERENCY := 1

# No need coherency memory because of HW assistency
USE_COHERENT_MEM := 0

# GIC600
GICV3_SUPPORT_GIC600 := 1

#
# MTK options
#
MCUSYS_VERSION := v1
PLAT_EXTRA_RODATA_INCLUDES := 1
CONFIG_MTK_DISABLE_CACHE_AS_RAM := $(COREBOOT)

# Configs for A78 and A55
CTX_INCLUDE_AARCH32_REGS := 0
ERRATA_A55_1530923 := 1
ERRATA_A55_1221012 := 1
ERRATA_A78_1688305 := 1
ERRATA_A78_1941498 := 1
ERRATA_A78_1951500 := 1
ERRATA_A78_1821534 := 1
ERRATA_A78_2132060 := 1
ERRATA_A78_2242635 := 1
ERRATA_A78_2376745 := 1
ERRATA_A78_2395406 := 1

CONFIG_ARCH_ARM_V8_2 := y
