#
# Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

CONSOLE_BASE			:=	PL011_UART3_BASE
CRASH_CONSOLE_BASE		:=	PL011_UART3_BASE
PLAT_PARTITION_MAX_ENTRIES	:=	12
PLAT_PL061_MAX_GPIOS		:=	160
COLD_BOOT_SINGLE_CPU		:=	1
PROGRAMMABLE_RESET_ADDRESS	:=	1

# Process flags
$(eval $(call add_define,CONSOLE_BASE))
$(eval $(call add_define,CRASH_CONSOLE_BASE))
$(eval $(call add_define,PLAT_PL061_MAX_GPIOS))
$(eval $(call add_define,PLAT_PARTITION_MAX_ENTRIES))
$(eval $(call FIP_ADD_IMG,SCP_BL2,--scp-fw))

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

BL2_SOURCES		+=	drivers/arm/sp804/sp804_delay_timer.c	\
				drivers/delay_timer/delay_timer.c	\
				drivers/io/io_block.c			\
				drivers/io/io_fip.c			\
				drivers/io/io_storage.c			\
				drivers/emmc/emmc.c			\
				drivers/synopsys/emmc/dw_mmc.c		\
				plat/hisilicon/hikey/aarch64/hikey_helpers.S \
				plat/hisilicon/hikey/hikey_bl2_setup.c	\
				plat/hisilicon/hikey/hikey_ddr.c	\
				plat/hisilicon/hikey/hikey_io_storage.c	\
				plat/hisilicon/hikey/hisi_dvfs.c	\
				plat/hisilicon/hikey/hisi_mcu.c

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

