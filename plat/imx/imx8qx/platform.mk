#
# Copyright (c) 2015-2018, ARM Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

PLAT_INCLUDES		:=	-Iplat/imx/imx8qx/include		\
				-Iplat/imx/common/include		\

IMX_GIC_SOURCES	:=		drivers/arm/gic/v3/gicv3_helpers.c	\
				drivers/arm/gic/v3/arm_gicv3_common.c	\
				drivers/arm/gic/v3/gic500.c		\
				drivers/arm/gic/v3/gicv3_main.c		\
				drivers/arm/gic/common/gic_common.c	\
				plat/common/plat_gicv3.c		\
				plat/imx/common/plat_imx8_gic.c

BL31_SOURCES		+=	plat/imx/common/lpuart_console.S	\
				plat/imx/common/imx8_helpers.S		\
				plat/imx/imx8qx/imx8qx_bl31_setup.c	\
				plat/imx/imx8qx/imx8qx_psci.c		\
				plat/imx/common/imx8_topology.c		\
				plat/imx/common/imx8_psci.c		\
				plat/imx/common/imx_sip_svc.c		\
				plat/imx/common/imx_sip_handler.c	\
				plat/common/plat_psci_common.c		\
				lib/xlat_tables/xlat_tables_common.c	\
				lib/xlat_tables/aarch64/xlat_tables.c	\
				lib/cpus/aarch64/cortex_a35.S		\
				${IMX_GIC_SOURCES}			\

include plat/imx/common/sci/sci_api.mk

USE_COHERENT_MEM	:=	1
RESET_TO_BL31		:=	1
