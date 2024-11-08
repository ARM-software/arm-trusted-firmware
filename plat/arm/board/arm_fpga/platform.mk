#
# Copyright (c) 2021-2024, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

include common/fdt_wrappers.mk
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

ENABLE_BRBE_FOR_NS		:= 2
ENABLE_TRBE_FOR_NS		:= 2
ENABLE_FEAT_AMU			:= 2
ENABLE_FEAT_AMUv1p1		:= 2
ENABLE_FEAT_CSV2_2		:= 2
ENABLE_FEAT_ECV			:= 2
ENABLE_FEAT_FGT			:= 2
ENABLE_FEAT_HCX			:= 2
ENABLE_FEAT_MTE2		:= 2
ENABLE_FEAT_TCR2		:= 2
ENABLE_SYS_REG_TRACE_FOR_NS	:= 2
ENABLE_TRF_FOR_NS		:= 2
ENABLE_SME_FOR_NS		:= 2
ENABLE_SME2_FOR_NS		:= 2
ENABLE_FEAT_LS64_ACCDATA	:= 2

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
	FPGA_CPU_LIBS	+=	lib/cpus/aarch64/cortex_a510.S			\
				lib/cpus/aarch64/cortex_a520.S			\
				lib/cpus/aarch64/cortex_a715.S			\
				lib/cpus/aarch64/cortex_a720.S			\
				lib/cpus/aarch64/cortex_x3.S 			\
				lib/cpus/aarch64/cortex_x4.S			\
				lib/cpus/aarch64/neoverse_n_common.S		\
				lib/cpus/aarch64/neoverse_n1.S			\
				lib/cpus/aarch64/neoverse_n2.S			\
				lib/cpus/aarch64/neoverse_v1.S			\
				lib/cpus/aarch64/cortex_a725.S		\
				lib/cpus/aarch64/cortex_x925.S

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

GIC_ENABLE_V4_EXTN	:=	1

# Include GICv3 driver files
include drivers/arm/gic/v3/gicv3.mk

FPGA_GIC_SOURCES	:=	${GICV3_SOURCES}			\
				plat/common/plat_gicv3.c		\
				plat/arm/board/arm_fpga/fpga_gicv3.c

FDT_SOURCES		:=	fdts/arm_fpga.dts

PLAT_INCLUDES		:=	-Iplat/arm/board/arm_fpga/include

PLAT_BL_COMMON_SOURCES	:=	plat/arm/board/arm_fpga/${ARCH}/fpga_helpers.S

BL31_SOURCES		+=	common/fdt_fixup.c				\
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

BL31_SOURCES		+=	${FDT_WRAPPERS_SOURCES}

$(eval $(call MAKE_S,$(BUILD_PLAT),plat/arm/board/arm_fpga/rom_trampoline.S,bl31,BL31))
$(eval $(call MAKE_S,$(BUILD_PLAT),plat/arm/board/arm_fpga/kernel_trampoline.S,bl31,BL31))
$(eval $(call MAKE_LD,$(BUILD_PLAT)/build_axf.ld,plat/arm/board/arm_fpga/build_axf.ld.S,bl31,BL31))

ifeq ($($(ARCH)-ld-id),gnu-gcc)
        AXF_LDFLAGS	+=	-Wl,--build-id=none -mno-fix-cortex-a53-843419
else
        AXF_LDFLAGS	+=	--build-id=none
endif

AXF_LDFLAGS += -nostdlib -no-pie

bl31.axf: bl31 dtbs ${BUILD_PLAT}/rom_trampoline.o ${BUILD_PLAT}/kernel_trampoline.o ${BUILD_PLAT}/build_axf.ld
	$(s)echo "  LD      $@"
	$(q)$($(ARCH)-ld) -T ${BUILD_PLAT}/build_axf.ld -L ${BUILD_PLAT} ${AXF_LDFLAGS} -s -n -o ${BUILD_PLAT}/bl31.axf

all: bl31.axf
