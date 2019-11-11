#
# Copyright (c) 2020, ARM Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

RESET_TO_BL31 := 1
ifeq (${RESET_TO_BL31}, 0)
$(error "This is a BL31-only port; RESET_TO_BL31 must be enabled")
endif

CTX_INCLUDE_AARCH32_REGS := 0
ifeq (${CTX_INCLUDE_AARCH32_REGS}, 1)
$(error "This is an AArch64-only port; CTX_INCLUDE_AARCH32_REGS must be disabled")
endif

ifeq (${TRUSTED_BOARD_BOOT}, 1)
$(error "TRUSTED_BOARD_BOOT must be disabled")
endif

ifndef PRELOADED_BL33_BASE
$(error "PRELOADED_BL33_BASE is not set")
endif

ifndef FPGA_PRELOADED_DTB_BASE
$(error "FPGA_PRELOADED_DTB_BASE is not set")
else
$(eval $(call add_define,FPGA_PRELOADED_DTB_BASE))
endif

# Treating this as a memory-constrained port for now
USE_COHERENT_MEM	:=	0

# The CPU in the initial image makes use of this feature
HW_ASSISTED_COHERENCY	:=	1

FPGA_CPU_LIBS		:=	lib/cpus/${ARCH}/aem_generic.S		\
				lib/cpus/aarch64/neoverse_zeus.S

FPGA_GIC_SOURCES	:=	drivers/arm/gic/common/gic_common.c	\
				drivers/arm/gic/v3/gicv3_main.c		\
				plat/common/plat_gicv3.c

PLAT_INCLUDES		:=	-Iplat/arm/board/arm_fpga/include

PLAT_BL_COMMON_SOURCES	:=	plat/arm/board/arm_fpga/${ARCH}/fpga_helpers.S

BL31_SOURCES		+=	drivers/delay_timer/delay_timer.c		\
				drivers/delay_timer/generic_delay_timer.c	\
				drivers/arm/pl011/${ARCH}/pl011_console.S	\
				plat/common/plat_psci_common.c			\
				plat/arm/board/arm_fpga/fpga_pm.c			\
				plat/arm/board/arm_fpga/fpga_topology.c		\
				plat/arm/board/arm_fpga/fpga_console.c		\
				plat/arm/board/arm_fpga/fpga_bl31_setup.c		\
				${FPGA_CPU_LIBS}				\
				${FPGA_GIC_SOURCES}

all: bl31
