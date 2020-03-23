#
# Copyright (c) 2015-2020, ARM Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

PLAT_INCLUDES		:=	-Iplat/imx/imx8qm/include		\
				-Iplat/imx/common/include		\

# Include GICv3 driver files
include drivers/arm/gic/v3/gicv3.mk

IMX_GIC_SOURCES	:=		${GICV3_SOURCES}			\
				plat/common/plat_gicv3.c		\
				plat/common/plat_psci_common.c		\
				plat/imx/common/plat_imx8_gic.c

BL31_SOURCES		+=	plat/imx/common/lpuart_console.S	\
				plat/imx/common/imx8_helpers.S		\
				plat/imx/imx8qm/imx8qm_bl31_setup.c	\
				plat/imx/imx8qm/imx8qm_psci.c		\
				plat/imx/common/imx8_topology.c		\
				plat/imx/common/imx8_psci.c		\
				plat/imx/common/imx_sip_svc.c		\
				plat/imx/common/imx_sip_handler.c	\
				lib/xlat_tables/aarch64/xlat_tables.c		\
				lib/xlat_tables/xlat_tables_common.c		\
				lib/cpus/aarch64/cortex_a53.S			\
				lib/cpus/aarch64/cortex_a72.S			\
				drivers/arm/cci/cci.c				\
				${IMX_GIC_SOURCES}				\

include plat/imx/common/sci/sci_api.mk

USE_COHERENT_MEM	:=	1
RESET_TO_BL31		:=	1
A53_DISABLE_NON_TEMPORAL_HINT := 0
ERRATA_A72_859971	:=	1

ERRATA_A53_835769	:=	1
ERRATA_A53_843419	:=	1
ERRATA_A53_855873	:=	1

IMX_DEBUG_UART		?= 	0
$(eval $(call add_define,IMX_USE_UART${IMX_DEBUG_UART}))

DEBUG_CONSOLE		?= 	0
$(eval $(call add_define,DEBUG_CONSOLE))
