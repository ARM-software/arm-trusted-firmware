# Copyright (c) 2024, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

RD_FREMONT_VARIANTS := 0 1
ifneq ($(NRD_PLATFORM_VARIANT),						\
	$(filter $(NRD_PLATFORM_VARIANT),$(RD_FREMONT_VARIANTS)))
	$(error "NRD_PLATFORM_VARIANT for RD-FREMONT should be 0 or 1,"
	"currently set to ${NRD_PLATFORM_VARIANT}.")
endif

# Build options
# Major and Minor versions
override ARM_ARCH_MAJOR			:= 8
override ARM_ARCH_MINOR			:= 7

# Image flags
override NEED_BL1			:= yes
override NEED_BL2			:= yes
override NEED_BL32			:= no
override NEED_RMM			:= no

# Misc options
override CTX_INCLUDE_AARCH32_REGS	:= 0

# RD-Fremont platform uses GIC-700 which is based on GICv4.1
GIC_ENABLE_V4_EXTN			:= 1

include plat/arm/board/neoverse_rd/common/nrd-common.mk

RDFREMONT_BASE	=	plat/arm/board/neoverse_rd/platform/rdfremont

PLAT_INCLUDES	+=	-I${NRD_COMMON_BASE}/include/nrd3/		\
			-I${RDFREMONT_BASE}/include/

NRD_CPU_SOURCES	:=	lib/cpus/aarch64/neoverse_v3.S

# Source files for RD-Fremont variants
PLAT_BL_COMMON_SOURCES							\
		+=	${NRD_COMMON_BASE}/nrd_plat3.c			\
			${RDFREMONT_BASE}/rdfremont_common.c

BL1_SOURCES	+=	${NRD_CPU_SOURCES}				\
			${RDFREMONT_BASE}/rdfremont_err.c
ifeq (${TRUSTED_BOARD_BOOT}, 1)
BL1_SOURCES	+=	${RDFREMONT_BASE}/rdfremont_trusted_boot.c
endif

BL2_SOURCES	+=	${RDFREMONT_BASE}/rdfremont_security.c		\
			${RDFREMONT_BASE}/rdfremont_err.c		\
			lib/utils/mem_region.c				\
			plat/arm/common/arm_nor_psci_mem_protect.c
ifeq (${TRUSTED_BOARD_BOOT}, 1)
BL2_SOURCES	+=	${RDFREMONT_BASE}/rdfremont_trusted_boot.c
endif

BL31_SOURCES	+=	${NRD_CPU_SOURCES}				\
			${RDFREMONT_BASE}/rdfremont_bl31_setup.c	\
			${RDFREMONT_BASE}/rdfremont_topology.c		\
			${RDFREMONT_BASE}/rdfremont_plat_attest_token.c	\
			${RDFREMONT_BASE}/rdfremont_realm_attest_key.c	\
			drivers/cfi/v2m/v2m_flash.c			\
			lib/utils/mem_region.c				\
			plat/arm/common/arm_nor_psci_mem_protect.c

# XLAT options for RD-Fremont variants
BL31_CFLAGS	+=      -DPLAT_XLAT_TABLES_DYNAMIC
BL2_CFLAGS	+=      -DPLAT_XLAT_TABLES_DYNAMIC
