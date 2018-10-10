#
# Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

CSS_USE_SCMI_SDS_DRIVER		:=	1

CSS_ENT_BASE			:=	plat/arm/css/sgi

RAS_EXTENSION			:=	0

ENABLE_SPM			:=	0

SDEI_SUPPORT			:=	0

EL3_EXCEPTION_HANDLING		:=	0

HANDLE_EA_EL3_FIRST		:=	0

INTERCONNECT_SOURCES	:=	${CSS_ENT_BASE}/sgi_interconnect.c

PLAT_INCLUDES		+=	-I${CSS_ENT_BASE}/include

ENT_CPU_SOURCES		:=	lib/cpus/aarch64/cortex_a75.S

ENT_GIC_SOURCES		:=	drivers/arm/gic/common/gic_common.c	\
				drivers/arm/gic/v3/gicv3_main.c		\
				drivers/arm/gic/v3/gicv3_helpers.c	\
				plat/common/plat_gicv3.c		\
				plat/arm/common/arm_gicv3.c		\
				drivers/arm/gic/v3/gic600.c


PLAT_BL_COMMON_SOURCES	+=	${CSS_ENT_BASE}/sgi_plat.c	\
				${CSS_ENT_BASE}/aarch64/sgi_helper.S

BL1_SOURCES		+=	${INTERCONNECT_SOURCES}			\
				${ENT_CPU_SOURCES}			\
				${CSS_ENT_BASE}/sgi_bl1_setup.c	\
				${CSS_ENT_BASE}/sgi_plat_config.c

BL2_SOURCES		+=	${CSS_ENT_BASE}/sgi_security.c		\
				${CSS_ENT_BASE}/sgi_image_load.c

BL31_SOURCES		+=	${ENT_CPU_SOURCES}			\
				${INTERCONNECT_SOURCES}			\
				${ENT_GIC_SOURCES}			\
				${CSS_ENT_BASE}/sgi_bl31_setup.c	\
				${CSS_ENT_BASE}/sgi_topology.c	\
				${CSS_ENT_BASE}/sgi_plat_config.c

ifeq (${RAS_EXTENSION},1)
BL31_SOURCES		+=	${CSS_ENT_BASE}/sgi_ras.c
endif

# Add the FDT_SOURCES and options for Dynamic Config
FDT_SOURCES		+=	${CSS_ENT_BASE}/fdts/${PLAT}_tb_fw_config.dts
TB_FW_CONFIG		:=	${BUILD_PLAT}/fdts/${PLAT}_tb_fw_config.dtb

# Add the TB_FW_CONFIG to FIP and specify the same to certtool
$(eval $(call TOOL_ADD_PAYLOAD,${TB_FW_CONFIG},--tb-fw-config))

FDT_SOURCES		+=	${CSS_ENT_BASE}/fdts/${PLAT}.dts
HW_CONFIG		:=	${BUILD_PLAT}/fdts/${PLAT}.dtb

# Add the HW_CONFIG to FIP and specify the same to certtool
$(eval $(call TOOL_ADD_PAYLOAD,${HW_CONFIG},--hw-config))

ifneq (${RESET_TO_BL31},0)
  $(error "Using BL31 as the reset vector is not supported on ${PLATFORM} platform. \
  Please set RESET_TO_BL31 to 0.")
endif

$(eval $(call add_define,SGI_PLAT))

override CSS_LOAD_SCP_IMAGES	:=	0
override NEED_BL2U		:=	no
override ARM_BL31_IN_DRAM	:=	1
override ARM_PLAT_MT		:=	1

# System coherency is managed in hardware
HW_ASSISTED_COHERENCY	:=	1

# When building for systems with hardware-assisted coherency, there's no need to
# use USE_COHERENT_MEM. Require that USE_COHERENT_MEM must be set to 0 too.
USE_COHERENT_MEM	:=	0

include plat/arm/common/arm_common.mk
include plat/arm/css/common/css_common.mk
include plat/arm/soc/common/soc_css.mk
include plat/arm/board/common/board_common.mk
