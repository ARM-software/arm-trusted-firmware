#
# Copyright (c) 2017-2021, ARM Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

include lib/xlat_tables_v2/xlat_tables.mk
include lib/libfdt/libfdt.mk
include drivers/arm/gic/v2/gicv2.mk

AW_PLAT			:=	plat/allwinner

PLAT_INCLUDES		:=	-Iinclude/plat/arm/common/aarch64	\
				-I${AW_PLAT}/common/include		\
				-I${AW_PLAT}/${PLAT}/include

PLAT_BL_COMMON_SOURCES	:=	drivers/ti/uart/${ARCH}/16550_console.S	\
				${XLAT_TABLES_LIB_SRCS}			\
				${AW_PLAT}/common/plat_helpers.S	\
				${AW_PLAT}/common/sunxi_common.c

BL31_SOURCES		+=	drivers/allwinner/axp/common.c		\
				${GICV2_SOURCES}			\
				drivers/delay_timer/delay_timer.c	\
				drivers/delay_timer/generic_delay_timer.c \
				lib/cpus/${ARCH}/cortex_a53.S		\
				plat/common/plat_gicv2.c		\
				plat/common/plat_psci_common.c		\
				${AW_PLAT}/common/sunxi_bl31_setup.c	\
				${AW_PLAT}/common/sunxi_pm.c		\
				${AW_PLAT}/${PLAT}/sunxi_power.c	\
				${AW_PLAT}/common/sunxi_security.c	\
				${AW_PLAT}/common/sunxi_topology.c

# By default, attempt to use SCPI to the ARISC management processor. If SCPI
# is not enabled or SCP firmware is not loaded, fall back to a simpler native
# implementation that does not support CPU or system suspend.
#
# If SCP firmware will always be present (or absent), the unused implementation
# can be compiled out.
SUNXI_PSCI_USE_NATIVE	?=	1
SUNXI_PSCI_USE_SCPI	?=	1

$(eval $(call assert_boolean,SUNXI_PSCI_USE_NATIVE))
$(eval $(call assert_boolean,SUNXI_PSCI_USE_SCPI))
$(eval $(call add_define,SUNXI_PSCI_USE_NATIVE))
$(eval $(call add_define,SUNXI_PSCI_USE_SCPI))

ifeq (${SUNXI_PSCI_USE_NATIVE}${SUNXI_PSCI_USE_SCPI},00)
$(error "At least one of SCPI or native PSCI ops must be enabled")
endif

ifeq (${SUNXI_PSCI_USE_NATIVE},1)
BL31_SOURCES		+=	${AW_PLAT}/common/sunxi_cpu_ops.c	\
				${AW_PLAT}/common/sunxi_native_pm.c
endif

ifeq (${SUNXI_PSCI_USE_SCPI},1)
BL31_SOURCES		+=	drivers/allwinner/sunxi_msgbox.c	\
				drivers/arm/css/scpi/css_scpi.c		\
				${AW_PLAT}/common/sunxi_scpi_pm.c
endif

# The bootloader is guaranteed to only run on CPU 0 by the boot ROM.
COLD_BOOT_SINGLE_CPU		:=	1

# Do not enable SPE (not supported on ARM v8.0).
ENABLE_SPE_FOR_LOWER_ELS	:=	0

# Do not enable SVE (not supported on ARM v8.0).
ENABLE_SVE_FOR_NS		:=	0

# Enable workarounds for Cortex-A53 errata. Allwinner uses at least r0p4.
ERRATA_A53_835769		:=	1
ERRATA_A53_843419		:=	1
ERRATA_A53_855873		:=	1
ERRATA_A53_1530924		:=	1

# The traditional U-Boot load address is 160MB into DRAM.
PRELOADED_BL33_BASE		?=	0x4a000000

# The reset vector can be changed for each CPU.
PROGRAMMABLE_RESET_ADDRESS	:=	1

# Allow mapping read-only data as execute-never.
SEPARATE_CODE_AND_RODATA	:=	1

# BL31 gets loaded alongside BL33 (U-Boot) by U-Boot's SPL
RESET_TO_BL31			:=	1

# This platform is single-cluster and does not require coherency setup.
WARMBOOT_ENABLE_DCACHE_EARLY	:=	1
