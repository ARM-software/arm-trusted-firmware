#
# Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

include plat/arm/css/sgi/sgi-common.mk

BL2_SOURCES		+=	lib/utils/mem_region.c			\
				plat/arm/common/arm_nor_psci_mem_protect.c

BL31_SOURCES		+=	drivers/cfi/v2m/v2m_flash.c		\
				lib/utils/mem_region.c			\
				plat/arm/common/arm_nor_psci_mem_protect.c
