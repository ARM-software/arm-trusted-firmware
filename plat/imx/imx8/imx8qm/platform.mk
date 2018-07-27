#
# Copyright (c) 2015-2018, ARM Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

PLAT_INCLUDES		:=	-Iplat/imx/imx8/imx8qm/include		\
				-Iplat/imx/imx8/common/include		\

IMX_GIC_SOURCES	:=		drivers/arm/gic/v3/gicv3_helpers.c	\
				drivers/arm/gic/v3/arm_gicv3_common.c   \
				drivers/arm/gic/v3/gic500.c             \
				drivers/arm/gic/v3/gicv3_main.c		\
				drivers/arm/gic/common/gic_common.c	\
				plat/common/plat_gicv3.c		\
				plat/common/plat_psci_common.c		\
				plat/imx/imx8/common/plat_imx8_gic.c

BL31_SOURCES		+=	plat/imx/imx8/common/lpuart_console.S	\
				plat/imx/imx8/common/imx8_helpers.S		\
				plat/imx/imx8/imx8qm/imx8qm_bl31_setup.c	\
				plat/imx/imx8/imx8qm/imx8qm_psci.c		\
				plat/imx/imx8/common/imx8_topology.c		\
				plat/imx/imx8/common/imx8_psci.c		\
				lib/xlat_tables/aarch64/xlat_tables.c		\
				lib/xlat_tables/xlat_tables_common.c		\
				lib/cpus/aarch64/cortex_a53.S			\
				lib/cpus/aarch64/cortex_a72.S			\
				drivers/console/aarch64/console.S		\
				drivers/arm/cci/cci.c				\
				${IMX_GIC_SOURCES}				\

include plat/imx/imx8/common/sci/sci_api.mk

ENABLE_PLAT_COMPAT	:=	0
USE_COHERENT_MEM	:=	1
RESET_TO_BL31		:=	1
ARM_GIC_ARCH		:=	3
A53_DISABLE_NON_TEMPORAL_HINT := 0
MULTI_CONSOLE_API	:=	1
ERRATA_A72_859971	:=	1
