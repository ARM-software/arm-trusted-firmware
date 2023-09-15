#
# Copyright (c) 2023, Linaro Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

include lib/libfdt/libfdt.mk
include common/fdt_wrappers.mk

PLAT_INCLUDES		:=	-Iinclude/plat/arm/common/		\
				-I${PLAT_QEMU_COMMON_PATH}/include	\
				-I${PLAT_QEMU_PATH}/include		\
				-Iinclude/common/tbbr

ifeq (${ARCH},aarch32)
QEMU_CPU_LIBS		:=	lib/cpus/${ARCH}/cortex_a15.S
else
QEMU_CPU_LIBS		:=	lib/cpus/aarch64/aem_generic.S		\
				lib/cpus/aarch64/cortex_a53.S		\
				lib/cpus/aarch64/cortex_a55.S		\
				lib/cpus/aarch64/cortex_a57.S		\
				lib/cpus/aarch64/cortex_a72.S		\
				lib/cpus/aarch64/cortex_a76.S		\
				lib/cpus/aarch64/cortex_a710.S		\
				lib/cpus/aarch64/neoverse_n_common.S	\
				lib/cpus/aarch64/neoverse_n1.S		\
				lib/cpus/aarch64/neoverse_v1.S		\
				lib/cpus/aarch64/neoverse_n2.S		\
				lib/cpus/aarch64/qemu_max.S

PLAT_INCLUDES		+=	-Iinclude/plat/arm/common/${ARCH}

# Cpu core architecture level:
# v8.0: a53, a57, a72
# v8.2: a76, n1
# v8.4: v1
# v9.0: a710, n2
#
# let treat v9.0 as v8.5 as they share cpu features
# https://developer.arm.com/documentation/102378/0201/Armv8-x-and-Armv9-x-extensions-and-features

ARM_ARCH_MAJOR		:=	8
ARM_ARCH_MINOR		:=	5
endif

PLAT_BL_COMMON_SOURCES	:=	${PLAT_QEMU_COMMON_PATH}/qemu_common.c		\
				${PLAT_QEMU_COMMON_PATH}/qemu_console.c		\
				drivers/arm/pl011/${ARCH}/pl011_console.S

include lib/xlat_tables_v2/xlat_tables.mk
PLAT_BL_COMMON_SOURCES	+=	${XLAT_TABLES_LIB_SRCS}

ifneq ($(ENABLE_STACK_PROTECTOR), 0)
	PLAT_BL_COMMON_SOURCES += ${PLAT_QEMU_COMMON_PATH}/qemu_stack_protector.c
endif

BL1_SOURCES		+=	drivers/io/io_semihosting.c		\
				drivers/io/io_storage.c			\
				drivers/io/io_fip.c			\
				drivers/io/io_memmap.c			\
				lib/semihosting/semihosting.c		\
				lib/semihosting/${ARCH}/semihosting_call.S	\
				${PLAT_QEMU_COMMON_PATH}/qemu_io_storage.c	\
				${PLAT_QEMU_COMMON_PATH}/${ARCH}/plat_helpers.S	\
				${PLAT_QEMU_COMMON_PATH}/qemu_bl1_setup.c	\
				${QEMU_CPU_LIBS}

BL2_SOURCES		+=	drivers/io/io_semihosting.c		\
				drivers/io/io_storage.c			\
				drivers/io/io_fip.c			\
				drivers/io/io_memmap.c			\
				lib/semihosting/semihosting.c		\
				lib/semihosting/${ARCH}/semihosting_call.S		\
				${PLAT_QEMU_COMMON_PATH}/qemu_io_storage.c		\
				${PLAT_QEMU_COMMON_PATH}/${ARCH}/plat_helpers.S		\
				${PLAT_QEMU_COMMON_PATH}/qemu_bl2_setup.c		\
				${PLAT_QEMU_COMMON_PATH}/qemu_bl2_mem_params_desc.c	\
				${PLAT_QEMU_COMMON_PATH}/qemu_image_load.c		\
				common/desc_image_load.c		\
				common/fdt_fixup.c

BL31_SOURCES		+=	${QEMU_CPU_LIBS}				\
				lib/semihosting/semihosting.c			\
				lib/semihosting/${ARCH}/semihosting_call.S	\
				plat/common/plat_psci_common.c			\
				${PLAT_QEMU_COMMON_PATH}/aarch64/plat_helpers.S	\
				${PLAT_QEMU_COMMON_PATH}/qemu_bl31_setup.c	\
				common/fdt_fixup.c				\
				${QEMU_GIC_SOURCES}

# CPU flag enablement
ifeq (${ARCH},aarch64)

# Later QEMU versions support SME and SVE.
# SPM_MM is not compatible with ENABLE_SVE_FOR_NS (build breaks)
ifeq (${SPM_MM},1)
	ENABLE_SVE_FOR_NS	:= 0
	ENABLE_SME_FOR_NS	:= 0
else
	ENABLE_SVE_FOR_NS	:= 2
	ENABLE_SME_FOR_NS	:= 2
endif

# QEMU will use the RNDR instruction for the stack protector canary.
ENABLE_FEAT_RNG			:= 2

# QEMU 7.2+ has support for FGT and Linux needs it enabled to boot on max
ENABLE_FEAT_FGT			:= 2

# Treating this as a memory-constrained port for now
USE_COHERENT_MEM	:=	0

# This can be overridden depending on CPU(s) used in the QEMU image
HW_ASSISTED_COHERENCY	:=	1

CTX_INCLUDE_AARCH32_REGS := 0
ifeq (${CTX_INCLUDE_AARCH32_REGS}, 1)
$(error "This is an AArch64-only port; CTX_INCLUDE_AARCH32_REGS must be disabled")
endif

# Pointer Authentication sources
ifeq (${ENABLE_PAUTH}, 1)
PLAT_BL_COMMON_SOURCES	+=	plat/arm/common/aarch64/arm_pauth.c
CTX_INCLUDE_PAUTH_REGS	:=	1
endif

endif
