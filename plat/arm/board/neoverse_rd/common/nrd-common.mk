#
# Copyright (c) 2018-2024, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

CSS_USE_SCMI_SDS_DRIVER		:=	1

NRD_COMMON_BASE			:=	plat/arm/board/neoverse_rd/common

ENABLE_FEAT_RAS			:=	1

SDEI_SUPPORT			:=	0

EL3_EXCEPTION_HANDLING		:=	0

HANDLE_EA_EL3_FIRST_NS		:=	0

NRD_CHIP_COUNT		:=	1

NRD_PLATFORM_VARIANT	:=	0

# Do not enable SVE
ENABLE_SVE_FOR_NS		:=	0

CTX_INCLUDE_FPREGS		:=	1

INTERCONNECT_SOURCES	:=	${NRD_COMMON_BASE}/nrd_interconnect.c

PLAT_INCLUDES		+=	-I${NRD_COMMON_BASE}/include

# GIC-600 configuration
GICV3_SUPPORT_GIC600	:=	1

# Include GICv3 driver files
include drivers/arm/gic/v3/gicv3.mk

ENT_GIC_SOURCES		:=	${GICV3_SOURCES}		\
				plat/common/plat_gicv3.c	\
				plat/arm/common/arm_gicv3.c

PLAT_BL_COMMON_SOURCES	+=	${NRD_COMMON_BASE}/arch/aarch64/nrd_helper.S

BL1_SOURCES		+=	${INTERCONNECT_SOURCES}			\
				${NRD_COMMON_BASE}/nrd_bl1_setup.c	\
				drivers/arm/sbsa/sbsa.c

BL2_SOURCES		+=	${NRD_COMMON_BASE}/nrd_image_load.c	\
				drivers/arm/css/sds/sds.c

BL31_SOURCES		+=	${INTERCONNECT_SOURCES}			\
				${ENT_GIC_SOURCES}			\
				${NRD_COMMON_BASE}/nrd_bl31_setup.c	\
				${NRD_COMMON_BASE}/nrd_topology.c	\
				drivers/delay_timer/generic_delay_timer.c

ifneq (${RESET_TO_BL31},0)
  $(error "Using BL31 as the reset vector is not supported on ${PLAT} platform. \
  Please set RESET_TO_BL31 to 0.")
endif

$(eval $(call add_define,NRD_CHIP_COUNT))

$(eval $(call add_define,NRD_PLATFORM_VARIANT))

override CSS_LOAD_SCP_IMAGES	:=	0
override NEED_BL2U		:=	no
override ARM_PLAT_MT		:=	1
override PSCI_EXTENDED_STATE_ID	:=	1
override ARM_RECOM_STATE_ID_ENC	:=	1

# System coherency is managed in hardware
HW_ASSISTED_COHERENCY	:=	1

# When building for systems with hardware-assisted coherency, there's no need to
# use USE_COHERENT_MEM. Require that USE_COHERENT_MEM must be set to 0 too.
USE_COHERENT_MEM	:=	0

include plat/arm/common/arm_common.mk
include plat/arm/css/common/css_common.mk
include plat/arm/board/common/board_common.mk
