#
# Copyright (c) 2023, STMicroelectronics - All Rights Reserved
#
# SPDX-License-Identifier: BSD-3-Clause
#

include plat/st/common/common.mk

CRASH_REPORTING			:=	1
ENABLE_PIE			:=	1
PROGRAMMABLE_RESET_ADDRESS	:=	1

# Default Device tree
DTB_FILE_NAME			?=	stm32mp257f-ev1.dtb

STM32MP25			:=	1

# STM32 image header version v2.2
STM32_HEADER_VERSION_MAJOR	:=	2
STM32_HEADER_VERSION_MINOR	:=	2

# Number of TF-A copies in the device
STM32_TF_A_COPIES		:=	2

# PLAT_PARTITION_MAX_ENTRIES must take care of STM32_TF-A_COPIES and other partitions
# such as metadata (2) and fsbl-m (2) to find all the FIP partitions (default is 2).
PLAT_PARTITION_MAX_ENTRIES	:=	$(shell echo $$(($(STM32_TF_A_COPIES) + 6)))

# Device tree
BL2_DTSI			:=	stm32mp25-bl2.dtsi
FDT_SOURCES			:=	$(addprefix ${BUILD_PLAT}/fdts/, $(patsubst %.dtb,%-bl2.dts,$(DTB_FILE_NAME)))

# Macros and rules to build TF binary
STM32_TF_STM32			:=	$(addprefix ${BUILD_PLAT}/tf-a-, $(patsubst %.dtb,%.stm32,$(DTB_FILE_NAME)))
STM32_LD_FILE			:=	plat/st/stm32mp2/${ARCH}/stm32mp2.ld.S
STM32_BINARY_MAPPING		:=	plat/st/stm32mp2/${ARCH}/stm32mp2.S

# STM32MP2x is based on Cortex-A35, which is Armv8.0, and does not support BTI
# Disable mbranch-protection to avoid adding useless code
TF_CFLAGS			+=	-mbranch-protection=none

# Include paths and source files
PLAT_INCLUDES			+=	-Iplat/st/stm32mp2/include/

PLAT_BL_COMMON_SOURCES		+=	lib/cpus/${ARCH}/cortex_a35.S
PLAT_BL_COMMON_SOURCES		+=	drivers/st/uart/${ARCH}/stm32_console.S
PLAT_BL_COMMON_SOURCES		+=	plat/st/stm32mp2/${ARCH}/stm32mp2_helper.S

BL2_SOURCES			+=	plat/st/stm32mp2/plat_bl2_mem_params_desc.c
BL2_SOURCES			+=	plat/st/stm32mp2/bl2_plat_setup.c

include plat/st/common/common_rules.mk
