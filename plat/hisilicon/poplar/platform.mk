#
# Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

# On Poplar, the TSP can execute from TZC secure area in DRAM.
POPLAR_TSP_RAM_LOCATION	:=	dram
ifeq (${POPLAR_TSP_RAM_LOCATION}, dram)
  POPLAR_TSP_RAM_LOCATION_ID = POPLAR_DRAM_ID
else ifeq (${HIKEY960_TSP_RAM_LOCATION}, sram)
  POPLAR_TSP_RAM_LOCATION_ID := POPLAR_SRAM_ID
else
  $(error "Currently unsupported POPLAR_TSP_RAM_LOCATION value")
endif
$(eval $(call add_define,POPLAR_TSP_RAM_LOCATION_ID))

POPLAR_RECOVERY		:= 0
$(eval $(call add_define,POPLAR_RECOVERY))

NEED_BL33			:= yes

COLD_BOOT_SINGLE_CPU		:= 1
PROGRAMMABLE_RESET_ADDRESS	:= 1
CTX_INCLUDE_FPREGS		:= 1
ENABLE_PLAT_COMPAT		:= 0
ERRATA_A53_855873		:= 1
ERRATA_A53_835769		:= 1
ERRATA_A53_843419		:= 1
ENABLE_SVE_FOR_NS		:= 0

ARM_GIC_ARCH			:= 2
$(eval $(call add_define,ARM_GIC_ARCH))

PLAT_PL061_MAX_GPIOS 		:= 104
$(eval $(call add_define,PLAT_PL061_MAX_GPIOS))

PLAT_INCLUDES	:=	-Iplat/hisilicon/poplar/include		\
			-Iinclude/plat/arm/common/		\
			-Iplat/hisilicon/poplar			\
			-Iinclude/common/tbbr			\
			-Iinclude/drivers/synopsys		\
			-Iinclude/drivers/io

PLAT_BL_COMMON_SOURCES	:=						\
		lib/aarch64/xlat_tables.c				\
		drivers/delay_timer/generic_delay_timer.c		\
		drivers/arm/gic/common/gic_common.c			\
		drivers/arm/gic/v2/gicv2_helpers.c			\
		drivers/delay_timer/delay_timer.c			\
		drivers/arm/pl011/pl011_console.S			\
		drivers/arm/gic/v2/gicv2_main.c				\
		plat/arm/common/aarch64/arm_helpers.S			\
		plat/arm/common/arm_gicv2.c				\
		plat/common/plat_gicv2.c				\
		plat/hisilicon/poplar/aarch64/platform_common.c

BL1_SOURCES	+=							\
		lib/cpus/aarch64/cortex_a53.S				\
		drivers/arm/pl061/pl061_gpio.c				\
		drivers/emmc/emmc.c					\
		drivers/synopsys/emmc/dw_mmc.c				\
		drivers/io/io_storage.c					\
		drivers/io/io_block.c					\
		drivers/gpio/gpio.c					\
		drivers/io/io_fip.c					\
		drivers/io/io_memmap.c					\
		plat/hisilicon/poplar/bl1_plat_setup.c			\
		plat/hisilicon/poplar/plat_storage.c			\


BL2_SOURCES	+=      						\
		drivers/arm/pl061/pl061_gpio.c				\
		drivers/emmc/emmc.c					\
		drivers/synopsys/emmc/dw_mmc.c				\
		drivers/io/io_storage.c					\
		drivers/io/io_block.c					\
		drivers/io/io_fip.c					\
		drivers/gpio/gpio.c					\
		drivers/io/io_memmap.c					\
		plat/hisilicon/poplar/bl2_plat_setup.c			\
		plat/hisilicon/poplar/plat_storage.c


BL31_SOURCES	+=							\
		lib/cpus/aarch64/aem_generic.S				\
		lib/cpus/aarch64/cortex_a53.S				\
		plat/common/aarch64/plat_psci_common.c			\
		plat/hisilicon/poplar/bl31_plat_setup.c			\
		plat/hisilicon/poplar/plat_topology.c			\
		plat/hisilicon/poplar/plat_pm.c
