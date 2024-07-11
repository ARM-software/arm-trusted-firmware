#
# Copyright (c) 2023-2024, STMicroelectronics - All Rights Reserved
#
# SPDX-License-Identifier: BSD-3-Clause
#

# Extra partitions used to find FIP, contains:
# metadata (2) and fsbl-m (2) and the FIP partitions (default is 2).
STM32_EXTRA_PARTS		:=	6

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

# Set load address for serial boot devices
DWL_BUFFER_BASE 	?=	0x87000000

# Device tree
BL2_DTSI			:=	stm32mp25-bl2.dtsi
FDT_SOURCES			:=	$(addprefix ${BUILD_PLAT}/fdts/, $(patsubst %.dtb,%-bl2.dts,$(DTB_FILE_NAME)))

# Macros and rules to build TF binary
STM32_TF_STM32			:=	$(addprefix ${BUILD_PLAT}/tf-a-, $(patsubst %.dtb,%.stm32,$(DTB_FILE_NAME)))
STM32_LD_FILE			:=	plat/st/stm32mp2/${ARCH}/stm32mp2.ld.S
STM32_BINARY_MAPPING		:=	plat/st/stm32mp2/${ARCH}/stm32mp2.S

$(eval $(call add_defines,\
	$(sort \
		DWL_BUFFER_BASE \
)))

# STM32MP2x is based on Cortex-A35, which is Armv8.0, and does not support BTI
# Disable mbranch-protection to avoid adding useless code
TF_CFLAGS			+=	-mbranch-protection=none

# Include paths and source files
PLAT_INCLUDES			+=	-Iplat/st/stm32mp2/include/

PLAT_BL_COMMON_SOURCES		+=	lib/cpus/${ARCH}/cortex_a35.S
PLAT_BL_COMMON_SOURCES		+=	drivers/st/uart/${ARCH}/stm32_console.S
PLAT_BL_COMMON_SOURCES		+=	plat/st/stm32mp2/${ARCH}/stm32mp2_helper.S

PLAT_BL_COMMON_SOURCES		+=	drivers/st/bsec/bsec3.c					\
					drivers/st/reset/stm32mp2_reset.c

PLAT_BL_COMMON_SOURCES		+=	drivers/st/clk/clk-stm32-core.c				\
					drivers/st/clk/clk-stm32mp2.c

BL2_SOURCES			+=	plat/st/stm32mp2/plat_bl2_mem_params_desc.c
BL2_SOURCES			+=	plat/st/stm32mp2/bl2_plat_setup.c

ifeq (${STM32MP_USB_PROGRAMMER},1)
BL2_SOURCES			+=	plat/st/stm32mp2/stm32mp2_usb_dfu.c
endif

include plat/st/common/common_rules.mk
