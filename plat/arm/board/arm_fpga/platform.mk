#
# Copyright (c) 2020, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

include lib/libfdt/libfdt.mk

RESET_TO_BL31 := 1
ifeq (${RESET_TO_BL31}, 0)
$(error "This is a BL31-only port; RESET_TO_BL31 must be enabled")
endif

ifeq (${ENABLE_PIE}, 1)
override SEPARATE_CODE_AND_RODATA := 1
endif

CTX_INCLUDE_AARCH32_REGS := 0
ifeq (${CTX_INCLUDE_AARCH32_REGS}, 1)
$(error "This is an AArch64-only port; CTX_INCLUDE_AARCH32_REGS must be disabled")
endif

ifeq (${TRUSTED_BOARD_BOOT}, 1)
$(error "TRUSTED_BOARD_BOOT must be disabled")
endif

PRELOADED_BL33_BASE := 0x80080000

FPGA_PRELOADED_DTB_BASE := 0x80070000
$(eval $(call add_define,FPGA_PRELOADED_DTB_BASE))

FPGA_PRELOADED_CMD_LINE := 0x1000
$(eval $(call add_define,FPGA_PRELOADED_CMD_LINE))

# Treating this as a memory-constrained port for now
USE_COHERENT_MEM	:=	0

# This can be overridden depending on CPU(s) used in the FPGA image
HW_ASSISTED_COHERENCY	:=	1

PL011_GENERIC_UART	:=	1

SUPPORT_UNKNOWN_MPID	?=	1

FPGA_CPU_LIBS	:=	lib/cpus/${ARCH}/aem_generic.S

# select a different set of CPU files, depending on whether we compile for
# hardware assisted coherency cores or not
ifeq (${HW_ASSISTED_COHERENCY}, 0)
# Cores used without DSU
	FPGA_CPU_LIBS	+=	lib/cpus/aarch64/cortex_a35.S	\
				lib/cpus/aarch64/cortex_a53.S	\
				lib/cpus/aarch64/cortex_a57.S	\
				lib/cpus/aarch64/cortex_a72.S	\
				lib/cpus/aarch64/cortex_a73.S
else
# AArch64-only cores
	FPGA_CPU_LIBS	+=	lib/cpus/aarch64/cortex_a76.S		\
				lib/cpus/aarch64/cortex_a76ae.S		\
				lib/cpus/aarch64/cortex_a77.S		\
				lib/cpus/aarch64/cortex_a78.S		\
				lib/cpus/aarch64/neoverse_n1.S		\
				lib/cpus/aarch64/neoverse_e1.S		\
				lib/cpus/aarch64/neoverse_v1.S		\
				lib/cpus/aarch64/cortex_a78_ae.S	\
				lib/cpus/aarch64/cortex_a65.S		\
				lib/cpus/aarch64/cortex_a65ae.S		\
				lib/cpus/aarch64/cortex_klein.S		\
				lib/cpus/aarch64/cortex_matterhorn.S

# AArch64/AArch32 cores
	FPGA_CPU_LIBS	+=	lib/cpus/aarch64/cortex_a55.S	\
				lib/cpus/aarch64/cortex_a75.S
endif

ifeq (${SUPPORT_UNKNOWN_MPID}, 1)
# Add support for unknown/invalid MPIDs (aarch64 only)
$(eval $(call add_define,SUPPORT_UNKNOWN_MPID))
	FPGA_CPU_LIBS	+=	lib/cpus/aarch64/generic.S
endif

# Allow detection of GIC-600
GICV3_SUPPORT_GIC600	:=	1

# Include GICv3 driver files
include drivers/arm/gic/v3/gicv3.mk

FPGA_GIC_SOURCES	:=	${GICV3_SOURCES}			\
				plat/common/plat_gicv3.c		\
				plat/arm/board/arm_fpga/fpga_gicv3.c

FDT_SOURCES		:=	fdts/arm_fpga.dts

PLAT_INCLUDES		:=	-Iplat/arm/board/arm_fpga/include

PLAT_BL_COMMON_SOURCES	:=	plat/arm/board/arm_fpga/${ARCH}/fpga_helpers.S

BL31_SOURCES		+=	common/fdt_wrappers.c				\
				common/fdt_fixup.c				\
				drivers/delay_timer/delay_timer.c		\
				drivers/delay_timer/generic_delay_timer.c	\
				drivers/arm/pl011/${ARCH}/pl011_console.S	\
				plat/common/plat_psci_common.c			\
				plat/arm/board/arm_fpga/fpga_pm.c			\
				plat/arm/board/arm_fpga/fpga_topology.c		\
				plat/arm/board/arm_fpga/fpga_console.c		\
				plat/arm/board/arm_fpga/fpga_bl31_setup.c		\
				${FPGA_CPU_LIBS}				\
				${FPGA_GIC_SOURCES}

$(eval $(call MAKE_S,$(BUILD_PLAT),plat/arm/board/arm_fpga/rom_trampoline.S,31))
$(eval $(call MAKE_LD,$(BUILD_PLAT)/build_axf.ld,plat/arm/board/arm_fpga/build_axf.ld.S,31))

bl31.axf: bl31 dtbs ${BUILD_PLAT}/rom_trampoline.o ${BUILD_PLAT}/build_axf.ld
	$(ECHO) "  LD      $@"
	$(Q)$(LD) -T ${BUILD_PLAT}/build_axf.ld -L ${BUILD_PLAT} --strip-debug -o ${BUILD_PLAT}/bl31.axf

all: bl31.axf
