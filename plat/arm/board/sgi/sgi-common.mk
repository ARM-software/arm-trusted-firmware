#
# Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

ENABLE_PLAT_COMPAT		:=	0

CSS_ENT_BASE			:=	plat/arm/board/sgi

INTERCONNECT_SOURCES	:=	${CSS_ENT_BASE}/sgi_interconnect.c

PLAT_INCLUDES		+=	-I${CSS_ENT_BASE}/include

ENT_CPU_SOURCES		:=	lib/cpus/aarch64/cortex_a75.S

ENT_GIC_SOURCES		:=	drivers/arm/gic/common/gic_common.c	\
				drivers/arm/gic/v3/gicv3_main.c		\
				drivers/arm/gic/v3/gicv3_helpers.c	\
				plat/common/plat_gicv3.c		\
				plat/arm/common/arm_gicv3.c		\
				${CSS_ENT_BASE}/sgi_gic_config.c	\
				drivers/arm/gic/v3/gic600.c


PLAT_BL_COMMON_SOURCES	+=	${CSS_ENT_BASE}/sgi_plat.c	\
				${CSS_ENT_BASE}/aarch64/sgi_helper.S

BL1_SOURCES		+=	${INTERCONNECT_SOURCES}			\
				${ENT_CPU_SOURCES}			\
				${CSS_ENT_BASE}/sgi_bl1_setup.c	\
				${CSS_ENT_BASE}/sgi_plat_config.c

BL2_SOURCES		+=	${CSS_ENT_BASE}/sgi_security.c

BL31_SOURCES		+=	${ENT_CPU_SOURCES}			\
				${INTERCONNECT_SOURCES}			\
				${ENT_GIC_SOURCES}			\
				${CSS_ENT_BASE}/sgi_bl31_setup.c	\
				${CSS_ENT_BASE}/sgi_topology.c	\
				${CSS_ENT_BASE}/sgi_plat_config.c

$(eval $(call add_define,SGI_PLAT))

override CSS_LOAD_SCP_IMAGES	:=	0
override NEED_BL2U		:=	no
override ARM_BL31_IN_DRAM	:=	1

# System coherency is managed in hardware
HW_ASSISTED_COHERENCY	:=	1

# When building for systems with hardware-assisted coherency, there's no need to
# use USE_COHERENT_MEM. Require that USE_COHERENT_MEM must be set to 0 too.
USE_COHERENT_MEM	:=	0

include plat/arm/common/arm_common.mk
include plat/arm/css/common/css_common.mk
include plat/arm/soc/common/soc_css.mk
include plat/arm/board/common/board_common.mk
