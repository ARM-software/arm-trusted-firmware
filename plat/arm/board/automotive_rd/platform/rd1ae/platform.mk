# Copyright (c) 2024, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

# RD1AE (Kronos) platform.
$(info Platform ${PLAT} is (kronos) specific.)

RD1AE_BASE		=	plat/arm/board/automotive_rd/platform/rd1ae

PLAT_INCLUDES		+=	-I${RD1AE_BASE}/include/

override ARM_PLAT_MT			:=	1
override ARM_RECOM_STATE_ID_ENC		:=	1
override CSS_LOAD_SCP_IMAGES		:=	0
override CTX_INCLUDE_AARCH32_REGS	:=	0
override ENABLE_SVE_FOR_NS		:=	1
override ENABLE_SVE_FOR_SWD		:=	1
override NEED_BL1			:=	0
override NEED_BL2U			:=	0
override NEED_BL31			:=	0
override PSCI_EXTENDED_STATE_ID		:=	1

ARM_ARCH_MAJOR				:=	9
ARM_ARCH_MINOR				:=	2
CSS_USE_SCMI_SDS_DRIVER			:=	1
ENABLE_FEAT_AMU				:=	1
ENABLE_FEAT_ECV				:=	1
ENABLE_FEAT_FGT				:=	1
ENABLE_FEAT_MTE2			:=	1
ENABLE_MPAM_FOR_LOWER_ELS		:=	1
HW_ASSISTED_COHERENCY			:=	1
RESET_TO_BL2				:=	1
SVE_VECTOR_LEN				:=	128
USE_COHERENT_MEM			:=	0

RD1AE_CPU_SOURCES	:=	lib/cpus/aarch64/neoverse_v3.S

PLAT_BL_COMMON_SOURCES	+=	${RD1AE_BASE}/rd1ae_plat.c	\
				${RD1AE_BASE}/include/rd1ae_helpers.S

BL2_SOURCES	+=	${RD1AE_CPU_SOURCES}	\
			${RD1AE_BASE}/rd1ae_err.c	\
			lib/utils/mem_region.c	\
			plat/arm/common/arm_nor_psci_mem_protect.c	\
			drivers/arm/sbsa/sbsa.c

include plat/arm/common/arm_common.mk
include plat/arm/css/common/css_common.mk
include plat/arm/board/common/board_common.mk
