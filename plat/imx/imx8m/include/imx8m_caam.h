/*
 * Copyright (c) 2019, NXP. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef IMX8M_CAAM_H
#define IMX8M_CAAM_H

#include <lib/utils_def.h>

#include <platform_def.h>

#define CAAM_JR0MID		(IMX_CAAM_BASE + 0x10)
#define CAAM_JR1MID		(IMX_CAAM_BASE + 0x18)
#define CAAM_JR2MID		(IMX_CAAM_BASE + 0x20)
#define CAAM_NS_MID		(0x1)

#define JR0_BASE		(IMX_CAAM_BASE + 0x1000)

#define SM_P0_PERM		(JR0_BASE + 0xa04)
#define SM_P0_SMAG2		(JR0_BASE + 0xa08)
#define SM_P0_SMAG1		(JR0_BASE + 0xa0c)
#define SM_CMD			(JR0_BASE + 0xbe4)

/* secure memory command */
#define SMC_PAGE_SHIFT		16
#define SMC_PART_SHIFT		8

#define SMC_CMD_ALLOC_PAGE	0x01	/* allocate page to this partition */
#define SMC_CMD_DEALLOC_PART	0x03	/* deallocate partition */

void imx8m_caam_init(void);

#endif /* IMX8M_CAAM_H */
