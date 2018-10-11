#
# Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

include plat/arm/css/sgm/sgm-common.mk

SGM775_BASE= plat/arm/board/sgm775

FDT_SOURCES += ${SGM775_BASE}/fdts/sgm775_tb_fw_config.dts

PLAT_INCLUDES +=-I${SGM775_BASE}/include/

BL2_SOURCES		+=	lib/utils/mem_region.c                  \
				plat/arm/common/arm_nor_psci_mem_protect.c

BL31_SOURCES		+=	drivers/cfi/v2m/v2m_flash.c		\
				lib/utils/mem_region.c			\
				plat/arm/common/arm_nor_psci_mem_protect.c
