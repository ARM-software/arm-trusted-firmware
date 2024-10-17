#
# Copyright (c) 2018-2023, ARM Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

# Translation tables library
include lib/xlat_tables_v2/xlat_tables.mk

PLAT_INCLUDES		:=	-Iplat/imx/common/include		\
				-Iplat/imx/imx8m/include		\
				-Iplat/imx/imx8m/imx8mq/include

# Include GICv3 driver files
include drivers/arm/gic/v3/gicv3.mk

IMX_DRAM_SOURCES	:=	plat/imx/imx8m/ddr/dram.c		\
				plat/imx/imx8m/ddr/clock.c		\
				plat/imx/imx8m/ddr/dram_retention.c	\
				plat/imx/imx8m/ddr/ddr4_dvfs.c		\
				plat/imx/imx8m/ddr/lpddr4_dvfs.c

IMX_GIC_SOURCES		:=	${GICV3_SOURCES}			\
				plat/common/plat_gicv3.c		\
				plat/common/plat_psci_common.c		\
				plat/imx/common/plat_imx8_gic.c

BL31_SOURCES		+=	plat/imx/common/imx8_helpers.S			\
				plat/imx/imx8m/imx8mq/imx8mq_bl31_setup.c	\
				plat/imx/imx8m/imx8mq/imx8mq_psci.c		\
				plat/imx/imx8m/gpc_common.c			\
				plat/imx/imx8m/imx_hab.c			\
				plat/imx/imx8m/imx_aipstz.c			\
				plat/imx/imx8m/imx8m_caam.c			\
				plat/imx/imx8m/imx8m_ccm.c			\
				plat/imx/imx8m/imx8m_psci_common.c		\
				plat/imx/imx8m/imx8mq/gpc.c			\
				plat/imx/common/imx8_topology.c			\
				plat/imx/common/imx_sip_handler.c		\
				plat/imx/common/imx_sip_svc.c			\
				plat/imx/common/imx_uart_console.S		\
				lib/cpus/aarch64/cortex_a53.S			\
				drivers/arm/tzc/tzc380.c			\
				drivers/delay_timer/delay_timer.c		\
				drivers/delay_timer/generic_delay_timer.c	\
				${XLAT_TABLES_LIB_SRCS}				\
				${IMX_GIC_SOURCES}

ENABLE_PIE		:=	1
USE_COHERENT_MEM	:=	1
RESET_TO_BL31		:=	1
A53_DISABLE_NON_TEMPORAL_HINT := 0
WARMBOOT_ENABLE_DCACHE_EARLY	:=	1

ERRATA_A53_835769	:=	1
ERRATA_A53_843419	:=	1
ERRATA_A53_855873	:=	1

IMX_DRAM_RETENTION	?=	0
$(eval $(call assert_boolean,IMX_DRAM_RETENTION))
$(eval $(call add_define,IMX_DRAM_RETENTION))

ifeq (${IMX_DRAM_RETENTION},1)
BL31_SOURCES		+=	${IMX_DRAM_SOURCES}
endif

SEPARATE_NOBITS_REGION  :=        1

ifneq (${PRELOADED_BL33_BASE},)
$(eval $(call add_define_val,PLAT_NS_IMAGE_OFFSET,${PRELOADED_BL33_BASE}))
endif

BL32_BASE		?=	0xfe000000
$(eval $(call add_define,BL32_BASE))

BL32_SIZE		?=	0x2000000
$(eval $(call add_define,BL32_SIZE))

IMX_BOOT_UART_BASE	?=	0x30860000
ifeq (${IMX_BOOT_UART_BASE},auto)
    override IMX_BOOT_UART_BASE	:=	0
endif
$(eval $(call add_define,IMX_BOOT_UART_BASE))

ifeq (${SPD},trusty)
	BL31_CFLAGS    +=      -DPLAT_XLAT_TABLES_DYNAMIC=1
endif
