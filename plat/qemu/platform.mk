#
# Copyright (c) 2013-2017, ARM Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

include lib/libfdt/libfdt.mk

PLAT_INCLUDES		:=	-Iinclude/plat/arm/common/		\
				-Iinclude/plat/arm/common/aarch64/	\
				-Iplat/qemu/include			\
				-Iinclude/common/tbbr


PLAT_BL_COMMON_SOURCES	:=	plat/qemu/qemu_common.c			\
				drivers/arm/pl011/aarch64/pl011_console.S \
				lib/xlat_tables/xlat_tables_common.c	\
				lib/xlat_tables/aarch64/xlat_tables.c

BL1_SOURCES		+=	drivers/io/io_semihosting.c		\
				drivers/io/io_storage.c			\
				drivers/io/io_fip.c			\
				drivers/io/io_memmap.c			\
				lib/semihosting/semihosting.c		\
				lib/semihosting/aarch64/semihosting_call.S \
				plat/qemu/qemu_io_storage.c		\
				lib/cpus/aarch64/aem_generic.S		\
				lib/cpus/aarch64/cortex_a53.S		\
				lib/cpus/aarch64/cortex_a57.S		\
				plat/qemu/aarch64/plat_helpers.S	\
				plat/qemu/qemu_bl1_setup.c

BL2_SOURCES		+=	drivers/io/io_semihosting.c		\
				drivers/io/io_storage.c			\
				drivers/io/io_fip.c			\
				drivers/io/io_memmap.c			\
				lib/semihosting/semihosting.c		\
				lib/semihosting/aarch64/semihosting_call.S\
				plat/qemu/qemu_io_storage.c		\
				plat/qemu/aarch64/plat_helpers.S	\
				plat/qemu/qemu_bl2_setup.c		\
				plat/qemu/dt.c				\
				$(LIBFDT_SRCS)

BL31_SOURCES		+=	lib/cpus/aarch64/aem_generic.S		\
				lib/cpus/aarch64/cortex_a53.S		\
				lib/cpus/aarch64/cortex_a57.S		\
				drivers/arm/gic/v2/gicv2_helpers.c	\
				drivers/arm/gic/v2/gicv2_main.c		\
				drivers/arm/gic/common/gic_common.c	\
				plat/common/aarch64/plat_psci_common.c	\
				plat/qemu/qemu_pm.c			\
				plat/qemu/topology.c			\
				plat/qemu/aarch64/plat_helpers.S	\
				plat/qemu/qemu_bl31_setup.c		\
				plat/qemu/qemu_gic.c

# Disable the PSCI platform compatibility layer
ENABLE_PLAT_COMPAT	:= 	0

BL32_RAM_LOCATION	:=	tdram
ifeq (${BL32_RAM_LOCATION}, tsram)
  BL32_RAM_LOCATION_ID = SEC_SRAM_ID
else ifeq (${BL32_RAM_LOCATION}, tdram)
  BL32_RAM_LOCATION_ID = SEC_DRAM_ID
else
  $(error "Unsupported BL32_RAM_LOCATION value")
endif

# Process flags
$(eval $(call add_define,BL32_RAM_LOCATION_ID))
