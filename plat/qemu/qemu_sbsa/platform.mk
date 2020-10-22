#
# Copyright (c) 2019-2020, Linaro Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

CRASH_REPORTING	:=	1

include lib/libfdt/libfdt.mk

ifeq (${SPM_MM},1)
NEED_BL32		:=	yes
EL3_EXCEPTION_HANDLING	:=	1
GICV2_G0_FOR_EL3	:=	1
endif

# Enable new version of image loading on QEMU platforms
LOAD_IMAGE_V2		:=	1

ifeq ($(NEED_BL32),yes)
$(eval $(call add_define,QEMU_LOAD_BL32))
endif

PLAT_QEMU_PATH		:=	plat/qemu/qemu_sbsa
PLAT_QEMU_COMMON_PATH	:=	plat/qemu/common
PLAT_INCLUDES		:=	-Iinclude/plat/arm/common/			\
				-I${PLAT_QEMU_COMMON_PATH}/include		\
				-I${PLAT_QEMU_PATH}/include			\
				-Iinclude/common/tbbr

PLAT_INCLUDES		+=	-Iinclude/plat/arm/common/${ARCH}

PLAT_BL_COMMON_SOURCES	:=	${PLAT_QEMU_COMMON_PATH}/qemu_common.c		\
				${PLAT_QEMU_COMMON_PATH}/qemu_console.c		\
				drivers/arm/pl011/${ARCH}/pl011_console.S

include lib/xlat_tables_v2/xlat_tables.mk
PLAT_BL_COMMON_SOURCES	+=	${XLAT_TABLES_LIB_SRCS}

BL1_SOURCES		+=	drivers/io/io_semihosting.c			\
				drivers/io/io_storage.c				\
				drivers/io/io_fip.c				\
				drivers/io/io_memmap.c				\
				lib/semihosting/semihosting.c			\
				lib/semihosting/${ARCH}/semihosting_call.S	\
				${PLAT_QEMU_COMMON_PATH}/qemu_io_storage.c	\
				${PLAT_QEMU_COMMON_PATH}/${ARCH}/plat_helpers.S	\
				${PLAT_QEMU_COMMON_PATH}/qemu_bl1_setup.c

BL1_SOURCES		+=	lib/cpus/aarch64/cortex_a57.S

BL2_SOURCES		+=	drivers/io/io_semihosting.c			\
				drivers/io/io_storage.c				\
				drivers/io/io_fip.c				\
				drivers/io/io_memmap.c				\
				lib/semihosting/semihosting.c			\
				lib/semihosting/${ARCH}/semihosting_call.S	\
				${PLAT_QEMU_COMMON_PATH}/qemu_io_storage.c	\
				${PLAT_QEMU_COMMON_PATH}/${ARCH}/plat_helpers.S	\
				${PLAT_QEMU_COMMON_PATH}/qemu_bl2_setup.c	\
				common/fdt_fixup.c				\
				$(LIBFDT_SRCS)
ifeq (${LOAD_IMAGE_V2},1)
BL2_SOURCES		+=	${PLAT_QEMU_COMMON_PATH}/qemu_bl2_mem_params_desc.c	\
				${PLAT_QEMU_COMMON_PATH}/qemu_image_load.c		\
				common/desc_image_load.c
endif

# Include GICv3 driver files
include drivers/arm/gic/v3/gicv3.mk

QEMU_GIC_SOURCES	:=	${GICV3_SOURCES}				\
				plat/common/plat_gicv3.c			\
				${PLAT_QEMU_COMMON_PATH}/qemu_gicv3.c

BL31_SOURCES		+=	lib/cpus/aarch64/cortex_a57.S			\
				lib/semihosting/semihosting.c			\
				lib/semihosting/${ARCH}/semihosting_call.S	\
				plat/common/plat_psci_common.c			\
				${PLAT_QEMU_COMMON_PATH}/qemu_pm.c		\
				${PLAT_QEMU_COMMON_PATH}/topology.c		\
				${PLAT_QEMU_COMMON_PATH}/aarch64/plat_helpers.S	\
				${PLAT_QEMU_COMMON_PATH}/qemu_bl31_setup.c	\
				${QEMU_GIC_SOURCES}
ifeq (${SPM_MM},1)
	BL31_SOURCES		+=	${PLAT_QEMU_COMMON_PATH}/qemu_spm.c
endif

SEPARATE_CODE_AND_RODATA	:= 1
ENABLE_STACK_PROTECTOR		:= 0
ifneq ($(ENABLE_STACK_PROTECTOR), 0)
	PLAT_BL_COMMON_SOURCES	+=	${PLAT_QEMU_COMMON_PATH}/qemu_stack_protector.c
endif

MULTI_CONSOLE_API	:= 1

# Disable the PSCI platform compatibility layer
ENABLE_PLAT_COMPAT	:= 0

# Use known base for UEFI if not given from command line
# By default BL33 is at FLASH1 base
PRELOADED_BL33_BASE	?= 0x10000000

# Qemu SBSA plafrom only support SEC_SRAM
BL32_RAM_LOCATION_ID	= SEC_SRAM_ID
$(eval $(call add_define,BL32_RAM_LOCATION_ID))

# Don't have the Linux kernel as a BL33 image by default
ARM_LINUX_KERNEL_AS_BL33	:=	0
$(eval $(call assert_boolean,ARM_LINUX_KERNEL_AS_BL33))
$(eval $(call add_define,ARM_LINUX_KERNEL_AS_BL33))

ARM_PRELOADED_DTB_BASE := PLAT_QEMU_DT_BASE
$(eval $(call add_define,ARM_PRELOADED_DTB_BASE))

# Do not enable SVE
ENABLE_SVE_FOR_NS	:= 0
