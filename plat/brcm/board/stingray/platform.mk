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

# BL31 is in DRAM
ARM_BL31_IN_DRAM	:=	1

USE_CRMU_SRAM := yes

# Use single cluster
ifeq (${USE_SINGLE_CLUSTER},yes)
$(info Using Single Cluster)
$(eval $(call add_define,USE_SINGLE_CLUSTER))
endif

ifeq (${BOARD_CFG},)
BOARD_CFG := bcm958742k
endif

# BL31 build for standalone mode
ifeq (${STANDALONE_BL31},yes)
RESET_TO_BL31 := 1
$(info Using RESET_TO_BL31)
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
				plat/${SOC_DIR}/src/tz_sec.c \
				drivers/arm/tzc/tzc400.c \
				plat/${SOC_DIR}/src/topology.c


# Include GICv3 driver files
include drivers/arm/gic/v3/gicv3.mk

BRCM_GIC_SOURCES	:=	${GICV3_SOURCES}		\
				plat/common/plat_gicv3.c	\
				plat/brcm/common/brcm_gicv3.c

BL31_SOURCES		+=	\
				drivers/arm/ccn/ccn.c \
				plat/brcm/board/common/timer_sync.c \
				plat/brcm/common/brcm_ccn.c \
				plat/common/plat_psci_common.c \
				plat/${SOC_DIR}/driver/ihost_pll_config.c \
				${BRCM_GIC_SOURCES}

ifdef SCP_BL2
PLAT_INCLUDES		+=	-Iplat/brcm/common/

BL31_SOURCES		+=	plat/brcm/common/brcm_mhu.c \
				plat/brcm/common/brcm_scpi.c \
				plat/${SOC_DIR}/src/brcm_pm_ops.c
else
BL31_SOURCES		+=	plat/${SOC_DIR}/src/ihost_pm.c \
				plat/${SOC_DIR}/src/pm.c
endif

# Do not execute the startup code on warm reset.
PROGRAMMABLE_RESET_ADDRESS	:=	1
