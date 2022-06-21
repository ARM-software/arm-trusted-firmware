/*
 * Copyright (c) 2022, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SGI_SOC_CSS_DEF_H
#define SGI_SOC_CSS_DEF_H

#include <lib/utils_def.h>
#include <plat/arm/board/common/v2m_def.h>
#include <plat/arm/soc/common/soc_css_def.h>
#include <plat/common/common_def.h>

/*
 * Definitions common to all ARM CSSv1-based development platforms
 */

/* Platform ID address */
#define BOARD_CSS_PLAT_ID_REG_ADDR		UL(0x7ffe00e0)

/* Platform ID related accessors */
#define BOARD_CSS_PLAT_ID_REG_ID_MASK		0x0f
#define BOARD_CSS_PLAT_ID_REG_ID_SHIFT		0x0
#define BOARD_CSS_PLAT_TYPE_EMULATOR		0x02

#ifndef __ASSEMBLER__

#include <lib/mmio.h>

#define BOARD_CSS_GET_PLAT_TYPE(addr)					\
	((mmio_read_32(addr) & BOARD_CSS_PLAT_ID_REG_ID_MASK)		\
	>> BOARD_CSS_PLAT_ID_REG_ID_SHIFT)

#endif /* __ASSEMBLER__ */

#define MAX_IO_DEVICES			3
#define MAX_IO_HANDLES			4

/* Reserve the last block of flash for PSCI MEM PROTECT flag */
#define PLAT_ARM_FLASH_IMAGE_BASE	V2M_FLASH0_BASE
#define PLAT_ARM_FLASH_IMAGE_MAX_SIZE	(V2M_FLASH0_SIZE - V2M_FLASH_BLOCK_SIZE)

#define PLAT_ARM_NVM_BASE		V2M_FLASH0_BASE
#define PLAT_ARM_NVM_SIZE		(V2M_FLASH0_SIZE - V2M_FLASH_BLOCK_SIZE)

#endif /* SGI_SOC_CSS_DEF_H */
