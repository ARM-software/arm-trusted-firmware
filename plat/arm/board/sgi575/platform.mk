#
# Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

include plat/arm/css/sgi/sgi-common.mk

SGI575_BASE		=	plat/arm/board/sgi575

PLAT_INCLUDES		+=	-I${SGI575_BASE}/include/

SGI_CPU_SOURCES		:=	lib/cpus/aarch64/cortex_a75.S

BL1_SOURCES		+=	${SGI_CPU_SOURCES}

BL2_SOURCES		+=	lib/utils/mem_region.c			\
				plat/arm/common/arm_nor_psci_mem_protect.c

BL31_SOURCES		+=	${SGI_CPU_SOURCES}			\
				drivers/cfi/v2m/v2m_flash.c		\
				lib/utils/mem_region.c			\
				plat/arm/common/arm_nor_psci_mem_protect.c
