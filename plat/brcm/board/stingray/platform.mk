#
# Copyright (c) 2019-2020, Broadcom
#
# SPDX-License-Identifier: BSD-3-Clause
#

# Enable workaround for ERRATA_A72_859971
ERRATA_A72_859971 := 1

# Cache Coherency Interconnect Driver needed
DRIVER_CC_ENABLE := 1
$(eval $(call add_define,DRIVER_CC_ENABLE))

USE_CRMU_SRAM := yes

# Use single cluster
ifeq (${USE_SINGLE_CLUSTER},yes)
$(info Using Single Cluster)
$(eval $(call add_define,USE_SINGLE_CLUSTER))
endif

ifeq (${BOARD_CFG},)
BOARD_CFG := bcm958742k
endif

# For testing purposes, use memsys stubs.  Remove once memsys is fully tested.
USE_MEMSYS_STUBS := yes

# Default, use BL1_RW area
ifneq (${BL2_USE_BL1_RW},no)
$(eval $(call add_define,USE_BL1_RW))
endif

# Default soft reset is L3
$(eval $(call add_define,CONFIG_SOFT_RESET_L3))

include plat/brcm/board/common/board_common.mk

SOC_DIR			:= 	brcm/board/stingray

PLAT_INCLUDES		+=	-Iplat/${SOC_DIR}/include/ \
				-Iinclude/plat/brcm/common/ \
				-Iplat/brcm/common/

PLAT_BL_COMMON_SOURCES	+=	lib/cpus/aarch64/cortex_a72.S \
				plat/${SOC_DIR}/aarch64/plat_helpers.S \
				drivers/ti/uart/aarch64/16550_console.S \
				drivers/arm/tzc/tzc400.c
