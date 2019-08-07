#
# Copyright (c) 2019, ARM Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

PLAT_INCLUDES		:=	\
			-Iplat/intel/soc/stratix10/include/		\
			-Iplat/intel/soc/common/drivers/		\
			-Iplat/intel/soc/common/include/

PLAT_BL_COMMON_SOURCES	:=	\
			lib/xlat_tables/xlat_tables_common.c 		\
			lib/xlat_tables/aarch64/xlat_tables.c 		\
			drivers/arm/gic/common/gic_common.c		\
			drivers/arm/gic/v2/gicv2_main.c			\
			drivers/arm/gic/v2/gicv2_helpers.c		\
			plat/common/plat_gicv2.c			\
			drivers/delay_timer/delay_timer.c		\
			drivers/delay_timer/generic_delay_timer.c  	\
			drivers/ti/uart/aarch64/16550_console.S		\
			plat/intel/soc/common/aarch64/platform_common.c \
			plat/intel/soc/common/aarch64/plat_helpers.S

BL2_SOURCES     +=	\
		drivers/partition/partition.c				\
		drivers/partition/gpt.c					\
		drivers/arm/pl061/pl061_gpio.c				\
		drivers/mmc/mmc.c					\
		drivers/synopsys/emmc/dw_mmc.c				\
		drivers/io/io_storage.c					\
		drivers/io/io_block.c					\
		drivers/io/io_fip.c					\
		drivers/gpio/gpio.c					\
		drivers/intel/soc/stratix10/io/s10_memmap_qspi.c	\
		plat/intel/soc/stratix10/bl2_plat_setup.c		\
		plat/intel/soc/stratix10/plat_storage.c			\
                plat/intel/soc/common/bl2_plat_mem_params_desc.c	\
		plat/intel/soc/stratix10/soc/s10_reset_manager.c	\
		plat/intel/soc/stratix10/soc/s10_handoff.c		\
		plat/intel/soc/stratix10/soc/s10_clock_manager.c	\
		plat/intel/soc/stratix10/soc/s10_pinmux.c		\
		plat/intel/soc/stratix10/soc/s10_memory_controller.c	\
		plat/intel/soc/common/socfpga_delay_timer.c		\
		lib/cpus/aarch64/cortex_a53.S				\
		plat/intel/soc/common/socfpga_image_load.c		\
		plat/intel/soc/stratix10/soc/s10_system_manager.c	\
		common/desc_image_load.c				\
		plat/intel/soc/stratix10/soc/s10_mailbox.c		\
		plat/intel/soc/common/drivers/qspi/cadence_qspi.c	\
		plat/intel/soc/common/drivers/wdt/watchdog.c

BL31_SOURCES	+=	drivers/arm/cci/cci.c				\
		lib/cpus/aarch64/cortex_a53.S				\
			lib/cpus/aarch64/aem_generic.S			\
			lib/cpus/aarch64/cortex_a53.S			\
			plat/common/plat_psci_common.c			\
			plat/intel/soc/stratix10/plat_sip_svc.c		\
			plat/intel/soc/stratix10/bl31_plat_setup.c 	\
			plat/intel/soc/stratix10/plat_psci.c		\
			plat/intel/soc/common/socfpga_topology.c	\
			plat/intel/soc/common/socfpga_delay_timer.c	\
			plat/intel/soc/stratix10/soc/s10_reset_manager.c\
			plat/intel/soc/stratix10/soc/s10_pinmux.c	\
			plat/intel/soc/stratix10/soc/s10_clock_manager.c\
			plat/intel/soc/stratix10/soc/s10_handoff.c	\
			plat/intel/soc/stratix10/soc/s10_mailbox.c

PROGRAMMABLE_RESET_ADDRESS	:= 0
BL2_AT_EL3			:= 1
USE_COHERENT_MEM		:= 1
