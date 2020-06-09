#
# Copyright (c) 2018-2020, ARM Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

CSS_USE_SCMI_SDS_DRIVER	:=	1

CSS_SGM_BASE		:=	plat/arm/css/sgm

PLAT_INCLUDES		:=	-I${CSS_SGM_BASE}/include

PLAT_BL_COMMON_SOURCES	:=	${CSS_SGM_BASE}/sgm_mmap_config.c	\
				${CSS_SGM_BASE}/aarch64/css_sgm_helpers.S

SECURITY_SOURCES	:=	drivers/arm/tzc/tzc_dmc500.c		\
				plat/arm/common/arm_tzc_dmc500.c	\
				${CSS_SGM_BASE}/sgm_security.c

SGM_CPU_SOURCES		:=	lib/cpus/aarch64/cortex_a55.S		\
				lib/cpus/aarch64/cortex_a75.S

INTERCONNECT_SOURCES	:=	${CSS_SGM_BASE}/sgm_interconnect.c

# GIC-600 configuration
GICV3_SUPPORT_GIC600	:=	1

# Include GICv3 driver files
include drivers/arm/gic/v3/gicv3.mk

SGM_GIC_SOURCES		:=	${GICV3_SOURCES}			\
				plat/common/plat_gicv3.c		\
				plat/arm/common/arm_gicv3.c

BL1_SOURCES		+=	$(SGM_CPU_SOURCES)			\
				${INTERCONNECT_SOURCES}			\
				${CSS_SGM_BASE}/sgm_bl1_setup.c		\
				${CSS_SGM_BASE}/sgm_plat_config.c	\
				drivers/arm/sp805/sp805.c

BL2_SOURCES		+=	${SECURITY_SOURCES}			\
				${CSS_SGM_BASE}/sgm_plat_config.c

BL2U_SOURCES		+=	${SECURITY_SOURCES}

BL31_SOURCES		+=	$(SGM_CPU_SOURCES)			\
				${INTERCONNECT_SOURCES}			\
				${SECURITY_SOURCES}			\
				${SGM_GIC_SOURCES}			\
				${CSS_SGM_BASE}/sgm_topology.c		\
				${CSS_SGM_BASE}/sgm_bl31_setup.c	\
				${CSS_SGM_BASE}/sgm_plat_config.c

ifneq (${RESET_TO_BL31},0)
  $(error "Using BL31 as the reset vector is not supported on ${PLAT} platform. \
  Please set RESET_TO_BL31 to 0.")
endif

# sgm uses CCI-500 as Cache Coherent Interconnect
ARM_CCI_PRODUCT_ID	:=	500

# System coherency is managed in hardware
HW_ASSISTED_COHERENCY	:=	1

# When building for systems with hardware-assisted coherency, there's no need to
# use USE_COHERENT_MEM. Require that USE_COHERENT_MEM must be set to 0 too.
USE_COHERENT_MEM	:=	0

override ARM_PLAT_MT	:=	1

$(eval $(call add_define,SGM_PLAT))

include plat/arm/common/arm_common.mk
include plat/arm/board/common/board_common.mk
include plat/arm/css/common/css_common.mk
include plat/arm/soc/common/soc_css.mk
