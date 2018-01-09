#
# Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

# Enable version2 of image loading
LOAD_IMAGE_V2	:=	1

# On Hikey960, the TSP can execute from TZC secure area in DRAM.
HIKEY960_TSP_RAM_LOCATION	:=	dram
ifeq (${HIKEY960_TSP_RAM_LOCATION}, dram)
  HIKEY960_TSP_RAM_LOCATION_ID = HIKEY960_DRAM_ID
else ifeq (${HIKEY960_TSP_RAM_LOCATION}, sram)
  HIKEY960_TSP_RAM_LOCATION_ID := HIKEY960_SRAM_ID
else
  $(error "Currently unsupported HIKEY960_TSP_RAM_LOCATION value")
endif

CRASH_CONSOLE_BASE		:=	PL011_UART6_BASE
COLD_BOOT_SINGLE_CPU		:=	1
PROGRAMMABLE_RESET_ADDRESS	:=	1
ENABLE_SVE_FOR_NS		:=	0

# Process flags
$(eval $(call add_define,HIKEY960_TSP_RAM_LOCATION_ID))
$(eval $(call add_define,CRASH_CONSOLE_BASE))

# Add the build options to pack Trusted OS Extra1 and Trusted OS Extra2 images
# in the FIP if the platform requires.
ifneq ($(BL32_EXTRA1),)
$(eval $(call FIP_ADD_IMG,BL32_EXTRA1,--tos-fw-extra1))
endif
ifneq ($(BL32_EXTRA2),)
$(eval $(call FIP_ADD_IMG,BL32_EXTRA2,--tos-fw-extra2))
endif

ENABLE_PLAT_COMPAT	:=	0

USE_COHERENT_MEM	:=	1

PLAT_INCLUDES		:=	-Iinclude/common/tbbr			\
				-Iplat/hisilicon/hikey960/include

PLAT_BL_COMMON_SOURCES	:=	drivers/arm/pl011/pl011_console.S	\
				drivers/delay_timer/delay_timer.c	\
				drivers/delay_timer/generic_delay_timer.c \
				lib/aarch64/xlat_tables.c		\
				plat/hisilicon/hikey960/aarch64/hikey960_common.c \
				plat/hisilicon/hikey960/hikey960_boardid.c

HIKEY960_GIC_SOURCES	:=	drivers/arm/gic/common/gic_common.c	\
				drivers/arm/gic/v2/gicv2_main.c		\
				drivers/arm/gic/v2/gicv2_helpers.c	\
				plat/common/plat_gicv2.c

BL1_SOURCES		+=	bl1/tbbr/tbbr_img_desc.c		\
				drivers/io/io_block.c			\
				drivers/io/io_fip.c			\
				drivers/io/io_storage.c			\
				drivers/synopsys/ufs/dw_ufs.c		\
				drivers/ufs/ufs.c 			\
				lib/cpus/aarch64/cortex_a53.S		\
				plat/hisilicon/hikey960/aarch64/hikey960_helpers.S \
				plat/hisilicon/hikey960/hikey960_bl1_setup.c 	\
				plat/hisilicon/hikey960/hikey960_io_storage.c \
				${HIKEY960_GIC_SOURCES}

BL2_SOURCES		+=	drivers/io/io_block.c			\
				drivers/io/io_fip.c			\
				drivers/io/io_storage.c			\
				drivers/ufs/ufs.c			\
				plat/hisilicon/hikey960/hikey960_bl2_setup.c \
				plat/hisilicon/hikey960/hikey960_io_storage.c \
				plat/hisilicon/hikey960/hikey960_mcu_load.c

ifeq (${LOAD_IMAGE_V2},1)
BL2_SOURCES		+=	plat/hisilicon/hikey960/hikey960_bl2_mem_params_desc.c \
				plat/hisilicon/hikey960/hikey960_image_load.c \
				common/desc_image_load.c

ifeq (${SPD},opteed)
BL2_SOURCES		+=	lib/optee/optee_utils.c
endif
endif

BL31_SOURCES		+=	drivers/arm/cci/cci.c			\
				lib/cpus/aarch64/cortex_a53.S           \
				lib/cpus/aarch64/cortex_a72.S		\
				lib/cpus/aarch64/cortex_a73.S		\
				plat/common/aarch64/plat_psci_common.c  \
				plat/hisilicon/hikey960/aarch64/hikey960_helpers.S \
				plat/hisilicon/hikey960/hikey960_bl31_setup.c \
				plat/hisilicon/hikey960/hikey960_pm.c	\
				plat/hisilicon/hikey960/hikey960_topology.c \
				plat/hisilicon/hikey960/drivers/pwrc/hisi_pwrc.c \
				plat/hisilicon/hikey960/drivers/ipc/hisi_ipc.c \
				${HIKEY960_GIC_SOURCES}

# Enable workarounds for selected Cortex-A53 errata.
ERRATA_A53_836870		:=	1
ERRATA_A53_843419		:=	1
ERRATA_A53_855873		:=	1

FIP_ALIGN			:=	512
