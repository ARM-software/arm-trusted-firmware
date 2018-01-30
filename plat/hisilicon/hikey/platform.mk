#
# Copyright (c) 2017-2018, ARM Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

# Enable version2 of image loading
LOAD_IMAGE_V2	:=	1

# On Hikey, the TSP can execute from TZC secure area in DRAM (default)
# or SRAM.
HIKEY_TSP_RAM_LOCATION	:=	dram
ifeq (${HIKEY_TSP_RAM_LOCATION}, dram)
  HIKEY_TSP_RAM_LOCATION_ID = HIKEY_DRAM_ID
else ifeq (${HIKEY_TSP_RAM_LOCATION}, sram)
  HIKEY_TSP_RAM_LOCATION_ID := HIKEY_SRAM_ID
else
  $(error "Currently unsupported HIKEY_TSP_RAM_LOCATION value")
endif

CONSOLE_BASE			:=	PL011_UART3_BASE
CRASH_CONSOLE_BASE		:=	PL011_UART3_BASE
PLAT_PARTITION_MAX_ENTRIES	:=	12
PLAT_PL061_MAX_GPIOS		:=	160
COLD_BOOT_SINGLE_CPU		:=	1
PROGRAMMABLE_RESET_ADDRESS	:=	1
ENABLE_SVE_FOR_NS		:=	0

# Process flags
$(eval $(call add_define,HIKEY_TSP_RAM_LOCATION_ID))
$(eval $(call add_define,CONSOLE_BASE))
$(eval $(call add_define,CRASH_CONSOLE_BASE))
$(eval $(call add_define,PLAT_PL061_MAX_GPIOS))
$(eval $(call add_define,PLAT_PARTITION_MAX_ENTRIES))

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
				-Iinclude/drivers/synopsys		\
				-Iplat/hisilicon/hikey/include

PLAT_BL_COMMON_SOURCES	:=	drivers/arm/pl011/pl011_console.S	\
				lib/aarch64/xlat_tables.c		\
				plat/hisilicon/hikey/aarch64/hikey_common.c

BL1_SOURCES		+=	bl1/tbbr/tbbr_img_desc.c		\
				drivers/arm/pl061/pl061_gpio.c		\
				drivers/arm/sp804/sp804_delay_timer.c	\
				drivers/delay_timer/delay_timer.c	\
				drivers/gpio/gpio.c			\
				drivers/io/io_block.c			\
				drivers/io/io_fip.c			\
				drivers/io/io_storage.c			\
				drivers/emmc/emmc.c			\
				drivers/synopsys/emmc/dw_mmc.c		\
				lib/cpus/aarch64/cortex_a53.S		\
				plat/hisilicon/hikey/aarch64/hikey_helpers.S \
				plat/hisilicon/hikey/hikey_bl1_setup.c	\
				plat/hisilicon/hikey/hikey_io_storage.c

BL2_SOURCES		+=	common/desc_image_load.c		\
				drivers/arm/sp804/sp804_delay_timer.c	\
				drivers/delay_timer/delay_timer.c	\
				drivers/io/io_block.c			\
				drivers/io/io_fip.c			\
				drivers/io/io_storage.c			\
				drivers/emmc/emmc.c			\
				drivers/synopsys/emmc/dw_mmc.c		\
				plat/hisilicon/hikey/aarch64/hikey_helpers.S \
				plat/hisilicon/hikey/hikey_bl2_mem_params_desc.c \
				plat/hisilicon/hikey/hikey_bl2_setup.c	\
				plat/hisilicon/hikey/hikey_security.c   \
				plat/hisilicon/hikey/hikey_ddr.c	\
				plat/hisilicon/hikey/hikey_image_load.c \
				plat/hisilicon/hikey/hikey_io_storage.c	\
				plat/hisilicon/hikey/hisi_dvfs.c	\
				plat/hisilicon/hikey/hisi_mcu.c

ifeq (${SPD},opteed)
BL2_SOURCES		+=	lib/optee/optee_utils.c
endif

HIKEY_GIC_SOURCES	:=	drivers/arm/gic/common/gic_common.c	\
				drivers/arm/gic/v2/gicv2_main.c		\
				drivers/arm/gic/v2/gicv2_helpers.c	\
				plat/common/plat_gicv2.c

BL31_SOURCES		+=	drivers/arm/cci/cci.c			\
				drivers/arm/sp804/sp804_delay_timer.c	\
				drivers/delay_timer/delay_timer.c	\
				lib/cpus/aarch64/cortex_a53.S		\
				plat/common/aarch64/plat_psci_common.c	\
				plat/hisilicon/hikey/aarch64/hikey_helpers.S \
				plat/hisilicon/hikey/hikey_bl31_setup.c	\
				plat/hisilicon/hikey/hikey_pm.c		\
				plat/hisilicon/hikey/hikey_topology.c	\
				plat/hisilicon/hikey/hisi_ipc.c		\
				plat/hisilicon/hikey/hisi_pwrc.c	\
				plat/hisilicon/hikey/hisi_pwrc_sram.S	\
				${HIKEY_GIC_SOURCES}
ifeq (${ENABLE_PMF}, 1)
BL31_SOURCES		+=	plat/hisilicon/hikey/hisi_sip_svc.c			\
				lib/pmf/pmf_smc.c
endif

# Enable workarounds for selected Cortex-A53 errata.
ERRATA_A53_836870		:=	1
ERRATA_A53_843419		:=	1
ERRATA_A53_855873		:=	1

FIP_ALIGN			:=	512
