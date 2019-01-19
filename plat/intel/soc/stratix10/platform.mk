#
# Copyright (c) 2019, ARM Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#


LOAD_IMAGE_V2 := 1

PLAT_INCLUDES		:=	\
			-Iinclude/plat/arm/common/			\
			-Iinclude/drivers/synopsys/			\
                        -Iinclude/plat/common/          		\
			-Iinclude/plat/arm/common/aarch64/		\
			-Iinclude/common/tbbr				\
			-Iinclude/common/				\
			-Iinclude/drivers/partition			\
			-Iplat/intel/soc/stratix10/			\
			-Iplat/intel/soc/stratix10/include/		\

PLAT_BL_COMMON_SOURCES	:=	\
			lib/xlat_tables/xlat_tables_common.c 		\
			lib/xlat_tables/aarch64/xlat_tables.c 		\
			drivers/arm/gic/common/gic_common.c		\
			drivers/arm/gic/v2/gicv2_main.c			\
			drivers/arm/gic/v2/gicv2_helpers.c		\
			plat/arm/common/arm_gicv2.c			\
			drivers/delay_timer/delay_timer.c		\
			drivers/delay_timer/generic_delay_timer.c  	\
			drivers/console/aarch64/console.S		\
			drivers/ti/uart/aarch64/16550_console.S		\
			plat/common/plat_gicv2.c			\
			plat/intel/soc/stratix10/aarch64/platform_common.c \
			plat/intel/soc/stratix10/aarch64/plat_helpers.S \

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
		drivers/io/io_memmap.c					\
		plat/intel/soc/stratix10/bl2_plat_setup.c		\
		plat/intel/soc/stratix10/plat_storage.c			\
                plat/intel/soc/stratix10/bl2_plat_mem_params_desc.c	\
		plat/intel/soc/stratix10/soc/s10_reset_manager.c	\
		plat/intel/soc/stratix10/soc/s10_handoff.c		\
		plat/intel/soc/stratix10/soc/s10_clock_manager.c	\
		plat/intel/soc/stratix10/soc/s10_pinmux.c		\
		plat/intel/soc/stratix10/soc/s10_memory_controller.c	\
		plat/intel/soc/stratix10/plat_topology.c		\
		plat/intel/soc/stratix10/plat_delay_timer.c		\
		lib/cpus/aarch64/cortex_a53.S				\
		plat/intel/soc/stratix10/stratix10_image_load.c		\
		plat/intel/soc/stratix10/soc/s10_system_manager.c	\
                common/desc_image_load.c

PROGRAMMABLE_RESET_ADDRESS	:=	0
BL2_AT_EL3 := 1
DISABLE_PEDANTIC		:= 1
ENABLE_PLAT_COMPAT := 0

TF_CFLAGS += -O0
