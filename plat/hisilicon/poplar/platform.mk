#
# Copyright (c) 2017-2018, ARM Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

# On Poplar, the TSP can execute from TZC secure area in DRAM.
POPLAR_TSP_RAM_LOCATION	?=	dram
ifeq (${POPLAR_TSP_RAM_LOCATION}, dram)
  POPLAR_TSP_RAM_LOCATION_ID = POPLAR_DRAM_ID
else ifeq (${POPLAR_TSP_RAM_LOCATION}, sram)
  POPLAR_TSP_RAM_LOCATION_ID = POPLAR_SRAM_ID
else
  $(error "Currently unsupported POPLAR_TSP_RAM_LOCATION value")
endif
$(eval $(call add_define,POPLAR_TSP_RAM_LOCATION_ID))

POPLAR_DRAM_SIZE ?= two_gig
ifeq (${POPLAR_DRAM_SIZE}, two_gig)
  POPLAR_DRAM_SIZE_ID = POPLAR_DRAM_SIZE_2G
else ifeq (${POPLAR_DRAM_SIZE}, one_gig)
  POPLAR_DRAM_SIZE_ID = POPLAR_DRAM_SIZE_1G
else
  $(error "Currently unsupported POPLAR_DRAM_SIZE value")
endif
$(eval $(call add_define,POPLAR_DRAM_SIZE_ID))

POPLAR_RECOVERY		:= 0
$(eval $(call add_define,POPLAR_RECOVERY))

# Add the build options to pack Trusted OS Extra1 and Trusted OS Extra2 images
# in the FIP if the platform requires.
ifneq ($(BL32_EXTRA1),)
$(eval $(call TOOL_ADD_IMG,BL32_EXTRA1,--tos-fw-extra1))
endif
ifneq ($(BL32_EXTRA2),)
$(eval $(call TOOL_ADD_IMG,BL32_EXTRA2,--tos-fw-extra2))
endif

NEED_BL33			:= yes

COLD_BOOT_SINGLE_CPU		:= 1
PROGRAMMABLE_RESET_ADDRESS	:= 1
CTX_INCLUDE_FPREGS		:= 1
ERRATA_A53_855873		:= 1
ERRATA_A53_835769		:= 1
ERRATA_A53_843419		:= 1
ENABLE_SVE_FOR_NS		:= 0
MULTI_CONSOLE_API		:= 1
WORKAROUND_CVE_2017_5715	:= 0

PLAT_PL061_MAX_GPIOS 		:= 104
$(eval $(call add_define,PLAT_PL061_MAX_GPIOS))

PLAT_INCLUDES	:=	-Iplat/hisilicon/poplar/include		\
			-Iplat/hisilicon/poplar

PLAT_BL_COMMON_SOURCES	:=						\
		lib/xlat_tables/aarch64/xlat_tables.c			\
		lib/xlat_tables/xlat_tables_common.c			\
		drivers/delay_timer/generic_delay_timer.c		\
		drivers/arm/gic/common/gic_common.c			\
		drivers/arm/gic/v2/gicv2_helpers.c			\
		drivers/delay_timer/delay_timer.c			\
		drivers/arm/pl011/aarch64/pl011_console.S		\
		drivers/arm/gic/v2/gicv2_main.c				\
		plat/common/plat_gicv2.c				\
		plat/hisilicon/poplar/aarch64/platform_common.c		\
		plat/hisilicon/poplar/aarch64/poplar_helpers.S		\
		plat/hisilicon/poplar/poplar_gicv2.c

BL1_SOURCES	+=							\
		lib/cpus/aarch64/cortex_a53.S				\
		drivers/arm/pl061/pl061_gpio.c				\
		drivers/mmc/mmc.c					\
		drivers/synopsys/emmc/dw_mmc.c				\
		drivers/io/io_storage.c					\
		drivers/io/io_block.c					\
		drivers/gpio/gpio.c					\
		drivers/io/io_fip.c					\
		drivers/io/io_memmap.c					\
		plat/hisilicon/poplar/bl1_plat_setup.c			\
		plat/hisilicon/poplar/plat_storage.c

BL2_SOURCES	+=      						\
		drivers/arm/pl061/pl061_gpio.c				\
		drivers/mmc/mmc.c					\
		drivers/synopsys/emmc/dw_mmc.c				\
		drivers/io/io_storage.c					\
		drivers/io/io_block.c					\
		drivers/io/io_fip.c					\
		drivers/gpio/gpio.c					\
		drivers/io/io_memmap.c					\
		plat/hisilicon/poplar/bl2_plat_setup.c			\
		plat/hisilicon/poplar/plat_storage.c

BL2_SOURCES	+=							\
		plat/hisilicon/poplar/bl2_plat_mem_params_desc.c	\
		plat/hisilicon/poplar/poplar_image_load.c		\
		common/desc_image_load.c

ifeq (${SPD},opteed)
BL2_SOURCES	+=							\
		lib/optee/optee_utils.c
endif

BL31_SOURCES	+=							\
		lib/cpus/aarch64/aem_generic.S				\
		lib/cpus/aarch64/cortex_a53.S				\
		plat/common/plat_psci_common.c			\
		plat/hisilicon/poplar/bl31_plat_setup.c			\
		plat/hisilicon/poplar/plat_topology.c			\
		plat/hisilicon/poplar/plat_pm.c
