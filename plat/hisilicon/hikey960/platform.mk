#
# Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

CRASH_CONSOLE_BASE		:=	PL011_UART6_BASE
COLD_BOOT_SINGLE_CPU		:=	1
PROGRAMMABLE_RESET_ADDRESS	:=	1

# Process flags
$(eval $(call add_define,CRASH_CONSOLE_BASE))
$(eval $(call FIP_ADD_IMG,SCP_BL2,--scp-fw))

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
